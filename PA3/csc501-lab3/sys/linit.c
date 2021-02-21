


int linit() {

    struct	rwlock	*lptr;
    nextlock = NLOCKS-1;

    int i;
    for (i=0 ; i<NLOCKS ; i++) {	/* initialize locks */
      (lptr = &rwlock_tab[i])->lstate = LFREE;
      lptr->lqtail = 1 + (lptr->lqhead = newqueue());
	}
}