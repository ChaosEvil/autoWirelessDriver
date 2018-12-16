#ifndef resphandles_h
#define resphandles_h

struct Tmod_node {
    unsigned short moduleid;
    void (* initHandler)(void *);
    void * (* respHandler)(void *);
    struct Tmod_node * next;
};

// Lista de todos os modulos
typedef struct {
    int size;
    struct Tmod_node * first;
} Tmodules;

Tmodules allModules;

#endif