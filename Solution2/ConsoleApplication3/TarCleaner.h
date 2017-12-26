#pragma once
#ifndef TARCLEANER_H_INCLUDED
#define TARCLEANER_H_INCLUDED
#include <stdio.h>

class TarCleaner
{
public:
	typedef __int64 Integer;
	typedef struct _header {
		char fileName[100];
		int fileSize;
		int bufferSize;
		Integer headerIndex;
	}Header;
private:
	FILE *in_file;
	FILE *out_file;
	Integer tarSize = 0;
	int writeNull(int);
	int copy(Integer, Integer);
	int checkLeak(Integer);
	int leaking(Integer);
	Header parseHeader(Integer);
	Integer findSize(void);
	int findStr(const char*, Integer);
	int cleanAndCopy();
	int checkUstar(Integer);
	TarCleaner(const char*, const char*);
	TarCleaner();
	~TarCleaner();
public:
	static int TarCleaner::cleanCopy(const char* input, const char* output) {
		TarCleaner *cleaner = new TarCleaner(input, output);
		int return_val = cleaner->cleanAndCopy();
		delete cleaner;
		return return_val;
	}
};

#endif
