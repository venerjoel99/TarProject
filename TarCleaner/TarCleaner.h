#ifndef TARCLEANER_H_INCLUDED
#define TARCLEANER_H_INCLUDED
#include <stdio.h>

typedef struct _header{
    char fileName[100];
    unsigned int fileSize;
    unsigned int bufferSize;
    unsigned int headerIndex;
}header;

int cleanAndCopy(const char* fileName, const char* copyFileName);

#endif
