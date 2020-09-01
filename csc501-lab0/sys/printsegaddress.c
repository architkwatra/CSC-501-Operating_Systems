

#include<stdio.h>
extern int etext, edata, end;
void printsegaddress() {
	
	int *a = &etext;
	int *b = &edata;
	int *c = &end;
	
	printf("Current: extext[0x%x]=0x%x, edata[0x%x]=0x%x, ebss[0x%x]=0x%x\n", a, *a, b, *b, c, *c);
	--a, --b, --c;
	printf("Preceding: extext[0x%x]=0x%x, edata[0x%x]=0x%x, ebss[0x%x]=0x%x\n", a, *a, b, *b, c, *c);
	++a, ++b, ++c;
	++a, ++b, ++c; 
	printf("After: extext[0x%x]=0x%x, edata[0x%x]=0x%x, ebss[0x%x]=0x%x\n\n", a, *a, b, *b, c, *c);

}
