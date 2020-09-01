/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include "lab0.h"

extern int ctr1000;
extern int shouldRecord;


/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
SYSCALL	setnok(int nok, int pid)
{	
	int start = ctr1000;
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);
	if (shouldRecord) {
                int pid = getpid();
                struct pentry *temp = &proctab[pid];
		struct sysCallSummary *ptr = &(temp->sysCalls)[14];
                strcpy(ptr->pName, "setnok");
                ptr->pid = pid;
                ptr->frq++;
                ptr->totalTime = ptr->totalTime + ctr1000-start;
        }
	return(OK);
}
