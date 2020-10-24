/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;

	int pid = create(procaddr,ssize,priority,name,nargs,args);
	disable(ps);
	struct pentry *ptr = &proctab[pid];
	
	//used in xmmap
	ptr->isPrivate = 1;

	//get_bsm will return a free entry from bsm_tab by checking its status
	int freeStore = get_bsm(NULL);
	if (freeStore != SYSERR) {
		
		bsm_map(pid, procaddr>>12, freeStore, hsize);
		bsm_tab[freeStore].bs_prvt = 1;
		
		(proctab[pid].vmemlist)->mnext = (mblock*) BACKING_STORE_BASE + freeStore*BACKING_STORE_UNIT_SIZE;
		proctab[pid].vmemlist->vhpnpages = hsize;
		proctab[pid].vmemlist->mlen = hsize*NBPG;
		
		ptr->store = freeStore;
		ptr->hsize = hsize;
		
	} else {
		return SYSERR;
	}
	restore(ps);
	return OK;

		
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
