/* receive.c - receive */

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
 *  receive  -  wait for a message and return it
 *------------------------------------------------------------------------
 */
SYSCALL	receive()
{	
	int start = ctr1000;
	STATWORD ps;    
	struct	pentry	*pptr;
	WORD	msg;

	disable(ps);
	pptr = &proctab[currpid];
	if ( !pptr->phasmsg ) {		/* if no message, wait for one	*/
		pptr->pstate = PRRECV;
		resched();
	}
	msg = pptr->pmsg;		/* retrieve message		*/
	pptr->phasmsg = FALSE;
	restore(ps);
	
	if (shouldRecord) {
                int pid = getpid();
		struct pentry *temp = &proctab[pid];
		struct sysCallSummary *ptr = &(temp->sysCalls)[6];
                strcpy(ptr->pName, "receive");
                ptr->pid = pid;
                ptr->frq++;
                ptr->totalTime = ptr->totalTime + ctr1000-start;
        }

	return(msg);
}
