#ifndef MZIP_H
#define MZIP_H

#include <stdint.h>

typedef struct _MZIP_T MZIP_T;

MZIP_T* muzip_mopen(uint8_t* buf, int buf_len);
MZIP_T* muzip_fopen(const char* filename);
int muzip_free(MZIP_T* pzip);
void* mzip_getfile(MZIP_T* pzip,const char* filename, int* file_size);

#endif // !MZIP_H

