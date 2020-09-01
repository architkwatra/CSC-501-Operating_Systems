/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

#include "lab0.h"

extern int ctr1000;
extern int shouldRecord;

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */
SYSCALL resume(int pid)
{	
	int start = ctr1000;
	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		restore(ps);
		return(SYSERR);
	}
	prio = pptr->pprio;
	ready(pid, RESCHYES);
	restore(ps);
	if (shouldRecord) {
      		int pid = getpid(); 
		struct pentry *temp = &proctab[pid];
		struct sysCallSummary *ptr = &(temp->sysCalls)[9];
                strcpy(ptr->pName, "resume");
                ptr->pid = pid;
                ptr->frq++;
                ptr->totalTime = ptr->totalTime + ctr1000-start;
        }
	return(prio);
}
