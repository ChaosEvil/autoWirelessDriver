#include "autoDemo.h"

unsigned short modID = 0xb0c;
ModArgs * handlers;

void * autoDemoInit(void * modArgs) {
    unsigned char * rData;
    handlers = modArgs;
    
    printf("Ok, autolib1 is up!\n");
    while(1) {
        rData = (*handlers->readData)(modID);
        
        if (rData != NULL) {
            printf("Packet Received!\n");
            
            unsigned char * data = "Hello World!";
            (*handlers->sendData)(modID, 0x0001, 0x0101, data);
        }
        
        sleep(2);
    }
}

void * autoDemoResponder(char *args[100]) {
    
}

void * autoDemoGetId(unsigned short *idHere) {
    *idHere = modID;
}
