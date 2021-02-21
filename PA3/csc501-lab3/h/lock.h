/* lock.h */

#ifndef _LOCK_H_
#define _LOCK_H_

#define READ 0
#define WRITE 1

#ifndef	NLOCKS
#define	NLOCKS		50	/* number of LOCKS, if not defined	*/
#endif

#define	LFREE	'\01'		/* this lock is free		*/
#define	LUSED	'\02'		/* this lock is used		*/

struct	rwlock	{

    char lstate;		/* the state LFREE or LUSED		*/
	int	lcnt;		/* count for this semaphore		*/
	int	lqhead;		/* q index of head of list		*/
	int	lqtail;		/* q index of tail of list		*/ 
	int lwaitret;
	
	int lprio; // maximum priority among all the processes waiting in the lock's wait queue
	int processesholdinglock[NPROC];
};

// this could be an array. Not sure right now
extern	struct	rwlock	rwlocks_tab;
extern	int	nextlock;

#define	isbadlock(s)	(s<0 || s>=NLOCKS)

#endif
