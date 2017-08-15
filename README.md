# TarProject

### Basic Algorithm
  ---
1. Check for USTAR indicator
2. Parse the header for file name, size, header index, and file buffer size
3. Check for leak by checking for the next header's USTAR indicator
4. Copy the header and file content if leak check determines no leak.  
5. If there is a leak, check the file for leaked strings and make sure they don't get copied
