
//目标：加载zip文件到内存，直接读取内存中文件数据

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOCAL_FILE_HEADER_SIGNATURE			0x04034b50
#define CENTRAL_DIRECTORY_HEADER_SIGNATURE  0x02014b50
#define DIGITAL_SIGNATURE_MAGIC				0x05054b50
#define END_CENTRAL_DIRECTORY_SIGNATURE		0x06054b50

#pragma pack(push)
#pragma pack(1)


struct LOCAL_FILE_HEADER {
	uint32_t signature;			//  local file header signature     4 bytes(0x04034b50)
	uint16_t extract_version;	//	version needed to extract       2 bytes
	uint16_t flag;				//	general purpose bit flag        2 bytes
	uint16_t compression_method; // compression method              2 bytes
	uint16_t last_mod_file_time;//	last mod file time              2 bytes
	uint16_t last_mod_file_date;//	last mod file date              2 bytes
	uint32_t crc_32;			//	crc - 32                        4 bytes
	uint32_t compressed_size;	//	compressed size                 4 bytes
	uint32_t uncompressed_size; //	uncompressed size               4 bytes
	uint16_t file_name_length;	//	file name length                2 bytes
	uint16_t extra_field_length;//	extra field length              2 bytes
	char     filename_extrafield[0];
};

typedef struct LOCAL_FILE_HEADER FILE_HEADER;

struct DATA_DESCRIPTOR {
	uint32_t crc_32;			//	crc - 32                        4 bytes
	uint32_t compressed_size;	//	compressed size                 4 bytes
	uint32_t uncompressed_size; //	uncompressed size               4 bytes
};

//每条记录头对应一个文件或目录
struct CENTRAL_DIRECTORY_HEADER {
	uint32_t header_signature;		//central file header signature   4 bytes(0x02014b50)
	uint16_t version_made_by;		//version made by                 2 bytes
	uint16_t version_needed_extract;//version needed to extract       2 bytes
	uint16_t general_purpose_flag;	//general purpose bit flag        2 bytes
	uint16_t compression_method;	//compression method              2 bytes
	uint16_t last_mod_filetime;		//last mod file time              2 bytes
	uint16_t last_mod_filedate;		//last mod file date              2 bytes
	uint32_t crc_32;				//crc - 32                        4 bytes
	uint32_t compressed_size;		//compressed size                 4 bytes
	uint32_t uncompressed_size;		//uncompressed size               4 bytes
	uint16_t filename_length;		//file name length                2 bytes
	uint16_t extra_field_length;	//extra field length              2 bytes
	uint16_t file_comment_length;	//file comment length             2 bytes
	uint16_t disk_number_start;		//disk number start               2 bytes
	uint16_t internal_file_attributes;//internal file attributes        2 bytes
	uint32_t external_file_attributes;//external file attributes        4 bytes
	uint32_t offset_local_header;	//relative offset of local header 4 bytes
	char     filename[0];			//file name(variable size)	
		//extra field(variable size)
		//file comment(variable size)
};

typedef struct CENTRAL_DIRECTORY_HEADER DIR_ENTER;

struct DIGITAL_SIGNATURE {
	uint32_t header_signature;	//header signature                4 bytes(0x05054b50)
	uint16_t size_data;			//size of data                    2 bytes
	uint8_t signature_data[0];	//signature data(variable size)
};


struct END_CENTRAL_DIRECTORY_RECORD {
	uint32_t signature;		//	end of central dir signature    4 bytes(0x06054b50)
	uint16_t number_disk;	//	number of this disk             2 bytes
							//	number of the disk with the
	uint16_t unused1;		//	start of the central directory  2 bytes
							//	total number of entries in the
	uint16_t unused2;		//	central directory on this disk  2 bytes
							//	total number of entries in
	uint16_t number_central_directory;		//	the central directory           2 bytes  zip压缩包中的文件总数
	uint32_t size_central_directory;		//	size of the central directory   4 bytes
							//	offset of start of central
							//	directory with respect to
	uint32_t offset_central;		//	the starting disk number        4 bytes
	uint16_t comment_lengt;	//	.ZIP file comment length        2 bytes
							//	.ZIP file comment(variable size)
};


#pragma pack(pop)


typedef struct MZIP_HANDLE {
	uint8_t* buf;
	int buf_len;
	uint8_t b_need_free; //如果是打开文件自己申请的内存，需要free

	struct END_CENTRAL_DIRECTORY_RECORD* end_dir;
	struct CENTRAL_DIRECTORY_HEADER* central_dir;

} MZIP_T;


static int find_central_directory(MZIP_T *  pzip)
{
	int find = 0;
	for (int i = 20; i < 2048 && i< pzip->buf_len; i++)
	{
		uint32_t* signature = (uint32_t*)(pzip->buf + pzip->buf_len - i);
		if (*signature == END_CENTRAL_DIRECTORY_SIGNATURE) {
			pzip->end_dir = (struct END_CENTRAL_DIRECTORY_RECORD*)signature;
			find = 1;
			break;
		}
	}
	if (find == 0)
		return -1;

	int offset = pzip->end_dir->offset_central;
	pzip->central_dir = (struct CENTRAL_DIRECTORY_HEADER*)(pzip->buf + offset);
	if (pzip->central_dir->header_signature != CENTRAL_DIRECTORY_HEADER_SIGNATURE) {
		return -1;
	}
	return 0;
}
static inline DIR_ENTER *next_dirrectory_header(DIR_ENTER * dir_entr)
{
	int offset = dir_entr->filename_length + dir_entr->file_comment_length 
				+ dir_entr->extra_field_length + sizeof(DIR_ENTER);
	DIR_ENTER* next_enter = (DIR_ENTER*)(((uint8_t*)dir_entr) + offset);
	if (next_enter->header_signature == CENTRAL_DIRECTORY_HEADER_SIGNATURE)
		return next_enter;
	else
		return NULL;
}
static inline int is_need_enter(DIR_ENTER* dir_enter, const char* filename)
{
	int name_len = dir_enter->filename_length;
	if (filename[name_len] != 0)//文件名长度都不一致，退出
		return 0;
	for (int i = 0; i < name_len; i++) {
		if (dir_enter->filename[i] != filename[i])
			return 0;
	}
	return 1;
}

void* mzip_getfile(MZIP_T* pzip, const char* filename, int* file_size)
{
	struct CENTRAL_DIRECTORY_HEADER* dir_enter = pzip->central_dir;
	while (dir_enter != NULL)
	{
		int need = is_need_enter(dir_enter, filename);
		if (need) {
			FILE_HEADER* file_header = (FILE_HEADER*)(pzip->buf + dir_enter->offset_local_header);
			if (file_header->signature != LOCAL_FILE_HEADER_SIGNATURE)
				return NULL;
			void* data = (uint8_t *)file_header + sizeof(FILE_HEADER) + file_header->file_name_length
				+ file_header->extra_field_length;
			*file_size = file_header->uncompressed_size;
			return data;
		}
		dir_enter = next_dirrectory_header(dir_enter);
	}
	return NULL;
}
MZIP_T* muzip_mopen(uint8_t* buf, int buf_len)
{
	int ret = 0;
	MZIP_T * pzip = malloc(sizeof(MZIP_T));
	pzip->buf = buf;
	pzip->buf_len = buf_len;
	ret = find_central_directory(pzip);
	//ret = find_central_directory_header(pzip);
	return pzip;
}

MZIP_T* muzip_fopen(const char * filename)
{
	FILE* pf = fopen(filename, "rb");
	if (pf == NULL)
		return NULL;
	fseek(pf, 0, SEEK_END);
	int fsize = ftell(pf);
	fseek(pf, 0, SEEK_SET);
	uint8_t* buf = malloc(fsize);
	if (buf == NULL) {
		fclose(pf);
		return NULL;
	}
	fread(buf, 1, fsize, pf);
	fclose(pf);

	MZIP_T * pzip = muzip_mopen(buf, fsize);
	pzip->b_need_free = 1;
	return pzip;
}
int muzip_free(MZIP_T* pzip)
{
	if (pzip != NULL) {
		if (pzip->buf != NULL)
			free(pzip->buf);
	}
	return 0;
}