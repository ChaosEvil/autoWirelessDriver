#include "communicator.h"
#include "packages.h"
#include "resphandles.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <string.h>

void * communication_handler(void * pars) {
    unsigned short modId;
    char request[MAX_PACK_SIZE], *token, *mArgs[100], *response = "";
    int socket = (int) pars;
    int mLen = 0, reserve = 0;
    
    // Le a mensagem do cliente, e se certifica de que eh um texto.
    int len = read(socket, request, MAX_PACK_SIZE);
    request[len] = 0;
    
    // Procura pelo ID do modulo (-m) e constroi um vetor de parametros para o modulo.
    while ((token = strsep(&request, "-;-"))) {
        if (strcmp(token, "-m") == 0 || reserve == 1) {
            if (reserve == 0) {
                reserve = 1;
            } else {
                modId = token;
                reserve = 0;
            }
        } else {
            mArgs[mLen] = token;
            mLen++;
        }
    }
    
    // Se encontrou o ID, entao procura o modulo na lista e chama a funcao de resposta do mesmo.
    if (modId != NULL) {
        struct Tmod_node * module = allModules.first;
        
        while (module) {
            if (module->moduleid == modId) {
                response = module->respHandler(mArgs);
                break;
            }
            
            module = module->next;
        }
    }
    
    write(socket, response, sizeof(response));
    close(socket);
}