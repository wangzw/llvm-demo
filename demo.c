//#include <stdio.h>

extern int puts(const char *);


const char * GetMsg(void) {
	return "This is default implementation of function \"GetMsg\"\n";
}

extern void PrintMsg(void) {
	puts(GetMsg());
}

