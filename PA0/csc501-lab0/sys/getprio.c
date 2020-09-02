/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include "lab0.h"
#include <i386.h>
#include <q.h>

extern int ctr1000;
extern int shouldRecord;


/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */
SYSCALL getprio(int pid)
{	
	int start = ctr1000;

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	if (shouldRecord) {
		int pid = getpid();
		struct pentry *temp = &proctab[pid];
                struct sysCallSummary *ptr = &(temp->sysCalls)[3];
                strcpy(ptr->pName, "getprio");
                ptr->pid = pid;
                ptr->frq++;
                ptr->totalTime = ptr->totalTime + ctr1000-start;
        }	
	return(pptr->pprio);
}
