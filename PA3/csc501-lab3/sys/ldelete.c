/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

#include <lock.h>

/*------------------------------------------------------------------------
 * ldelete  --  delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */
SYSCALL ldelete(int lock)
{
	STATWORD ps;    
	int	pid;
	struct	rwlock	*lptr;

	disable(ps);
	if (isbadlock(lock) || rwlocks_tab[lock].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &rwlocks_tab[lock];
	lptr->lstate = SFREE;
	if (nonempty(lptr->lqhead)) {
		while( (pid=getfirst(lptr->lqhead)) != EMPTY)
		  {
		    proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO);
		  }
		resched();
	}
	restore(ps);
	return(OK);
}
