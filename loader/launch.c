#include "loader.h"

#define TRUE 1
#define FALSE 0

typedef int bool;

int is_elf_header_valid(const char* filename) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    return FALSE;
  }

  unsigned char elf_magic[4];
  fread(elf_magic, sizeof(unsigned char), 4, file);

  int is_valid = (elf_magic[0] == 0x7F && elf_magic[1] == 'E' && elf_magic[2] == 'L' && elf_magic[3] == 'F');

  fclose(file);

  return is_valid;
}


int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  const char* elfFileName = argv[1];

  FILE* file = fopen(elfFileName, "rb");
  if (!file) {
      printf("Error: ELF file '%s' not found.\n", elfFileName);
      return 1;
  }
  fclose(file);

  if (!is_elf_header_valid(elfFileName)) {
      printf("Error: '%s' is not a valid ELF file.\n", elfFileName);
      return 1;
  }
  load_and_run_elf(argv);
  loader_cleanup();
  return 0;
}
