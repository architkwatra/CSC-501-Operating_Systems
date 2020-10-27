/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
        //check if this is required or not
	//struct mblock *ptr = &(proctab[currpid]->vmemlist);
		
	//kprintf("Inside vgetmem()\n");
	STATWORD ps;
        struct  mblock  *p, *q, *leftover;

        disable(ps);
        if (nbytes==0 || proctab[getpid()].vmemlist->mnext== (struct mblock *) NULL) {
                restore(ps);
		//kprintf("111 ----- Returning SYSERR from vgetmen\n");
                return( (WORD *)SYSERR);
        }
	//kprintf("vmemlist points at %x\n", proctab[getpid()].vmemlist->mnext);
        nbytes = (unsigned int) roundmb(nbytes);
        for (q = proctab[getpid()].vmemlist, p=(proctab[getpid()].vmemlist->mnext) ;
             p != (struct mblock *) NULL ;
             q=p,p=p->mnext)
                if ( p->mlen == nbytes) {
                        q->mnext = p->mnext;
                        restore(ps);
			//kprintf("11111---Returning %x from vgetmen \n", (WORD*)p);
                        return( (WORD *)p );
                } else if ( p->mlen > nbytes ) {
                        leftover = (struct mblock *)( (unsigned)p + nbytes );
                        q->mnext = leftover;
                        leftover->mnext = p->mnext;
                        leftover->mlen = p->mlen - nbytes;
                        restore(ps);
			//kprintf("2222 ---- Returning %x from vgetmen\n", (WORD*)p);
                        return( (WORD *)p );
                }
        restore(ps);
	//kprintf("Returning SYSERR from vgetmen\n");
        return( (WORD *)SYSERR );
}


