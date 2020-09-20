/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "sched.h"

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);

/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{

	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/

	//if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   //(lastkey(rdytail)<optr->pprio)) {
		//return(OK);
	//}
	
	optr = &proctab[currpid];


	if (getschedclass() == EXPDISTSCHED) {

		if (optr->pstate == PRCURR) {
	              	optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
         	}			

		nptr = 	&proctab[currpid = scheduleNewProc()];	
		nptr->pstate = PRCURR;
	
	} else if (getschedclass() == LINUXSCHED) {
               	if (optr->pstate == PRCURR) {
			optr->counter--;
			optr->goodness--;

			if (optr->counter <= 0) {
				
				optr->goodness = 0;
				optr->counter = 0;

                        	optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->goodness);
				
				if (hasepochended()) {
					setVariables();	
				}

				nptr = &proctab[currpid = scheduleNewProc()];
                		nptr->pstate = PRCURR;
			
			} else 
				return (OK);
		} else {
			nptr = &proctab[currpid = scheduleNewProc()];
                	nptr->pstate = PRCURR;
		}
			
	} else {

		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) && 
			(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}	
		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;
	}

	#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);		
	/* The OLD process returns here when resumed. */
	return OK;
}




