#include "TarCleaner.h"
#include <stdio.h>
#define FILE_SIZE_INDEX 124
#define USTAR_INDEX 257
#define BLOCKSIZE 512
#define true 0
#define false 1

FILE *fp;

const char* leaked_strings[] = {
"I:Closing tar\n",
"storing xattr user.default\n",
"storing xattr user.inode_cache\n",
"storing xattr user.inode_code_cache\n"
};

int findStr(const char *str, int index){
    fseek(fp, index, SEEK_SET);
    int i;
    for (i=0; i< strlen(str); i++){
        char c = (char)fgetc(fp);
        if (c!=str[i]) {
            return 1;
        }
    }
    return 0;
}

int findSize(){
    int size;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    size -= ftell(fp);
    return size;
}

int checkUstar(int index){
    return findStr("ustar  ", index);

}

/*
UstarFile parseFile(int index){
    UstarFile u;
    fseek(fp, index, SEEK_SET);
    fgets(u.fileName, sizeof u.fileName, fp);
    fseek(fp, index + 100, SEEK_SET);
    fgets(u.fileMode, sizeof u.fileMode, fp);
    fseek(fp, index + 108, SEEK_SET);
    fgets(u.ownerID, sizeof u.ownerID, fp);
    fseek(fp, index + 116, SEEK_SET);
    fgets(u.groupID, sizeof u.groupID, fp);
    fseek(fp, index + 124, SEEK_SET);
    fgets(u.fileSize, sizeof u.fileSize, fp);
    fseek(fp, index + 136, SEEK_SET);
    fgets(u.lastMod, sizeof u.lastMod, fp);
    fseek(fp, index + 148, SEEK_SET);
    fgets(u.checkSum, sizeof u.checkSum, fp);
    fseek(fp, index + 157, SEEK_SET);
    fgets(u.linkedFile, sizeof u.linkedFile, fp);
    fseek(fp, index + 257, SEEK_SET);
    fgets(u.indicator, sizeof u.indicator, fp);
    fseek(fp, index + 265, SEEK_SET);
    fgets(u.userName, sizeof u.userName, fp);
    fseek(fp, index + 297, SEEK_SET);
    fgets(u.groupName, sizeof u.groupName, fp);
    fseek(fp, index + 329, SEEK_SET);
    fgets(u.majorNum, sizeof u.majorNum, fp);
    fseek(fp, index + 337, SEEK_SET);
    fgets(u.minorNum, sizeof u.minorNum, fp);
    fseek(fp, index + 345, SEEK_SET);
    fgets(u.prefix, sizeof u.prefix, fp);
    return u;
}
int printFile(UstarFile file){
    printf("File: %s \n", file.fileName);
    printf("Mode: %s \n", file.fileMode);
    printf("Owner ID: %s \n", file.ownerID);
    printf("Group ID: %s \n", file.groupID);
    printf("Size: %s \n", file.fileSize);
    printf("Last mod time: %s \n", file.lastMod);
    printf("Check sum: %s \n", file.checkSum);
    printf("Linked file: %s \n", file.linkedFile);
    printf("Indicator: %s \n", file.indicator);
    printf("Username: %s \n", file.userName);
    printf("Group name: %s \n", file.groupName);
    printf("Major number: %s \n", file.majorNum);
    printf("Minor num: %s \n", file.minorNum);
    printf("Prefix: %s \n", file.prefix);
    return 0;
}
*/

header parseHeader(int index){
    header h;
    h.headerIndex = index;
    fseek(fp, h.headerIndex, SEEK_SET);
    fgets(h.fileName, sizeof h.fileName, fp);
    fseek(fp, h.headerIndex + FILE_SIZE_INDEX, SEEK_SET);
    fscanf(fp, "%o", &h.fileSize);
    int remainder = h.fileSize % BLOCKSIZE;
    h.bufferSize = h.fileSize;
    if (remainder > 0){
        h.bufferSize += (BLOCKSIZE - remainder);
    }
    return h;
}

int leaking(int index, int tarSize){
    if (index < BLOCKSIZE && index > USTAR_INDEX){
        return true;
    }
    header h = parseHeader(index);
    int secondUstar = h.headerIndex + USTAR_INDEX + h.bufferSize + BLOCKSIZE;
    if (secondUstar > tarSize){
        secondUstar -= USTAR_INDEX;
        if (secondUstar!=tarSize) return true;
    }
    else if(checkUstar(secondUstar)!=0){
        return true;
    }
    return false;
}

int checkLeak(int index){
    int l;
    int arrSize = sizeof(leaked_strings) / sizeof(leaked_strings[0]);
    for (l=0; l < arrSize; l++){
        if (findStr(leaked_strings[l], index)==true){
            return strlen(leaked_strings[l]);
        }
    }
    return 0;
}

int copy(int start, int finish, FILE* copy){
    char buffer[finish - start];
    fseek(fp, start, SEEK_SET);
    fread(buffer, 1, finish - start, fp);
    fseek(copy, 0, SEEK_END);
    fwrite(buffer, 1, finish - start, copy);
    return 0;
}

int copyLarge(int start, int finish, FILE* copyFile){
    int i;
    for (i = start; i < finish; i += BLOCKSIZE){
        int bufferSize = BLOCKSIZE;
        if (finish - i < bufferSize){
            bufferSize = finish - i;
        }
        copy(i, i + bufferSize, copyFile);
    }
    return 0;
}

int writeNull(int nullNum, FILE *copy){
    char nullChar[nullNum];
    int i;
    for (i=0; i < sizeof nullChar; i++){
        nullChar[i] = (char)0;
    }
    fseek(copy, 0, SEEK_END);
    fwrite(nullChar, 1, sizeof nullChar, copy);
    return 0;
}

int cleanAndCopy(const char* fileName, const char* copyFileName){
    fp = fopen(fileName, "rb");
    FILE *fcopy;
    fcopy = fopen(copyFileName, "wb");
    int size = findSize();
    int j;
    for (j = 0; j < size; j++){
        if ((j % 1000000)==0) printf("%i MB\n", j / 1000000);
        if (checkUstar(j)!=0) continue;
        header h = parseHeader(j - USTAR_INDEX);
        int end = h.headerIndex + BLOCKSIZE + h.bufferSize;
        if (leaking(h.headerIndex, size)==true){
            printf("%i Leaked file: %s\n", h.headerIndex, h.fileName);
            int k = 0;
            int startIndex = h.headerIndex;
            int index;
            int elements = end - h.headerIndex;
            while (k < elements){
                index = startIndex;
                while(checkLeak(index)<=0 && (index < end) && (index < size)){
                    index++;
                }
                int leakLength = checkLeak(index);
                end += leakLength;
                copyLarge(startIndex, index, fcopy);
                k = k + (index - startIndex);
                startIndex = index + leakLength;
            }
        }
        else{
            copyLarge(h.headerIndex, end, fcopy);
        }
        j = end - 1;
    }
    writeNull(BLOCKSIZE * 2, fcopy);
    return fclose(fcopy) + fclose(fp);
}
