#ifndef __PRINT_H__
#define __PRINT_H__

#include <stdio.h>

#define print(str) { const char *c = str; while(*c!='\0'){*d++=*c++;} *d='\0'; } // c walks src, d walks dst
#define printint(ndig,val) { char str[ndig+1]; sprintf(str, "%d", val); print(str); }

#endif // __PRINT_H__
