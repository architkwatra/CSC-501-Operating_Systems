

/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"
//#include "lab0.h"
static unsigned long    *esp;
static unsigned long    *ebp;

extern void printtos();
extern void printsegaddress();
extern void printprocstks();
extern void syscallsummary_start();
extern void syscallsummary_stop();
extern void printsyscallsummary();
extern long zfunction();

extern int ctr1000;

int prX;
void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */

prch(c)
char c;
{
	int i;
	sleep(5);
}

int main()
{	
	long zFunctionRet = zfunction(0xaafdccee);
	kprintf("\n\nvoid zfunction()\n");
	kprintf("0x%X\n\n", zFunctionRet);	
	
	kprintf("void printsegaddress()\n");
	printsegaddress();
	
	kprintf("\nvoid printtos()");
	printtos();
	
	kprintf("\n\nvoid printprocstks()\n");
	printprocstks(3);    	
	
	kprintf("\n\nvoid printsyscallsummary() \n");
	syscallsummary_start();	
	resume(prX = create(prch,2000,20,"proc X",1,'A'));
	sleep(10);
	syscallsummary_stop();
	printsyscallsummary();
	//kprintf("\nHere is the sysCalls array name value for 0th element = %s\n", sysCalls[]);
	
	kprintf("\n\nHello World, Xinu livesss\n\n");
	return 0;  
}

