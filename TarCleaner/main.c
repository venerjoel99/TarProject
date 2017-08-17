#include "TarCleaner.h"

int main(){
    char file1[100];
    char file2[100];
    printf("Enter file to clean:\n");
    scanf("%s", file1);
    printf("Enter copy file name:\n");
    scanf("%s", file2);
    cleanAndCopy(file1, file2);
    return 0;
}
