


/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */

int removeProcPages(int pid) {

	struct scPolicy *slow = &scPolicyHead, *fast = (&scPolicyHead)->next;
	int i = 0;
	if (q == NULL) {
		// kprintf("No node in the policy queue\n");
		i++;
	}

	slow = &scPolicyHead;
    fast = slow->next;
	i = 0;
	while (i < 1024 && fast != &scPolicyHead) {
		if (fast == NULL)
			return SYSERR;

		if (frm_tab[fast->frame].fr_pid == pid) {
			fast = fast->next;
			slow->next = fast;
			frm_tab[pid].fr_status = 0;
		}
		else {
			slow = slow->next;
			fast = fast->next;
		}
		++i;	
		
		if (fast == &scPolicyHead) {
			return OK;
		}	
	}
	return OK;

}

SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}

	removeProcPages(pid);	
	release_bs(proctab[pid].store);	
	restore(ps);
	return(OK);
}


