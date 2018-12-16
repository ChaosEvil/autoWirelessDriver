#include "resphandles.h"
#include "packages.h"
#include "modloader.h"
#include "stdio.h"
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

ModArgs * args;

void initHandles(void) {
    int i;
    DIR *dir;
    unsigned char * str;
    unsigned char * err;
    struct dirent *ent;
    struct Tmod_node * modNode;
    
    allModules.first = NULL;
    allModules.size = 0;
    
    unsigned char * modDir = "/app/spi/dist/Debug/GNU-Linux/";
    
    // Carrega todos os modulos pelo nome de suas pastas
    //if ((dir = opendir(".")) != NULL) {
    if ((dir = opendir(modDir)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".so") != NULL) {
                modNode = (struct Tmod_node *) malloc(sizeof(struct Tmod_node));
                
                str = (unsigned char *) malloc(200 * sizeof(unsigned char));
                str[0] = '\0';
                strcat(str, modDir);
                strcat(str, ent->d_name);
                void *modClass = dlopen(str, RTLD_LAZY);
                
                void (*handler)(void *);
                void * (*respHandler)(void *);
                void (*idHandler)();
                
                err = dlerror();
                
                free(str);
                str = (unsigned char *) malloc(200 * sizeof(unsigned char));
                str[0] = '\0';
                strcat(str, ent->d_name);
                str[strlen(str) - 3] = '\0';
                
                strcat(str, "Responder");
                str[strlen(str)] = '\0';
                *(void **) (&respHandler) = dlsym(modClass, str);
                
                free(str);
                str = (unsigned char *) malloc(200 * sizeof(unsigned char));
                str[0] = '\0';
                strcat(str, ent->d_name);
                str[strlen(str) - 3] = '\0';
                
                strcat(str, "Init");
                str[strlen(str)] = '\0';
                *(void **) (&handler) = dlsym(modClass, str);
                
                free(str);
                str = (unsigned char *) malloc(200 * sizeof(unsigned char));
                str[0] = '\0';
                strcat(str, ent->d_name);
                str[strlen(str) - 3] = '\0';
                
                strcat(str, "GetId");
                *(void **) (&idHandler) = dlsym(modClass, str);
                
                err = dlerror();
                
                (*idHandler)(&modNode->moduleid);
                modNode->initHandler = handler;
                modNode->respHandler = respHandler;
                modNode->next = allModules.first;
                
                allModules.first = modNode;
                allModules.size++;
                free(str);
            }
        }
        
        closedir (dir);
        
        // Cria uma thread para cada module
        pthread_t tid[allModules.size];
        
        modNode = allModules.first;
        while (modNode != NULL) {
            args = (ModArgs *) malloc(sizeof(ModArgs));
            args->readData = readData;
            args->sendData = insertPackage;
            
            pthread_create(tid[i], NULL, modNode->initHandler, args);
            insertModule(modNode->moduleid);
            
            modNode = modNode->next;
        }
    }
}