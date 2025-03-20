#define _CRT_SECURE_NO_WARNINGS 1 

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    int i = 0;
    char ch = '\0';

    if (3 != argc) {
        printf("\n    Usage: bin2c <bin_file>  <c_file>\n\n");
        return -1;
    }
    const char* bin_name = argv[1];
    const char* c_name = argv[2];
    char arry_name[1024] = "";
    FILE* binfile = fopen(bin_name, "rb");
    FILE* cfile   = fopen(c_name, "w");
    //获取无路径的文件名
    strcpy(arry_name, c_name);
    for (int i = 0; i < 1024; i++) {
        if (arry_name[i] == '.')
            arry_name[i] = '\0';
    }

    //二进制文件长度
    fseek(binfile, 0, SEEK_END);
    int size = ftell(binfile);
    fseek(binfile, 0, SEEK_SET);

    fprintf(cfile, "//本文件由bin2c程序自动生成;\n");
    fprintf(cfile,"int           %s_size   = %d;\n", arry_name,size);
    fprintf(cfile,"unsigned char %s_data[] = { \n", arry_name);

    i = 1;
    unsigned char one_char = 0;
    while (1) {
        int sz = fread(&one_char, 1, 1, binfile);
        if (sz == 0)
            break;
        fprintf(cfile,"0x%02X%,%s", one_char, (i++ % 16) == 0 ? "\n" : "");
    }
    fseek(cfile, -1, SEEK_CUR);
    fprintf(cfile,"\n};\n");

    fclose(binfile);
    fclose(cfile);
}