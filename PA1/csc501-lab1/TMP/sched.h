#ifndef SCHED_H
#define SCHED_H

#define EXPDISTSCHED 1
#define LINUXSCHED 2

extern int scheduledclass;

extern void setschedclass(int sched_class);
extern int getschedclass();
extern int scheduleNewProc();
extern int hasepochended();
extern void setVariables();
#endif

