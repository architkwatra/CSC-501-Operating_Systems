#ifndef LAB0_H
#define LAB0_H

extern int shouldRecord;

struct  sysCallSummary  {
	char pName[20];
	int pid;
	int frq;
	int totalTime;
	

};


extern struct  sysCallSummary sysCalls[27];

#endif



