#include<stdio.h>

extern int etext, edata, end;

int printsegaddress(void) {

    int *a = &_etext;
    int *b = &_edata;
    int *c = &_end;

    printf("Current: etext[%p]=%x, edata[%p]=%x, ebss[%p]=%x\n", a, *a, b, *b, c, *c);    
    printf("Preceding: etext[%p]=%x, edata[%p]=%x, ebss[%p]=%x\n", a-1, *(a-1), b-1, *(b-1), c-1, *(c-1));
    printf("After: etext[%p]=%x, edata[%p]=%x, ebss[%p]=%x\n\n\n", a+1, *(a+1), (b+1), *(b+1), (c+1), *(c+1));
} 

int main(void) {

    int *a = &_etext;
    int *b = &_edata;
    int *c = &_end;

    printf("Current: etext[%p]=%x, edata[%p]=%x, ebss[%p]=%x\n", a, *a, b, *b, c, *c);    
    printf("Preceding: etext[%p]=%x, edata[%p]=%x, ebss[%p]=%x\n", a-1, *(a-1), b-1, *(b-1), c-1, *(c-1));
    printf("After: etext[%p]=%x, edata[%p]=%x, ebss[%p]=%x\n\n\n", a+1, *(a+1), (b+1), *(b+1), (c+1), *(c+1));
}