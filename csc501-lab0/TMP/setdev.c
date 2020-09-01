/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>

#include <q.h>
#include <stdio.h>
#include "lab0.h"

extern int ctr1000;
extern int shouldRecord;


/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
SYSCALL	setdev(int pid, int dev1, int dev2)
{	
	int start = ctr1000;
	short	*nxtdev;
	if (isbadpid(pid))
		return(SYSERR);
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;
	if (shouldRecord) {
                int pid = getpid();
                struct pentry *temp = &proctab[pid];
		struct sysCallSummary *ptr = &(temp->sysCalls)[13];
                strcpy(ptr->pName, "setdev");
                ptr->pid = pid;
                ptr->frq++;
                ptr->totalTime = ptr->totalTime + ctr1000-start;
        }
	return(OK);
}
