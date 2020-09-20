#include <conf.h>
#include <kernel.h>
#include <q.h>
#include "math.h"
#include <sched.h>
#include <proc.h>

int scheduledclass = 0;

void setschedclass(int sched_class) {
	scheduledclass = sched_class;
}

int getschedclass() {
	return scheduledclass;
}

int scheduleNewProc() {
        int randValue = (int)expdev(0.1);

        int     next = q[rdyhead].qnext;
        int     prev = q[rdytail].qprev;

        if (next == rdytail) {
                return dequeue(rdyhead);
        }
        if (scheduledclass < 2) {
                if (randValue < q[next].qkey) {

                        return (dequeue(next));
                } else if (randValue > q[prev].qkey){
                        return (dequeue(prev));
                } else {

                        while (randValue > q[next].qkey && next < rdytail) {
                                next = q[next].qnext;
                        }
                        return (dequeue(next));
                }

        } else {
                int goodness = 0;
                int proc = 0;
                while (next < rdytail) {
                        struct pentry *ptr = &proctab[next];
                        if (ptr->ispartofepoch == 1 && goodness < ptr->goodness) {
                                goodness = ptr->goodness;
                                proc = next;
                        }
                        next = q[next].qnext;
                }
                return dequeue(proc);
        }
}


int hasepochended() {
        int     next = q[rdyhead].qnext;
        int     prev = q[rdytail].qprev;
        if (next == rdytail)
                return (1);
        while (next < rdytail) {
                struct pentry *ptr = &proctab[next];
                if (ptr->ispartofepoch == 1 && ptr->counter > 0){
                        return(0);
                }
                next = q[next].qnext;
        }
        return 1;
}


void setVariables() {
	
	q[rdyhead].qnext = rdytail;
	q[rdytail].qprev = rdyhead;

	int pid = 0;
	while (pid < NPROC) {
		struct pentry *ptr = &proctab[pid];
		ptr->ispartofepoch = 0;
		if  (ptr->pstate != PRFREE) {
			ptr->ispartofepoch = 1;
			ptr->timequantum = (int)ptr->counter/2 + ptr->pprio;
			ptr->counter = ptr->timequantum;
                        ptr->goodness = ptr->counter + ptr->pprio;
			
			if (ptr->pstate == PRREADY)
				insert(pid,rdyhead,ptr->goodness);
		}
		pid++;
	}
}
