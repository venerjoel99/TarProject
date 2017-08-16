# Tar Project
An extremely basic tar file cleaner that takes out specific strings specified from the [Team Win Recovery issue 964](https://github.com/TeamWin/Team-Win-Recovery-Project/issues/964) from a corrupted tar file and makes a clean copy 
### Basic Algorithm
  ---
1. Check for USTAR indicator
2. Parse the header for file name, size, header index, and file buffer size
3. Check for leak by checking for the next header's USTAR indicator
4. Copy the header and file content if leak check determines no leak.  
5. If there is a leak, check the file for leaked strings and make sure they don't get copied
### Some present fixable problems
  ---
1. Program doesn't remove any other strings other than the strings specified in TarCleaner.c
2. It will always assume that the last file has a leak due to a lack of a following ustar file header with a ustar indicator, which affects program runtime only due to the leaked string checks through the last file regardless of whether it has leaked strings or not
