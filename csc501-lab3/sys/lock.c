
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

int lock(int ldesc, int type, int lprio) {

    STATWORD ps;
    disable(ps);
    // Why LFREE returns an error (in semaphore)??
	if (isbadlock(ldesc) || (lptr = &rwlocks_tab[ldesc])->lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	
    int pid = getpid();

    if (lptr->lstate == DELETED ||  lptr->lstate == LFREE) {
        
        // basic lock initializations
        lptr->lstate = LUSED;
        // lptr->lcnt += 1;
        lptr->lprio = -1; // setting it to -1 because no proc is in the waitlist
        lptr->ltype = type;

        // adding the process to the processesholdinglock array
        lptr->processesholdinglock[pid] = 1;

        // adding the lock to the proctab bit mask
        proctab[pid].locksheld[ldesc] = 1;
        
    } else {

        if (lptr->lstate == READ) { // lock has already been acquired by a READER

            if (type == READ) { // the current proc is a also a READER


                // check for a higher priority WRITER process
                // if there, then add the READER proc to the wait list
                // else give the reader the lock
                int writeProc = getHigherWriteProcNumber(/* send head and tail as arguments */);

                if (writeProc == SYSERR || /* writeProc.prio < curproc.prio */) {
                    // give lock to the READER

                    // basic lock initializations
                    // lptr->lstate = LUSED; // This would already be done by the condition above
                    lptr->lcnt += 1;
                    lptr->lprio = -1; // setting it to -1 because no proc is in the waitlist
                    // lptr->ltype = type;

                    // adding the process to the processesholdinglock array
                    lptr->processesholdinglock[pid] = 1;

                    // adding the lock to the proctab bit mask
                    proctab[pid].locksheld[ldesc] = 1;

                } else {

                    // Make reader wait
                    (pptr = &proctab[currpid])->pstate = PRWAIT;
                    insert(currpid, lptr->lqhead, lprio);
                    
                    // Do we need to do this as well as lwaitret??
                    pptr->pwaitret = OK;
                    pptr->plockdesc = ldesc;
                    pptr->waittime = ctr1000;

                    lptr->lwaitret = OK;

                    // check for this logic. Check for the argument
                    lptr->lprio = getHighestPriorityFromWaitList(lptr);
                    // should we call resched?

                    // what to return??
                }

            } else if (type == WRITE) { // The current proc is a writer and it cannot acquire the lock
                
                // Make writer wait
                (pptr = &proctab[currpid])->pstate = PRWAIT;
                insert(currpid, lptr->lqhead, lprio);
                // Do we need to do this as well as lwaitret??
                pptr->pwaitret = OK;
                pptr->plockdesc = ldesc;
                pptr->waittime = ctr1000;

                lptr->lwaitret = OK;

                // check for this logic. Check for the argument
                lptr->lprio = getHighestPriorityFromWaitList(lptr);
                // should we call resched?

                // what to return??
            }
        }
    }


    int getHigherWriteProcNumber(int head, int tail) {

        h = &(q[q[head].qnext]);
        t = &q[tail]

        while (h != tail) {
            // get the type of the proc here i.e. READ or WRITE
            // and return the write proc's priority

            h = &q[h->qnext];
        }

        return SYSERR;
    }

    restore(ps);

}
/* int lock () { 

    if lock is avalable:
        do nothing
    else: 
        if pprio of lock P2 (process holding the lock) > P2.pprio (the requesting process) :
            do nothing
        else:
            increase the P2.pinh priority to P1.pinh if P2.pinh != 0 else P2.pprio
            Now take care of the transitivity problem
}
                */ 

int getHighestPriorityFromWaitList(struct pentry *lptr) {

    pid = q[lptr->qtail].qprev;
    prio = proctab[pid].pinh == 0 ? proctab[pid].pprio : proctab[pid].pinh;
    return prio;

}