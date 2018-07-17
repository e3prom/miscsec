/* fool.cpp: Defines the entry point for the console application.
 * Compile instructions:
 * cl /GS /EHsc fool.cpp foolDLL.lib /link /SafeSEH /DYNAMICBASE:no /NXCompat:no
 */
// #include "stdafx.h"
#include "stdio.h"
#include "windows.h"

extern "C" __declspec(dllimport)void test();

void GetInput(char *str, char *out)
{
  long lSize;
  char buffer[500];
  char *temp;
  FILE *hFile;
  size_t result;

  try {
    hFile = fopen(str, "rb");   // open file for reading of bytes
    if (hFile == NULL) {
      printf("No such file.\n");
      exit(1);
    }
    fseek(hFile, 0, SEEK_END);
    lSize = ftell(hFile);
    rewind(hFile);
    temp = (char *)malloc(sizeof(char)*lSize);
    result = fread(temp,1,lSize,hFile);
    memcpy(buffer, temp, result);         // vulnerable call to memcpy
    memcpy(out, buffer, strlen(buffer));  // triggers SEH before /GS
    printf("Input received: %s\n", buffer);
  }
  catch (char * strErr) {
    printf("No valid input received!\n");
    printf("Exception: %s\n", strErr);
  }
  test(); // Call the DLL with XOR, POP, POP, RETN instructins sequence.
}

int main(int argc, char *argv[])
{
  char foo[1048] = "beef"; // IMPORTANT: The overflow should occures with 3rd byte between 0x80 - 0xFB.
  char buf2[500];
  GetInput(argv[1],buf2);
  return 0;
}
