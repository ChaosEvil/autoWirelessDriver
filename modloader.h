#ifndef modloader_h
#define modloader_h

typedef struct {
    unsigned char * (*readData)(unsigned short module);
    void (*sendData)(unsigned short module, unsigned short macaddress, unsigned short nodeid, unsigned char * data);
} ModArgs;

void initHandles(void);

#endif