/*
 -source file-
*/
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;
#include "sim_mem.h"
char main_memory[MEMORY_SIZE];

typedef struct frames
{
    int count; //the time we used this frame last
    int Fpage; //the page number of this frame
} frames;

frames *frameStatus; //to keep the time and the page for each frame
int countTime; //counts the times we accessed the frames

//constructor
sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size,
                 int bss_size, int heap_stack_size, int num_of_pages, int page_size)
{
    countTime=0;
    frameStatus=(frames*) malloc((MEMORY_SIZE/page_size)*sizeof(frames));
    if(frameStatus==NULL)
    {
        perror("dynamic assigning failed");
        exit(1);
    }
    for (int i = 0; i < MEMORY_SIZE/page_size; ++i)
        frameStatus[i].count=-1; //marking all the frames as available

    //initialize all the programs data base:
    this->text_size=text_size;
    this->data_size=data_size;
    this->bss_size=bss_size;
    this->heap_stack_size=heap_stack_size;
    this->num_of_pages=num_of_pages;
    this->page_size=page_size;

    //assigning the executable file and creating the swap file:
    this->program_fd = open(exe_file_name, O_RDONLY, S_IRUSR| S_IWUSR| S_IRGRP| S_IROTH);
    if(program_fd == -1)
    {
        printf("cannot open executable file\n");
        free(frameStatus);
        exit(1);
    }
    this->swapfile_fd = open(swap_file_name, O_CREAT |O_RDWR, S_IRUSR| S_IWUSR| S_IRGRP| S_IROTH);
    if(swapfile_fd == -1)
    {
        printf("cannot open swap file\n");
        close(swapfile_fd);
        free(frameStatus);
        exit(1);
    }

    //resets the main memory:
    memset(main_memory, '0', MEMORY_SIZE);

    //assigning and resetting the page table:
    page_table=(page_descriptor*) malloc(num_of_pages*sizeof(page_descriptor));
    if (page_table==NULL)
    {
        perror("dynamic assigning failed");
        close(swapfile_fd);
        close(program_fd);
        free(frameStatus);
        exit(1);
    }
    int count=text_size/page_size;
    if(text_size%page_size>0)//if theres rest, needs one more page to cover all the text data
        count++;
    for (int i = 0; i <num_of_pages ; ++i)
    {
        page_table[i].V=0;
        page_table[i].frame=-1;
        page_table[i].D=0;
        page_table[i].swap_index=0;
        if(count>0)
        {
            page_table[i].P=0;  //text pages defined- read
            count--;
        }
        else
            page_table[i].P=1; //other pages defined- write
    }

    //resetting the swap file:
    int swap_size=page_size*(num_of_pages-text_size/page_size);
    char str[swap_size+1];
    for (int i = 0; i < swap_size; ++i)
        str[i]='0';
    str[swap_size]='\0';
    ssize_t test= write(swapfile_fd, str,swap_size);
    if (test==-1)
    {
        perror("writing to file failed");
        close(program_fd);
        close(swapfile_fd);
        free(page_table);
        free(frameStatus);
        exit(1);
    }
}
/**************************************************************************************/
char sim_mem::load(int address)
{
    int page= address/page_size;
    int offset=address%page_size;
    int frame;


    //page not valid
    if(page<0 || page>=num_of_pages)
    {
        std::cerr<<"address is not valid\n";
        return '\0';
    }

    //page in memory
    if(page_table[page].V==1)
    {
        frame=page_table[page].frame;
        frameStatus[frame].count=countTime;
        countTime++;
        frameStatus[frame].Fpage=page;
        return (main_memory[frame*page_size+offset]);
    }

    //else- page not in memory:
    frame=findFrame();
    if(frame<0)
        frame= clearMemory();

    char test= bringPage(page, address,frame,'l');
    if(test=='\0') //couldn't bring page
        return test;
    frameStatus[frame].count=countTime;
    countTime++;
    frameStatus[frame].Fpage=page;
    page_table[page].V=1;
    page_table[page].frame=frame;
    return main_memory[frame*page_size+offset];
}
/**************************************************************************************/
void sim_mem::store(int address, char value)
{
    int page= address/page_size;
    int offset=address%page_size;
    int frame;

    //page not valid
    if(page<0 || page>=num_of_pages)
    {
        std::cerr<<"address is not valid"<<endl;
        return;
    }

    //page in memory
    if(page_table[page].V==1)
    {
        int frame=page_table[page].frame;
        frameStatus[frame].count=countTime;
        countTime++;
        frameStatus[frame].Fpage=page;
        page_table[page].D=1;
        main_memory[frame*page_size+offset]=value;
        return;
    }

    //else- page not in memory, need to bring the file
    frame=findFrame();
    if(frame<0)
        frame= clearMemory();

    char test= bringPage(page, address,frame,'s');
    if(test=='\0') //couldn't bring page
        return;
    frameStatus[frame].count=countTime;
    countTime++;
    frameStatus[frame].Fpage=page;
    page_table[page].V=1;
    page_table[page].D=1;
    page_table[page].frame=frame;
    main_memory[frame*page_size+offset]=value;
}
/**************************************************************************************/
void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}
/************************************************************************************/
void sim_mem::print_swap() {
    char str[page_size+1];
    str[page_size]='\0';
    int i;
    printf("\n Swap memory\n");
    if(lseek(swapfile_fd, 0, SEEK_SET)==-1)      // go to the start of the file
    {
        perror("there was an error moving the pointer in lseek");
        close(program_fd);
        close(swapfile_fd);
        free(page_table);
        free(frameStatus);
        exit(1);
    }
    ssize_t test;
    do{
    test=read(swapfile_fd, str, this->page_size);
    if(test==-1)
    {
        perror("reading from file has failed");
        close(program_fd);
        close(swapfile_fd);
        free(page_table);
        free(frameStatus);
        free(str);
        exit(1);
    }

        for(i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }    while(test==page_size);
}
/***************************************************************************************/
void sim_mem::print_page_table() {
    int i;
    printf("\n page table \n");
    printf("page\t Valid\t Dirty\t Permission \t Frame\t Swap index\n");
    for(i = 0; i < num_of_pages; i++) {
        printf("%d\t [%d]\t  [%d]\t  [%d]\t  [%d]\t  [%d]\n",
               i,
               page_table[i].V,
               page_table[i].D,
               page_table[i].P,
               page_table[i].frame ,
               page_table[i].swap_index);
    }
}
/**************************************************************************************/
//destructor
sim_mem::~sim_mem ()
{
    close(program_fd);
    close(swapfile_fd);
    free(page_table);
    free(frameStatus);
}
/**************************************************************************************/

//help methods:

int sim_mem:: findSwapIndex()
{
    char str[page_size+1];
    char zeros[page_size+1];
    memset(zeros, '0', page_size);
    zeros[page_size]='\0';
    int test;
    int i=0;
    while(i < page_size*(num_of_pages-text_size/page_size))
    {
        if(lseek(swapfile_fd, i, SEEK_SET)==-1) // go to the current index in the swap
        {
            perror("there was an error moving the pointer in lseek\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }
        test=read(swapfile_fd, str, page_size);
        if (test==-1)
        {
            perror("reading from file failed\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }
        str[page_size]='\0';
        if(strncmp(str, zeros, page_size+1)==0)
            break;
        i+=page_size;
    }
    return i;
}

int sim_mem:: findFrame()
{
    for (int i = 0; i < MEMORY_SIZE/page_size; ++i)
        if(frameStatus[i].count==-1)
            return i;

    return -1;
}
/********************************************************/
int sim_mem:: clearMemory()
{
    //the method called only when theres no room in memory-all the frames status>=0

    int f=0; //starting from frame 0
    int min=frameStatus[0].count;
    //searching the frames and finding the min value to see which frame used lattes
    for (int i = 1; i < MEMORY_SIZE/page_size; ++i)
        if(frameStatus[i].count<min)
        {
            min=frameStatus[i].count;
            f=i;
        }

    if(page_table[frameStatus[f].Fpage].D==0)
        //the current's frame page isn't dirty-no need to back it up on swap
    {
        page_table[frameStatus[f].Fpage].V=0;
        return f;
    }

    //writing the chosen frame to the swap and updating the page table:
    int s=findSwapIndex();
    char str[page_size+1];
    for (int i = 0; i < page_size; ++i)
        str[i]=main_memory[f*page_size+i];
    str[page_size]='\0';
    if(lseek(swapfile_fd, s, SEEK_SET)==-1) // go to the empty room in the swap
    {
        perror("there was an error moving the pointer in lseek\n");
        close(program_fd);
        close(swapfile_fd);
        free(page_table);
        free(frameStatus);
        exit(1);
    }
    ssize_t test= write(swapfile_fd, str, page_size);
    if (test==-1)
    {
        perror("writing to file failed\n");
        close(program_fd);
        close(swapfile_fd);
        free(page_table);
        free(frameStatus);
        exit(1);
    }
    page_table[frameStatus[f].Fpage].V=0;
    page_table[frameStatus[f].Fpage].swap_index=s;

    return f; //returns the number of the frame to rewrite on, choosing by the LRU algorithm
}
/********************************************************/
char sim_mem:: bringPage(int page, int address,int frame, char method)
{
    ssize_t test;
    char str[page_size+1];
    str[page_size]='\0';

    //case (1)-the page is the program
    if(address<text_size)
    {
        if(method=='s') //cant store in text page
        {
            std::cerr << "cannot change the text pages\n";
            return '\0';
        }
        //else load from text:
        if(lseek(program_fd, page*page_size, SEEK_SET)==-1) // go to the start of the page in the file
        {
            perror("there was an error moving the pointer in lseek\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }
        //bringing page from the file to the memory
        test= read(program_fd,str,page_size);
        if (test==-1)
        {
            perror("reading from file failed\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }

        for (int i = 0; i < page_size; ++i)
            main_memory[frame*page_size+i]= str[i];

        return '1';
    }

//case (2)- dirty page, in the swap
    if(page_table[page].D==1)
    {
        if (lseek(swapfile_fd, page_table[page].swap_index, SEEK_SET) == -1)
        { // go to the start of the page in the swap
            perror("there was an error moving the pointer in lseek\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }
        //bringing page from the file to the memory
        test= read(swapfile_fd, str, page_size);
        if (test==-1)
        {
            perror("reading from file failed\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }

        for (int i = 0; i < page_size; ++i)
            main_memory[frame*page_size+ i] = str[i];

        //resetting the page we brought from the swap
        if (lseek(swapfile_fd, page_table[page].swap_index, SEEK_SET) == -1)
        { // go to the start of the page in the swap
            perror("there was an error moving the pointer in lseek\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }
        memset(str, '0', page_size);
        test= write(swapfile_fd, str, page_size);
        if (test==-1)
        {
            perror("writing to file failed\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }
        return '1';
    }

    //case (3)-data
    if(address<text_size+data_size)
    {
        if (lseek(program_fd, page*page_size, SEEK_SET) == -1) // go to the start of the data in the file
        {
            perror("there was an error moving the pointer in lseek\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }

        test= read(program_fd, str, page_size);
        if (test==-1)
        {
            perror("reading from file failed\n");
            close(program_fd);
            close(swapfile_fd);
            free(page_table);
            free(frameStatus);
            exit(1);
        }
        for (int i = 0; i < page_size; ++i) //bringing page from the program file to the memory
            main_memory[frame*page_size+ i] = str[i];

        return '1';
    }

    //case (4)-heap\stack\bss
    if ((method == 'l') && (address >= text_size + data_size + bss_size)) {
        std::cerr<<"heap/stack cannot be load for the first time\n";
        return '\0';
    }
    //else-assigning an empty page
    for (int i = 0; i < page_size; ++i)
        main_memory[frame*page_size+i]='0';
    return '1';
}

