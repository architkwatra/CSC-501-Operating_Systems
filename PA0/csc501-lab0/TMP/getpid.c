/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"
#include <stdio.h>
#include <i386.h>
#include <q.h>

extern int ctr1000;
extern int shouldRecord;

/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */

SYSCALL getpid()
{	
	int start = ctr1000;
	if (shouldRecord) {
		struct pentry *ptr = &proctab[currpid];
                //struct sysCallSummary *ptr = temp->sysCalls[2];
                //kprintf("\n\n\n\nInside the GETPID, with old pid = %d\n", ptr->pid);
                strcpy(ptr->sysCalls[2].pName, "getpid");
               	//kprintf("IS PROCTAB ACCESSIBLE?? %d\n"); 
		ptr->sysCalls[2].pid = currpid;
                //kprintf("currpid or new pid = %d\n\n", ptr->pid);
                ptr->sysCalls[2].frq++;
                ptr->sysCalls[2].totalTime = ptr->sysCalls[2].totalTime + ctr1000-start;
                //kprintf("Here is the pName = %s\n", ptr->pName);
        }

	return(currpid);
}
