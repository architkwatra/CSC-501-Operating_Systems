/* lcreate.c - lcreate, newlock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
SYSCALL lcreate(int count)
{
	STATWORD ps;    
	int	lock;

	disable(ps);
	if ( count<0 || (lock=newlock())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	rwlocks_tab[lock].lcnt = count;
	/* lqhead and lqtail were initialized at system startup */
	restore(ps);
	return(lock);
}

/*------------------------------------------------------------------------
 * newlock  --  allocate an unused lock and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newlock()
{
	int	lock;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		lock=nextlock--;
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (rwlocks_tab[lock].lstate==LFREE) {
			rwlocks_tab[lock].lstate = LUSED;
			return(lock);
		}
	}
	return(SYSERR);
}
