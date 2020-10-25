/* initialize.c - nulluser, sizmem, sysinit */

#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <paging.h>

/*#define DETAIL */
#define HOLESIZE	(600)	
#define	HOLESTART	(640 * 1024)
#define	HOLEEND		((1024 + HOLESIZE) * 1024)  
/* Extra 600 for bootp loading, and monitor */

extern	int	main();	/* address of user's main prog	*/

extern	int	start();

LOCAL		sysinit();

/* Declarations of major kernel variables */
struct	pentry	proctab[NPROC]; /* process table			*/
int	nextproc;		/* next process slot to use in create	*/
struct	sentry	semaph[NSEM];	/* semaphore table			*/
int	nextsem;		/* next sempahore slot to use in screate*/
struct	qent	q[NQENT];	/* q table (see queue.c)		*/
int	nextqueue;		/* next slot in q structure to use	*/
char	*maxaddr;		/* max memory address (set by sizmem)	*/
struct	mblock	memlist;	/* list of free memory blocks		*/


struct scq scqhead;
struct scq *scPointer = &scqhead;
//scPointer->next = NULL;
struct fifo fifohead;

bs_map_t bsm_tab[8];
fr_map_t frm_tab[NFRAMES];



#ifdef	Ntty
struct  tty     tty[Ntty];	/* SLU buffers and mode control		*/
#endif


/* active system status */
int	numproc;		/* number of live user processes	*/
int	currpid;		/* id of currently running process	*/
int	reboot = 0;		/* non-zero after first boot		*/

int	rdyhead,rdytail;	/* head/tail of ready list (q indicies)	*/
char 	vers[80];
int	console_dev;		/* the console device			*/

/*  added for the demand paging */
int page_replace_policy = SC;
//int page_replace_policy = AGING;

int frm = 0;




static unsigned long *eax;

int markPTENonExistent(int frameNumber) {

	int vpn = frm_tab[frameNumber].fr_vpno;
	unsigned long pdbr = proctab[frm_tab[frameNumber].fr_pid].pdbr;
	unsigned long ptNumber = vpn >> 10;
	unsigned long pageNumber = (vpn << 10) >> 10;
	pd_t *pdePtr = (pt_t*)(pdbr + sizeof(pt_t)*ptNumber);
	pt_t *ptePointer = pdePtr->pd_base*4096 + sizeof(pt_t)*pageNumber;
	ptePointer->pt_pres = 0;
	
	if (getpid() == frm_tab[frameNumber].fr_pid) {
		//static unsigned long *eax;
		eax = vpn*NBPG;
		asm("invlpg eax");	
	}
	
	--frm_tab[frameNumber].fr_refcnt;
	if (frm_tab[frameNumber].fr_refcnt == 0)
		pdePtr->pd_pres = 0; 
	return OK;
}

int isAccSet(int idx) {
	int vpn = frm_tab[idx].fr_vpno;
	
	unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vpn>>10;
        unsigned long pageNumber = (vpn<<10)>>10;
        unsigned long pdeAddress = pdbr + 4*ptNumber;
	pd_t *pdePtr = (pd_t*) pdeAddress;
	
	unsigned int pt =  pdePtr->pd_base*NBPG;
	pt_t *ptePointer = (pt_t*) pt + 4*pageNumber;
	if (ptePointer->pt_acc == 0) {
		return idx;
	}
	
	ptePointer->pt_acc = 0;
	return -1;
	
}


int markIfDirty(int idx) {
        int vpn = frm_tab[idx].fr_vpno;

        unsigned long pdbr = proctab[frm_tab[idx].fr_pid].pdbr;
        unsigned long ptNumber = vpn>>10;
        unsigned long pageNumber = (vpn<<10)>>10;

        unsigned long pdeAddress = pdbr + 4*ptNumber;
        pd_t *pdePtr = (pd_t*) pdeAddress;

        unsigned int pt =  pdePtr->pd_base*NBPG;
        pt_t *ptePointer = (pt_t*) pt + 4*pageNumber;
        if (ptePointer->pt_dirty == 1) {
		frm_tab[idx].fr_dirty = 1;
        }
	else
		frm_tab[idx].fr_dirty = 0;

        return OK;

}


int writeBackDirtyFrames(int pid) {

	int i = 0;
	while (i < NFRAMES) {
		markIfDirty(i);
		if (frm_tab[i].fr_pid == pid && frm_tab[i].fr_dirty == 1) {
			if (writeDirtyFrame(i) == SYSERR) {
				return SYSERR;
			}
		}
		i++;
	}
	return OK;
}

int writeDirtyFrame(int i) {

		int store, pageth; 
                if (bsm_lookup(frm_tab[i].fr_pid, frm_tab[i].fr_vpno*NBPG /*(vaddr)*/, &store, &pageth) == SYSERR) {
                        kill(frm_tab[i].fr_pid);
                        return SYSERR;
                }   
                char *pointerToSrc = (FRAME0 + i)*NBPG;
                write_bs(pointerToSrc, &store, &pageth);
                
                frm_tab[i].fr_dirty = 0;
		return OK;
}


int removeFramesOnKill(int pid) {

	struct scq *p = &scqhead;
	struct scq *q = p->next;
	
	while (1) {
		if (q == &scqhead) {
			return OK;
		}

		if (frm_tab[q->idx].fr_pid == pid) {
			q = q->next;
			p->next = q;
		}
		else {
			p = p->next;
			q = q->next;
		}		
	}
	return SYSERR;

}













/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED, and      ***/
/***   must eventually be enabled explicitly.  This routine turns     ***/
/***   itself into the null process after initialization.  Because    ***/
/***   the null process must always remain ready to run, it cannot    ***/
/***   execute code that might cause it to be suspended, wait for a   ***/
/***   semaphore, or put to sleep, or exit.  In particular, it must   ***/
/***   not do I/O unless it uses kprintf for polled output.           ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  nulluser  -- initialize system and become the null process (id==0)
 *------------------------------------------------------------------------
 */
nulluser()				/* babysit CPU when no one is home */
{
        int userpid;

	console_dev = SERIAL0;		/* set console to COM0 */
	
	initevec();

	kprintf("system running up!\n");
	sysinit();

	enable();		/* enable interrupts */

	sprintf(vers, "PC Xinu %s", VERSION);
	kprintf("\n\n%s\n", vers);
	if (reboot++ < 1)
		kprintf("\n");
	else
		kprintf("   (reboot %d)\n", reboot);


	kprintf("%d bytes real mem\n",
		(unsigned long) maxaddr+1);
#ifdef DETAIL	
	kprintf("    %d", (unsigned long) 0);
	kprintf(" to %d\n", (unsigned long) (maxaddr) );
#endif	

	kprintf("%d bytes Xinu code\n",
		(unsigned long) ((unsigned long) &end - (unsigned long) start));
#ifdef DETAIL	
	kprintf("    %d", (unsigned long) start);
	kprintf(" to %d\n", (unsigned long) &end );
#endif

#ifdef DETAIL	
	kprintf("%d bytes user stack/heap space\n",
		(unsigned long) ((unsigned long) maxaddr - (unsigned long) &end));
	kprintf("    %d", (unsigned long) &end);
	kprintf(" to %d\n", (unsigned long) maxaddr);
#endif	
	
	kprintf("clock %sabled\n", clkruns == 1?"en":"dis");


	/* create a process to execute the user's main program */
	userpid = create(main,INITSTK,INITPRIO,INITNAME,INITARGS);
	kprintf("RESUMING MAIN");
	resume(userpid);

	while (TRUE)
		/* empty */;
}

/*------------------------------------------------------------------------
 *  sysinit  --  initialize all Xinu data structeres and devices
 *------------------------------------------------------------------------
 */
LOCAL
sysinit()
{
	static	long	currsp;
	int	i,j;
	struct	pentry	*pptr;
	struct	sentry	*sptr;
	struct	mblock	*mptr;
	SYSCALL pfintr();

	numproc = 0;			/* initialize system variables */
	nextproc = NPROC-1;
	nextsem = NSEM-1;
	nextqueue = NPROC;		/* q[0..NPROC-1] are processes */

	/* initialize free memory list */
	/* PC version has to pre-allocate 640K-1024K "hole" */
	if (maxaddr+1 > HOLESTART) {
		memlist.mnext = mptr = (struct mblock *) roundmb(&end);
		mptr->mnext = (struct mblock *)HOLEEND;
		mptr->mlen = (int) truncew(((unsigned) HOLESTART -
	     		 (unsigned)&end));
        mptr->mlen -= 4;

		mptr = (struct mblock *) HOLEEND;
		mptr->mnext = 0;
		mptr->mlen = (int) truncew((unsigned)maxaddr - HOLEEND -
	      		NULLSTK);
/*
		mptr->mlen = (int) truncew((unsigned)maxaddr - (4096 - 1024 ) *  4096 - HOLEEND - NULLSTK);
*/
	} else {
		/* initialize free memory list */
		memlist.mnext = mptr = (struct mblock *) roundmb(&end);
		mptr->mnext = 0;
		mptr->mlen = (int) truncew((unsigned)maxaddr - (int)&end -
			NULLSTK);
	}
	

	for (i=0 ; i<NPROC ; i++)	/* initialize process table */
		proctab[i].pstate = PRFREE;


#ifdef	MEMMARK
	_mkinit();			/* initialize memory marking */
#endif

#ifdef	RTCLOCK
	clkinit();			/* initialize r.t.clock	*/
#endif

	mon_init();     /* init monitor */

#ifdef NDEVS
	for (i=0 ; i<NDEVS ; i++ ) {	    
	    init_dev(i);
	}
#endif

	pptr = &proctab[NULLPROC];	/* initialize null process entry */
	pptr->pstate = PRCURR;
	for (j=0; j<7; j++)
		pptr->pname[j] = "prnull"[j];
	pptr->plimit = (WORD)(maxaddr + 1) - NULLSTK;
	pptr->pbase = (WORD) maxaddr - 3;
/*
	pptr->plimit = (WORD)(maxaddr + 1) - NULLSTK - (4096 - 1024 )*4096;
	pptr->pbase = (WORD) maxaddr - 3 - (4096-1024)*4096;
*/
	pptr->pesp = pptr->pbase-4;	/* for stkchk; rewritten before used */
	*( (int *)pptr->pbase ) = MAGIC;
	pptr->paddr = (WORD) nulluser;
	pptr->pargs = 0;
	pptr->pprio = 0;

	

	currpid = NULLPROC;

	for (i=0 ; i<NSEM ; i++) {	/* initialize semaphores */
		(sptr = &semaph[i])->sstate = SFREE;
		sptr->sqtail = 1 + (sptr->sqhead = newqueue());
	}

	rdytail = 1 + (rdyhead=newqueue());/* initialize ready list */	
	
	

	kprintf("check if the init_frm() needs to be called with 'SYSCALL'\n");
	init_frm();
	init_bsm();	


	// creating global page tables
	// check the logic for pageNumber
	
	i = 0;
	j = 0;
	//int pageNumber: 20;
	int globalPagePFN = 0;
	kprintf("Setting the global page tables\n");
	for (i=0; i<20; ++i) {
		kprintf("frm_tab number = %d", i);
		kprintf("\nfrm_tab[i].fr_status = %d, frm_tab[i].fr_pid = %d, frm_tab[i].fr_type = %d\n", frm_tab[i].fr_status, frm_tab[i].fr_pid, frm_tab[i].fr_type);
	}
	kprintf("\nXXXXXXXXXXXXXXXXXXXXXXXX\n");
	i = 0;
	j = 0;
	while (i < 4) {
		//i+1 is done because 1025th frame is used for 
		//the page table. 1024th frame is used for the 
		//page directory. frm_tab is the frame array 
		//each having 1024 entries
		
		frm_tab[i+1].fr_status = 1;
		frm_tab[i+1].fr_type = FR_TBL;
		frm_tab[i+1].fr_pid = NULLPROC;

		pt_t *pageTableEntry = (FRAME0 + i + 1)*NBPG;

		while (j < 1024) {
			
			pageTableEntry->pt_pres = 1;
			pageTableEntry->pt_write = 1;
			pageTableEntry->pt_pwt = 0;
			pageTableEntry->pt_user = 0;
			pageTableEntry->pt_pcd = 0;
			pageTableEntry->pt_dirty = 0;
			pageTableEntry->pt_mbz = 0;
			pageTableEntry->pt_global = 1;
			pageTableEntry->pt_avail = 0;
			//check this logic
			pageTableEntry->pt_base = i*FRAME0 + j;
			j++;
			pageTableEntry++;
		}
		i++;
	}
	

	// adding pdbr for nulluser which is pointing at the 1024th frame/page
	kprintf("Setting the page directory for the null process\n");
	
	proctab[NULLPROC].pdbr = FRAME0*NBPG;

	frm_tab[0].fr_status = 1;
	frm_tab[0].fr_pid = NULLPROC;
	frm_tab[0].fr_type = FR_DIR;
	
	//setting the pdbr for the NULL proc
	pptr->pdbr = FRAME0*NBPG;
	pd_t *ptr = pptr->pdbr;
	
	i = 1;
	while (i < 5) {
		ptr->pd_pres = 1;
		//pd_write = 1 means that the page is not writable
		ptr->pd_write = 1;
		ptr->pd_base = (i + FRAME0);
		ptr++;
		i++;
		//check if other bits need to be set or not.
	}
	
	write_cr3(proctab[NULLPROC].pdbr);
	kprintf("\nFINISHED NULLPROC PD and called write_cr3() register in initialize.c\n");
	set_evec(14, (u_long)pfintr);
	kprintf("\nset_evec() called in initilize.c\n");
	enable_paging();	
	kprintf("\nenable_paging FINISHED\n");
	return(OK);
}

stop(s)
char	*s;
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* empty */;
}

delay(n)
int	n;
{
	DELAY(n);
}


#define	NBPG	4096

/*------------------------------------------------------------------------
 * sizmem - return memory size (in pages)
 *------------------------------------------------------------------------
 */
long sizmem()
{
	unsigned char	*ptr, *start, stmp, tmp;
	int		npages;

	/* at least now its hacked to return
	   the right value for the Xinu lab backends (16 MB) */

	return 4096; 

	start = ptr = 0;
	npages = 0;
	stmp = *start;
	while (1) {
		tmp = *ptr;
		*ptr = 0xA5;
		if (*ptr != 0xA5)
			break;
		*ptr = tmp;
		++npages;
		ptr += NBPG;
		if ((int)ptr == HOLESTART) {	/* skip I/O pages */
			npages += (1024-640)/4;
			ptr = (unsigned char *)HOLEEND;
		}
	}
	return npages;
}
