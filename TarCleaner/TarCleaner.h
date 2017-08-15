#ifndef TARCLEANER_H_INCLUDED
#define TARCLEANER_H_INCLUDED
#include <stdio.h>

typedef struct _header{
    char fileName[100];
    unsigned int fileSize;
    unsigned int bufferSize;
    unsigned int headerIndex;
}header;

/*
typedef struct _file{
    char fileName[100];
    char fileMode[8];
    char ownerID[8];
    char groupID[8];
    char fileSize[12];
    char lastMod[12];
    char checkSum[8];
    char linkedFile[100];
    char indicator[sizeof "ustar  "];
    char userName[32];
    char groupName[32];
    char majorNum[8];
    char minorNum[8];
    char prefix[155];
}UstarFile;
*/

int cleanAndCopy(const char* fileName, const char* copyFileName);

#endif
