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
		deleteCreatedTableData(pid);
		return SYSERR;
	}

	ptr->isPrivate = SET;
	int emptyStore = get_bsm(NULL);
	if (emptyStore == SYSERR) {
		deleteCreatedTableData(pid);	
		return SYSERR;
	}

	int vaddress = (int)procaddr>>12;
	int check = bsm_map(pid, vaddress, emptyStore, hsize);
	if (check == SYSERR)
		return SYSERR;

	bsm_tab[emptyStore].bs_isPrivate = 1;
	struct mblock *mptr;
	(proctab[pid].vmemlist)->mnext = (struct mblock*) (BACKING_STORE_BASE + emptyStore*BACKING_STORE_UNIT_SIZE);
	mptr = (proctab[pid].vmemlist)->mnext;
	proctab[pid].vhpnpages = hsize;
	mptr->mlen = hsize*NBPG;
	mptr->mnext = 0;
	
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

	int i = 0;
	while (i<NFRAMES) {
		if (frm_tab[i].fr_pid == pid)
			frm_tab[i].fr_status = UNSET;
		++i;
	}
}











