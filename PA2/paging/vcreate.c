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
	//get_bsm will return a free entry from bsm_tab by checking its status
	int freeStore = get_bsm(NULL);
	if (freeStore != -1) {
		//get_bs() returns any valid number if the freeStore(BS) has hsize (pages)
		//available else it return a SYSERR
		int backingStoreAllocation = get_bs(freeStore, hsize);
		if (backingStoreAllocation == SYSERR)
			return SYSERR;
		ptr->pid = pid;
		ptr->hsize = hsize;
	} else {
		return SYSERR;
	}
	restore(ps);
	return OK;
	
	//pseducode for the above logic
	/*1 getStoreId (free BS) from get_bs()
	2 If getStoreId != -1:
		store storeId in proctab
		update hsize in BS || or store hsize in proctab
	*/


		
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
