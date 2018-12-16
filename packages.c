#include <stdlib.h>
#include "transceivercom.h"
#include "packages.h"

const int maxPackages = 20;

void initPackages() {
    allPackages.first = NULL;
}

void insertPackage(unsigned short moduleid, unsigned short macaddress, unsigned short nodeid, unsigned char * data) {
    struct packageList * module = allPackages.first;
    struct packageList * packModule = NULL;
    struct package * pack = NULL;
    
    while (module) {
        if (module->moduleid == moduleid) {
            if (module->packCount == maxPackages)
                pack = module->last;
            
            packModule = module;
            break;
        }
        
        module = module->next;
    }
    
    if (packModule != NULL) {
        // Aloca o novo pacote
        struct package * newPackage = (struct package *) malloc(sizeof(struct package));
        newPackage->address  = macaddress;
        newPackage->nodeid = nodeid;
        newPackage->data = data;
        
        // Lista vazia
        if (module->first == NULL) {
            module->first = newPackage;
            module->last = newPackage;
            
            newPackage->next = NULL;
            newPackage->prev = NULL;
            
            module->packCount = 1;
            
        } else {
            newPackage->next = module->first;
            newPackage->prev = NULL;
            
            module->first->prev = newPackage;
            module->first = newPackage;
            
            // Lista cheia, remove o ultimo
            if (pack != NULL) {
                module->last->prev->next = NULL;
                module->last = module->last->prev;
                
                free(pack);
            } else {
                module->packCount++;
                
            }
            
        }
    }
}

unsigned char * readData(unsigned short moduleid) {
    struct packageList * module = allPackages.first;
    struct package * last;
    unsigned char * data = NULL;
    
    while (module) {
        if (module->moduleid == moduleid) {
            if (module->packCount > 0) {
                last = module->last;
                data = last->data;
                
                if (last->prev != NULL) {
                    last->prev->next = NULL;
                } else {
                    module->first = NULL;
                }
                
                module->last = last->prev;
                
                free(last);
                module->packCount--;
            }
            
            break;
        }
        
        module = module->next;
    }
    
    return data;
}

void insertModule(unsigned short module) {
    struct packageList * newModule = allPackages.first;
    
    int found = 0;
    while (newModule) {
        if (newModule->moduleid == module) {
            found = 1;
            break;
        }
        
        newModule = newModule->next;
    }
    
    if (found == 0) {
        newModule = (struct packageList *) malloc(sizeof(struct packageList));
        
        newModule->moduleid = module;
        newModule->first = NULL;
        newModule->last = NULL;
        newModule->packCount = 0;
        newModule->next = allPackages.first;
        
        allPackages.first = newModule;
    }
}
