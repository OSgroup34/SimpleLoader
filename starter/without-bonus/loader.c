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
void load_and_run_elf(char* exe) {
// 1. Load entire binary content into the memory from the ELF file.
    fd=open(exe, O_RDONLY);
    off_t size=lseek(fd,0,SEEK_END);
    //error handling
    if (size==-1){
        printf("error in file size\n");
        exit(1);
    }
    lseek(fd,0,SEEK_SET);
    //entire file
    char* heapmemalloc=(char*)malloc(size);
    //error handling
    if (!heapmemalloc){
        printf("error in memory allocation\n");
        exit(1);
    }
    ssize_t readfile=read(fd, heapmemalloc,size);

    //error handling
    if (readfile<0 || (size_t)readfile!=size){
    perror("error in reading file");
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
    
  // 3. Allocate memory of the size "p_memsz" using mmap function 
        //    and then copy the segment content
    for (int i=0;i<phnum;i++){
        if(phdr[i].p_type==PT_LOAD && ((*ehdr).e_entry>=phdr[i].p_vaddr) && ((*ehdr).e_entry<=(phdr[i].p_vaddr+phdr[i].p_memsz))){
            void *segment=mmap((void *)phdr[i].p_vaddr, phdr[i].p_memsz,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_ANONYMOUS,0,0);
            if (segment == MAP_FAILED) {
                perror("Error in mmap");
                free(heapmemalloc);
                exit(1);
            }

            memcpy(segment, heapmemalloc + phdr[i].p_offset, phdr[i].p_memsz);
          
        
        }
    }

    // 5. Typecast the entry point address to a function pointer
    void (*_start)();
    _start=(void (*)())epaddress;
  // 6. Call the "_start" method and print the value returned from the "_start"// 6. Call the entry point (typically "_start" in an ELF file)
    _start();  // Jump to the entry point and start execution

    free(heapmemalloc);  // Free the allocated memory
}

  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
 
    int result = _start();
    printf("User _start return value = %d\n",result);

}

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv[1]);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
