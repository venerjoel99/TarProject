#include "TarCleaner.h"
#include <time.h>

int main(){
    char file1[100];
    char file2[100];
    printf("Enter file to clean:\n");
    scanf("%s", file1);
    printf("Enter copy file name:\n");
    scanf("%s", file2);
    time_t a = time(0);
    cleanAndCopy(file1, file2);
    time_t b = time(0);
    printf("Clean time: %i seconds\n", b - a);
    return 0;
}
