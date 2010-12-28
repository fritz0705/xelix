#pragma once

#include <common/generic.h>

#include <filesystems/vfs.h>

typedef struct FILE
{
	const char* path;
	const char* modes;
	fsNode_t* node;
} FILE;

FILE *fopen(const char* path, const char* mode);
//FILE *freopen(const char *path, const char *mode, FILE *fp);
int fclose(FILE* fp);
int fgetc(FILE* fp);
size_t fwrite (const void *array, size_t size, size_t count, FILE *stream);
int fputc(int c, FILE *fp);
