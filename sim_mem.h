/*
 * SHIRAN GLASSER
 * 208608174
 -header file-

The program implements a memory simulator. In contrast to a real operating system, this simulation uses only one processes as the virtual memory and it will approach the physical memory when needed.  We use the "paging" principle that allow running a program when only a portion of it is in the memory.
The virtual memory is divided into pages, these pages are brought to the main memory by need, (the memory divided to frames at the pages size) and to the swap if more place needed.
  The simulation will be implemented by two main approaches, load and store.
Both methods will find the appropriate page to the given address and search the page on the main memory, if not found- will bring the whole page into the memory and then approaches it.
*/
using namespace std;

#ifndef SIM_MEM_H
#define SIM_MEM_H

#define MEMORY_SIZE 200
extern char main_memory[MEMORY_SIZE];
typedef struct page_descriptor
{
    int V; // valid
    int D; // dirty
    int P; // permission
    int frame; //the number of a frame if in case it is page-mapped
    int swap_index; // where the page is located in the swap file.
} page_descriptor;

class sim_mem {
    int swapfile_fd; //swap file fd
    int program_fd; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    page_descriptor *page_table; //pointer to page table
public:
    sim_mem(char exe_file_name[], char swap_file_name[], int text_size,
                     int data_size, int bss_size, int heap_stack_size,
                     int num_of_pages, int page_size);

    ~sim_mem();

    char load(int address); //returns the value in the given address (\0 if failed)
    void store(int address, char value); // writes the given value on the given address.

    void print_memory();

    void print_swap();

    void print_page_table();

    //help methods:

    /*the method assumes that the page is not on the main memory.
      it checks in which area the page located- (text, data, bss, heap or stack)
      brings the page into the memory and updates the valid sell on the page table
      returns \0 in case of error
     */
    char bringPage(int page, int address,int frame, char method);

    /* finds an empty frame in the memory to bring a page to
       if there isn't any- returns -1 */
    int findFrame();

    /* if there's no free frame to bring a page to, finds the frame that was in the
       last chronological use by the LRU algorithm. changes this frames current page to be not valid
       and rewriting on this frame the new page
       if the page that the frame represents is dirty(been changed), copies the page into the swap
       file to save the changes and updating the swap index on this page on the table
       returns the chosen frame */
    int clearMemory();

    /* the method finds an empty room on the swap file to write a page on. (page size number of zeros) */
    int findSwapIndex();
};
#endif //SIM_MEM_H
