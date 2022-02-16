/*
 -tester-

*/

#include <iostream>
#include "sim_mem.h"


int main() {
    char val;
    sim_mem mem_sm((char*)"exec_file", (char*)"swap_file" ,25,
                   50, 25,25, 25, 5);

    mem_sm.store( 29,'X');
    mem_sm.store( 34,'Y');
    mem_sm.store( 39,'Z');
    mem_sm.store( 44,'W');
    mem_sm.store( 49,'Q');
    mem_sm.store( 54,'M');
    mem_sm.store( 59,'A');
    mem_sm.store( 32,'B');
    mem_sm.store( 64,'C');
    val = mem_sm.load ( 78);
    val = mem_sm.load ( 105);


//
//    val = mem_sm.load ( 2);
//    val=mem_sm.load ( 70);
//    mem_sm.store( 96,'q');
//    mem_sm.store( 20,'z');
//    val=mem_sm.load ( 10);
//    val=mem_sm.load ( 12);
//   val=mem_sm.load ( 42);
//    val=mem_sm.load ( 96);
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();

    return 0;
}
