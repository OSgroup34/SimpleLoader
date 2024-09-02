#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  ehdr=NULL;
  free (ehdr);
  phdr=NULL;
  free (phdr);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
// 1. Load entire binary content into the memory from the ELF file.
    fd=open(*exe, O_RDONLY);
    off_t size=lseek(fd,0,SEEK_END);
    //error handling
    if (size==-1){
        printf("Error in getting file size\n");
        exit(1);
    }
    lseek(fd,0,SEEK_SET);
    //entire file memory allocation
    char* heapmemalloc=(char*)malloc(size);
    //error handling
    if (!heapmemalloc){
        printf("error in memory allocation\n");
        exit(1);
    }
    ssize_t readfile=read(fd, heapmemalloc,size);

    //error handling
    if (readfile<0 || (size_t)readfile!=size){
    perror("Error in reading file");
    free(heapmemalloc);
    exit(1);
    }
    //ehdr and phdrs alloc
    ehdr=(Elf32_Ehdr*)heapmemalloc;
    phdr=(Elf32_Phdr*)(heapmemalloc+(*ehdr).e_phoff);

// 2. Iterate through the PHDR table and find the section of PT_LOAD type that contains the address of the entrypoint method in fib.c
    unsigned int epaddress=(*ehdr).e_entry;
    int phdrsize=(*ehdr).e_phentsize;
    int phnum=(*ehdr).e_phnum;
    void* startAddress;
    Elf32_Phdr currphdr;
    
  // 3. Allocate memory of the size "p_memsz" using mmap function 
        //    and then copy the segment content
    for (int i=0;i<phnum;i++){
        currphdr=phdr[i];
        if(currphdr.p_type==PT_LOAD && (epaddress>=currphdr.p_vaddr) && (epaddress<=(currphdr.p_vaddr+currphdr.p_memsz))){
            void* virtual_mem=mmap(NULL, currphdr.p_memsz,PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE,0,0);
            if (virtual_mem==MAP_FAILED) {
                printf("Error in mmap");
                free(heapmemalloc);
                exit(1);
            }

            memcpy(virtual_mem,heapmemalloc+currphdr.p_offset, currphdr.p_memsz);
            startAddress = virtual_mem+epaddress-currphdr.p_vaddr;
            break;
          
        
        }
    }

    // 5. Typecast the entry point address to a function pointer
    int (*_start)(void) = (int (*)(void))startAddress;
  // 6. Call the "_start" method and print the value returned from the "_start"
    int result = _start();
    printf("User _start return value = %d\n",result);
    free(heapmemalloc);
    close (fd);
}

  
int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  FILE* ELFfile=fopen(argv[1],"rb");
  if (!ELFfile){
    printf("Error in opening ELF file");
    exit(1);}
  fclose (ELFfile);
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(&argv[1]);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
