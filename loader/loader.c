#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
unsigned int count_phdr_entry;
int page_faults,page_allocations;
size_t lost_memory;
// /*
//  * Seg fault handling function
//  */
void sigsegv_handler(int signum,siginfo_t *info,void *context) {
    void *fault_address=info->si_addr;
    printf("Fault_address: %p\n",fault_address);
    // size_t segment_index = find_segment_index(fault_address);
    
    // Find the segment that contains the fault_address
    int found=0;
    for (size_t i = 0; i < ehdr->e_phnum; i++) {
      printf("%d\n",i);
      uintptr_t segment_start = phdr[i].p_vaddr;
      uintptr_t segment_end = segment_start + phdr[i].p_memsz;
      printf("segment_end: %p\n", segment_end);
      printf("segment_start: %p\n", segment_start);
      printf("fault_address: %p\n", fault_address);

      if (fault_address >= (void *)segment_start && fault_address < (void *)segment_end) 
      {
        // Map memory for this segment
        size_t temp;
        if(phdr[i].p_memsz%4096==0){
          temp=phdr[i].p_memsz;
        }
        else{
          temp=((phdr[i].p_memsz/4096)+1)*4096;
        }
        printf("%d\n",temp);
        void *virt_mem = mmap((void *)(uintptr_t)phdr[i].p_vaddr,temp,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_FIXED, fd, phdr[i].p_offset);
        if (virt_mem == MAP_FAILED) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
        if (read(fd, (void *)(uintptr_t)phdr[i].p_vaddr, temp) == -1) {
            perror("Error reading segment data");
            close(fd);
            exit(1);
        }

        //printf("Mmap kar diya hai\n");
        // Update page fault and page allocation counts
        page_faults++;
        page_allocations++;
        found=1;
        break;
      }
    }
    if(found==1){
      printf("we got it\n");
    }
    else{
      printf("we did not get it\n");
    }
    // Continue execution
    return;
    // Perform custom error handling or cleanup here.
    exit(EXIT_FAILURE);
}
/*
 * release memory and other cleanups
 */
void loader_cleanup()
{
  for (size_t i = 0; i < count_phdr_entry; i++)
  {
    if (phdr[i].p_type == PT_LOAD)
    { 
      void *virt_mem = (void *)(uintptr_t)phdr[i].p_vaddr;
      size_t mem_size = phdr[i].p_memsz;
      if (virt_mem)
      {
        munmap(virt_mem, mem_size);
      }
    }
  }
  free(phdr);
  free(ehdr);
  close(fd);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **argv)
{
  // opening file
  fd = open(argv[1], O_RDONLY);
  if (fd == -1)
  {
    perror("Error opening File");
    return;
  }

  // allocating space for elf header
  ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
  int x = read(fd, ehdr, sizeof(Elf32_Ehdr));
  if (x == -1)
  {
    perror("Error in Elf_header");
    return;
  }

  unsigned int phdr_offset = ehdr->e_phoff;
  count_phdr_entry = ehdr->e_phnum;
  unsigned int size_phdr_entry = ehdr->e_phentsize;
  unsigned int elfentry = ehdr->e_entry;
  printf("phdr_offset: %u\n", ehdr->e_phoff);
  printf("count_phdr_entry: %u\n", ehdr->e_phnum);
  printf("size_phdr_entry: %u\n", ehdr->e_phentsize);
  printf("elfentry: %u\n", ehdr->e_entry);

  //int flag = 1;
  // allocating space for program header
  phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr) * count_phdr_entry);

  // looping over program header entries
  int counter = 0;
  // for (size_t i = 0; i < ehdr->e_phnum; i++)
  // {
  if (lseek(fd, ehdr->e_phoff, SEEK_SET) == -1)
  {
    close(fd);
    exit(EXIT_FAILURE);
  }
  int k = read(fd, phdr, ehdr->e_phentsize*ehdr->e_phnum);
  if (k == -1)
  {
    perror("Error in Program_header");
  }

  //   if (phdr[i].p_type == PT_LOAD)
  //   {
  //     counter++;
  //     // checking if the entry point lies in the segment and only then loading
  //     // if (phdr[i].p_memsz > (elfentry) - (phdr[i].p_vaddr) && phdr[i].p_vaddr <= elfentry) {        
  //     // creating a virtual mapping
  //     printf("mmap arguments:\n");
  //     printf("  Address: %p\n", (void *)(uintptr_t)phdr[i].p_vaddr);
  //     printf("  Offset: %u\n", phdr[i].p_offset);
  //     unsigned int temp=0;
  //     if(phdr[i].p_memsz%4096==0){
  //       temp=phdr[i].p_memsz;
  //     }
  //     else{
  //       temp=((phdr[i].p_memsz/4096))*4096;
  //       temp+=4096;
  //     }
  //     printf("  Length: %u\n", temp);

  //     void *virt_mem = mmap((void *)(uintptr_t)phdr[i].p_vaddr, temp, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, phdr[i].p_offset);
      
  //     if (virt_mem == MAP_FAILED)
  //     {
  //       perror("mmap");
  //       exit(EXIT_FAILURE);
  //     }
  //     if (lseek(fd, phdr[i].p_offset, SEEK_SET) == -1) {
  //           perror("Error seeking to segment data");
  //           close(fd);
  //           exit(1);
  //     }
  //     if (read(fd, virt_mem, phdr[i].p_filesz) == -1) {
  //           perror("Error reading segment data");
  //           close(fd);
  //           exit(1);
  //     }

  //   }
  // }
  // printf("The Number of PT_LOAD: %d\n",counter);

  // typecasting to a function pointer
  int (*_start)() = (int (*)())(ehdr->e_entry);
  // the last two steps of calling the function and printing the result
  int result = _start();
  printf("User _start return value = %d\n", result);
}

int main(int argc, char **argv)
{
  struct sigaction sig;
  memset(&sig,0,sizeof(sig));
  sig.sa_flags=SA_SIGINFO;
  sig.sa_sigaction=sigsegv_handler;
  sigaction(SIGSEGV,&sig,NULL);
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <elf_filename>\n", argv[0]);
    return 1;
  }

  load_and_run_elf(argv);
  loader_cleanup(); // Cleanup resources
  return 0;
}
