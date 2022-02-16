-MEMORY SIMULATOR-

Authored by Shiran Glasser 
208608174

==Description==
The program implements a memory simulator. In contrast to a real operating system, this simulation uses only one processes as the virtual memory and it will approach the physical memory when needed.  We use the "paging" principle that allow running a program when only a portion of it is in the memory.
The virtual memory is divided into pages, these pages are brought to the main memory by need, (the memory divided to frames at the pages size) and to the swap if more place needed.
  The simulation will be implemented by two main approaches, load and store.
Both methods will find the appropriate page to the given address and search the page on the main memory, if not found- will bring the whole page into the memory and then approaches it.   

==Program DATABASE: ==
1. main_memory=array of chars in size MEMORY_SIZE, this array simulate the RAM (random access memory).

2. page_descriptor- struct that contains the possible attributes of a page:
V- if it is valid-if located in the physical memory at the moment (1-valid, 0-not)
D-if it is a dirty page- if it changed in the past (1-dirty, 0-not)
         P-its permissions (0-read only, 1-read and write)
Frame- the mapping number of its memory frame in case it's in the RAM. 
swap_index- where the page is located in the swap file.

3. page_table- array of page_descriptor structs. the page table serves as a table of contents, where we can get all the information about each page

4. program_fd- file descriptor, holds the access to the executable file, this file simulate the virtual memory of a process. The first values in the file are for the text- the code, couldn’t be changed and the followed are for the data, can change in while running.

5.  swap_fd= file descriptor,hold the access to the swap file , the swap file simulate the Hard disk and will contain the pages that we cleared from the memory(when it was full) and we want to save the changes that they've had . 

6. Frames- struct that contains two numbers- count: will keep for each frame its time count of last usage in order to know which frame used last to clear it when room needed, and Fpage: will keep for each frame its page number to have access for it in the page table when clearing a frame and its valid needs to be changed.

7. frameStatus= an array of frames that contain for each frame its time of last usage and its page number.

==functions: ==
two main functions:
1. load- this method receives an address and ensure that the requested address will be in the main memory. If not-bring it from its right location. The method returns the char value in this address, in case of error- returns '\0'. 
Possible errors: invalid address, trying to load a from the stack\heap area for the first time.(there's nothing to load)

2. store- this method is very similar to "load" function, receive an address, and a value to store on this address. The method approaches the address page in the main memory and assigning the given value in the given address location on the memory.
  Possible errors: invalid address, trying to store a text page- the code can't be written on.  

 To get to a specific page the methods check:
- if its in the main memory, goes to the right frame on memory.
-if its not on memory its need to be brought from one of the areas (text, data, bss, heap or stack) 
-if the page not in memory but has been changed(dirty 1), its saved on the swap file and needs to be brought from there.

Functins to print the databases:
1. print_memory
2. print_swap
3. print_page_table

Help methods:
1. bringPage-the method assumes that the page is not on the main memory.
      it checks in which area the page located- (text, data, bss, heap or stack)
      brings the page into the memory and updates the valid sell on the page table
      returns \0 in case of error
2. findFrame- finds an empty frame in the memory to bring a page to
       if there isn't any- returns
3. clearMemory- if there's no free frame to bring a page to, finds the frame that was in the
       last chronological use by the LRU algorithm. changes this frames current page to be not valid
       and rewriting on this frame the new page
       if the page that the frame represents is dirty(been changed), copies the page into the swap
       file to save the changes and updating the swap index on this page on the table
       returns the chosen frame
4. findSwapIndex- the method finds an empty room on the swap file to write a page on. (page size number of zeros)

(*)Constructor- initializes the workplace. Opens the files, reseting the main memory and the swap and assigning and resetting the page table.
(*)Destructor- frees all the dete bases and closes the files.

==Program Files==
mem_sim.h- an header file ,contains a declaration of all the function data bases and the sizes. 
mem_sim.cpp- a source file- implements all the methods and initializing the attributes.
main.c- the tester for the program, creates the object of the process in the simulator.

==How to compile?== compile: g++ main.cpp sim_mem.cpp -o simMem run: ./simMem

==Input:== 
The executed file name, the swap file name, the text size in byts, the data size in byts,
the bss size in byts, the heap and stack size in byts, the number of pages in the virtual memory, and the size of each page and frame.

==Output:==
Possible outputs:
The main memory (RAM) 
The page table
The swap file
A char (that the load function returns).
