/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */
SYSCALL resume(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	//kprintf("bad pid = %d and pstate = %d\n", pid, proctab[pid].pstate);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		//kprintf("\nRESUME.C ENCOUNTERED AN ERROR\n");
		restore(ps);
		return(SYSERR);
	}
	prio = pptr->pprio;
	// kprintf("\n PUTTING THE PROCESS IN THE READY QUEUE, with pid = %d\n", pid);
	ready(pid, RESCHYES);
	restore(ps);
	return(prio);
}
