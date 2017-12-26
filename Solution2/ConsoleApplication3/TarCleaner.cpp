#include "stdafx.h"
#include "TarCleaner.h"
#include <string>
#define FILE_SIZE_INDEX 124
#define USTAR_INDEX 257
#define BLOCKSIZE 512
#define true 0
#define false 1

typedef TarCleaner::Integer Integer;
const char* leaked_strings[] = {
	"I:Closing tar\n",
	"storing xattr user.default\n",
	"storing xattr user.inode_cache\n",
	"storing xattr user.inode_code_cache\n"
};

TarCleaner::TarCleaner(const char* input, const char* output)
{
	fopen_s(&in_file, input, "rb");
	fopen_s(&out_file, output, "wb");
}

TarCleaner::~TarCleaner()
{
	fclose(in_file);
	fclose(out_file);
}

/**
* Find the string instance at a specific index
* @param str - the char array to match to
* @param index of the first character to compare
* @return 0 if it's a match, 1 if not
*/
int TarCleaner::findStr(const char *str, Integer index) {
	_fseeki64(in_file, index, SEEK_SET);
	int size = strlen(str);
	char *c = new char[size];
	fread(c, 1, size, in_file);
	c[size] = '\0';
	int result = strcmp(c, str);
	return result;
}

/**
* Find the number of bytes in a file
* @return the size in bytes
*/
Integer TarCleaner::findSize() {
	Integer size;
	_fseeki64(in_file, 0, SEEK_END);
	size = _ftelli64(in_file);
	_fseeki64(in_file, 0, SEEK_SET);
	size -= _ftelli64(in_file);
	return size;
}

/**
* Check for ustar indicator at a file index
* @param index of the first character to compare
* @return 0 if match, 1 if not
*/
int TarCleaner::checkUstar(Integer index) {
	return findStr("ustar", index);
}

/**
* Parse the tar header file for info
* at a specific index
* @param index of the first byte of the header
* @return a data structure with file name and size
*/
TarCleaner::Header TarCleaner::parseHeader(Integer index) {
	TarCleaner::Header h;
	h.headerIndex = index;
	_fseeki64(in_file, h.headerIndex, SEEK_SET);
	fgets(h.fileName, sizeof h.fileName, in_file);
	_fseeki64(in_file, h.headerIndex + FILE_SIZE_INDEX, SEEK_SET);
	fscanf_s(in_file, "%o", &h.fileSize);
	int remainder = h.fileSize % BLOCKSIZE;
	h.bufferSize = h.fileSize;
	if (remainder > 0) {
		h.bufferSize += (BLOCKSIZE - remainder);
	}
	return h;
}

/**
* Check for a leak starting at beginning the tar header file
* to the end of the file content
* @param index of the file's tar header
* @return 0 if there's a leak, 1 if no leak
*/
int TarCleaner::leaking(Integer index) {
	if (index < BLOCKSIZE && index > USTAR_INDEX) {
		return true;
	}
	Header h = parseHeader(index);
	Integer secondUstar = h.headerIndex + USTAR_INDEX + h.bufferSize + BLOCKSIZE;
	if (secondUstar > (tarSize - (BLOCKSIZE * 2))) {
		secondUstar -= USTAR_INDEX;
		if (secondUstar != (tarSize - (BLOCKSIZE * 2))) return true;
	}
	else if (checkUstar(secondUstar) != 0) {
		return true;
	}
	return false;
}

/**
* Check for the leaked strings
* at a specific index
* @param index of the first char to compare to
* @return the size of the leaked string if there's one,
*         0 if not
*/
int TarCleaner::checkLeak(Integer index) {
	int arrSize = sizeof(leaked_strings) / sizeof(leaked_strings[0]);
	for (int l = 0; l < arrSize; l++) {
		if (findStr(leaked_strings[l], index) == true) {
			return strlen(leaked_strings[l]);
		}
	}
	return 0;
}

/**
* Copy a large file 512 bytes at a time
* @param start - first index(inclusive)
* @param finish - last index(exclusive)
* @param stream to insert into
* @return 0 for success
*/
int TarCleaner::copy(Integer start, Integer finish) {
	const Integer blockSize = BLOCKSIZE * 64;
	char buffer[blockSize];
	_fseeki64(in_file, start, SEEK_SET);
	_fseeki64(out_file, 0, SEEK_END);
	for (Integer i = start; i < finish; i += blockSize) {
		Integer bufferSize = blockSize;
		if (finish - i < bufferSize) {
			bufferSize = finish - i;
		}
		//TODO: might need to change how the file is being copied to account for bigger files in tar files
		fread(buffer, 1, bufferSize, in_file);
		fwrite(buffer, 1, bufferSize, out_file);
	}
	return 0;
}

/**
* Write the last 2 blocks of null chars
* to the copy tar file
* @param nullNum - number of null chars to insert
* @return 0 for success
*/
int TarCleaner::writeNull(int nullNum) {
	char* nullChar = new char[nullNum];
	for (int i = 0; i < nullNum; i++) {
		nullChar[i] = (char)0;
	}
	fseek(out_file, 0, SEEK_END);
	fwrite(nullChar, 1, nullNum, out_file);
	delete[] nullChar;
	return 0;
}

/**
* The main public function to copy a clean version
* of a corrupted tar file
* @param fileName - the corrupted tar file
* @param copyFileName - the file to copy into
* @return 0 if both file streams close successfully
*/
int TarCleaner::cleanAndCopy() {
	tarSize = findSize();
	for (Integer j = 0; j < tarSize; j++) {
		//printf("%I64u %d \n", j, checkUstar(j + USTAR_INDEX));
		if (checkUstar(j + USTAR_INDEX) != 0) continue;
		Header h = parseHeader(j);
		Integer end = h.headerIndex + BLOCKSIZE + h.bufferSize;
		if (leaking(h.headerIndex) == true) {
			printf("%I64u Leaked file: %s\n", h.headerIndex, h.fileName);
			Integer k = 0;
			Integer startIndex = h.headerIndex;
			Integer index;
			Integer elements = end - h.headerIndex;
			while (k < elements) {
				index = startIndex;
				while ((index < tarSize) && (index < end) && checkLeak(index) <= 0) {
					index += BLOCKSIZE;
				}
				Integer leakLength = checkLeak(index);
				end += leakLength;
				copy(startIndex, index);
				k = k + (index - startIndex);
				startIndex = index + leakLength;
			}
		}
		else {
			copy(h.headerIndex, end);
		}
		j = end - 1;
	}
	writeNull(BLOCKSIZE * 2);
	return 0;
}
