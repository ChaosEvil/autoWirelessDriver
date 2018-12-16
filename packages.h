#ifndef packages_h
#define packages_h

#include <stdio.h>

// Pacotes de dados, contendo a id do noh, o macaddress e o dado em si
struct package {
    unsigned short nodeid;
    unsigned short address;
    
    unsigned char * data;
    
    struct package * next;
    struct package * prev;
};

// Lista de pacotes de um modulo especifico (moduleid)
struct packageList {
    unsigned short moduleid;
    
    int packCount;
    
    struct package * first;
    struct package * last;
    
    struct packageList * next;
};

// Lista onde estao todas as listas de todos os modulos (packageList)
struct allPacks {
    struct packageList * first;
};

struct allPacks allPackages;

extern const int maxPackages;

void initPackages();
void insertPackage(unsigned short module, unsigned short macaddress, unsigned short nodeid, unsigned char * data);
unsigned char * readData(unsigned short module);
void insertModule(unsigned short module);

#endif