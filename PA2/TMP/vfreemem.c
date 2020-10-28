/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include<paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	struct mblock *ptr = proctab[getpid()].vmemlist;
        
	STATWORD ps;
        struct  mblock  *p, *q;
        unsigned top;

        
        
        
        unsigned int maxadr = BACKING_STORE_BASE + proctab[getpid()].store*(BACKING_STORE_UNIT_SIZE) 
                +  proctab[getpid()].vhpnpages*NBPG;


        if (size==0 || (unsigned)block>(unsigned)maxaddr
            || ((unsigned)block) < ((unsigned) BACKING_STORE_BASE + proctab[getpid()].store*(BACKING_STORE_UNIT_SIZE)))
                return (SYSERR);
        size = (unsigned)roundmb(size);
        disable(ps);
        for( p = ptr->mnext,q= &ptr;
             p != (struct mblock *) NULL && p < block ;
             q=p,p=p->mnext )
                ;
        if (((top = q->mlen+(unsigned)q) > (unsigned)block && q!= &ptr) ||
            (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
                restore(ps);
                return(SYSERR);
        }
        if ( q!= &ptr && top == (unsigned)block )
                        q->mlen += size;
        else {
                block->mlen = size;
                block->mnext = p;
                q->mnext = block;
                q = block;
        }
        if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
                q->mlen += p->mlen;
                q->mnext = p->mnext;
        }
        restore(ps);
        return(OK);
		
}
