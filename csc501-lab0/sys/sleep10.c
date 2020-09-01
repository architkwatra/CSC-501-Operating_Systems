/* sleep10.c - sleep10 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>

#include "lab0.h"

extern int ctr1000;
extern int shouldRecord;

/*------------------------------------------------------------------------
 * sleep10  --  delay the caller for a time specified in tenths of seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep10(int n)
{	int start = ctr1000;
	STATWORD ps;    
	if (n < 0  || clkruns==0)
	         return(SYSERR);
	disable(ps);
	if (n == 0) {		/* sleep10(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*100);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
	 if (shouldRecord) {
                
		int pid = getpid();
                struct pentry *temp = &proctab[pid];
		struct sysCallSummary *ptr = &(temp->sysCalls)[19];
                strcpy(ptr->pName, "sleep10");
                ptr->pid = pid;
                ptr->frq++;
                ptr->totalTime = ptr->totalTime + ctr1000-start;
        }
	return(OK);
}
