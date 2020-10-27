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
	if (pid == SYSERR) {
		kprintf("FAIL 1\n");
		deleteCreatedTableData(pid);
		return SYSERR;
	}
	//used in xmmap
	ptr->isPrivate = 1;

	//get_bsm will return a free entry from bsm_tab by checking its status
	int freeStore = get_bsm(NULL);
	if (freeStore != SYSERR) {
		bsm_map(pid, (int)procaddr>>12, freeStore, hsize);
		bsm_tab[freeStore].bs_isPrivate = 1;
		struct mblock *mptr;


		(proctab[pid].vmemlist)->mnext = mptr = (struct mblock*) BACKING_STORE_BASE + freeStore*BACKING_STORE_UNIT_SIZE;
		proctab[pid].vhpnpages = hsize;
		mptr->mlen = hsize*NBPG;
		mptr->mnext = 0;
		
	} else {
		deleteCreatedTableData(pid);	
		return SYSERR;
	}
	restore(ps);
	return pid;

		
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

void deleteCreatedTableData(int pid) {

	int i;
	for (i=0; i<NFRAMES; ++i) {
		if (frm_tab[i].fr_pid == pid)
			frm_tab[i].fr_status = 0;
	}

}











