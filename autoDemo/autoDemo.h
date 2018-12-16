#ifndef autoDemo_h
#define autoDemo_h

#include <stdio.h>

typedef struct {
    unsigned char * (*readData)(unsigned short module);
    void (*sendData)(unsigned short module, unsigned short macaddress, unsigned short nodeid, unsigned char * data);
} ModArgs;

extern void * autoDemoInit(void * modArgs);
extern void * autoDemoGetId(unsigned short *idHere);
extern void * autoDemoResponder(char *args[100]);

#endif