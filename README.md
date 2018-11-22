# Tar Project
An extremely basic tar file cleaner that takes out specific strings specified from a corrupted tar file and makes a clean copy.  This program was made in response to the [Team Win Recovery issue 964](https://github.com/TeamWin/Team-Win-Recovery-Project/issues/964), and the specific strings removed are specified at the very bottom of the thread.  This is not the only tar file cleaner project on GitHub, so make sure to also check out [BuildingAtom's Tar File Cleaner](https://github.com/BuildingAtom/CleanTwrpTar) as well.
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
### Notes
  ---
1. There are 2 executables are located in the Executables folder. SmallerTarCleaner is written in C and cleans files up to 2GB only. BigTarCleaner is written in C++ and can be used for files bigger than 2GB.
2. BigTarCleaner requires [Microsoft Visual C++ Redistributable for Visual Studio 2017](https://go.microsoft.com/fwlink/?LinkId=746572)
