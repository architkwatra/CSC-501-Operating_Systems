/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include "lab0.h"

extern int ctr1000;
extern int shouldRecord;

//#include <string.h>
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{	
	int start = ctr1000;
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);
	if (shouldRecord) {
                struct pentry *temp = &proctab[currpid];
		struct sysCallSummary *ptr = &(temp->sysCalls)[1];
                //kprintf("\n\nInside the CHPRIO, with old pid = %d\n", ptr->pid);
              	strcpy(ptr->pName, "chprio");
		ptr->pid = getpid();
                //kprintf("currpid or new pid = %d\n\n", ptr->pid);
                ptr->frq++;
                ptr->totalTime = ptr->totalTime + ctr1000-start;
        }
	return(newprio);
}
