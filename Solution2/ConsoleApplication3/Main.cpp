// ConsoleApplication3.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

int main()
{
	char file1[100];
	char file2[100];
	printf("Enter file to clean:\n");
	scanf_s("%s", file1, 100);
	printf("Enter copy file name:\n");
	scanf_s("%s", file2, 100);
	time_t a = time(0);
	TarCleaner::cleanCopy(file1, file2);
	time_t b = time(0);
	printf("Clean time: %d seconds \n", b - a);
	printf("Enter anything to finish:\n");
	scanf_s("%s", file1, 100);
	return 0;
}

