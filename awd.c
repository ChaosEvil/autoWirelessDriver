#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "nodes.h"
#include "transceivercom.h"
#include "communicator.h"
#include "packages.h"
#include "modloader.h"
#include "packetHandler.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#define EUI_7 0xCA
#define EUI_6 0xBA
#define EUI_5 0x61
#define EUI_4 0x89
#define EUI_3 0x50
#define EUI_2 0x16
#define EUI_1 0x77
#define EUI_0 0x84

// Porta do socket em que o Driver Principal atende a chamadas externas
#define SERV_PORT 30003

// Endereços do transmissor principal, do ping na rede e o de difusão, respectivamente.
unsigned char mac64Address[8] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, EUI_7};
unsigned short mac64AddressPing = 0x1234;
unsigned short mac64AddressBroadcast = 0xFFFF;
unsigned char macAddr = 0x00;

// Main Thread
int main(int argc, char **argv) {
    char * semName = "awddriversema";
    
    // O Driver Principal já está instanciado?
    errno = 0;
    /*if (sem_open(semName, O_CREAT | O_EXCL, "0777", 3) == SEM_FAILED && errno == EEXIST) {
        // Sim, então se comporta como um cliente para o processo principal do driver
        
        int client_sockfd;
        struct sockaddr_in serv_addr;
        char *serv_host = "localhost";
        struct hostent *host_ptr;
        
        if ((host_ptr = gethostbyname(serv_host)) == NULL) {
            perror("Erro: Processo nao encontrado.");
            exit(1);
        }
        
        if (host_ptr->h_addrtype != AF_INET) {
            perror("Erro: Enderecador invalido.");
            exit(1);
        }
        
        // Configuracoes do socket
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = ((struct in_addr *) host_ptr->h_addr_list[0])->s_addr;
        serv_addr.sin_port = htobe16(SERV_PORT);
        
        client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
        if (client_sockfd < 0) {
            perror("Erro: Nao foi possivel abrir o socket.");
            exit(1);
        }
        
        if (connect(client_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            perror("Erro: O driver nao aceitou a conexao.");
            exit(1);
        }
        
        char * request = "";
        int i;
        for (i = i; i < argc; i++) {
            strcat(request, argv[i]);
            strcat(request, "-;-");
        }
        
        // Escreve o comando para o driver principal
        write(client_sockfd, request, sizeof(request));
        
        // Le e imprime a resposta
        char response[MAX_PACK_SIZE];
        int len = read(client_sockfd, response, MAX_PACK_SIZE);
        response[len] = 0;
        
        close(client_sockfd);
        
        return response;
        
    } else {*/
        // O Driver Principal ainda não foi instanciado
        unsigned char buf, send[2] = {0x12, 0x71};
        int i, ret;
        
        //sem_unlink(semName);
        
        // Inicializa a SPI
        printf("Iniciando SPI...\n");
        initSPI();
        printf("SPI Iniciada.\n");
        
        // Inicializa o Radio Transmissor
        printf("Iniciando Transmissor...\n");
        
        ret = RADIO_ERROR;
        while(ret == RADIO_ERROR) {
            ret = transceiverInit(mac64Address);
            
            if (ret == RADIO_ERROR) {
                printf("Falha de Comunicacao\n");
                sleep(2);
            }
        }
        
        printf("Transmissor Iniciado.\n");
        
        // Inicializa a lista de transmissao para sensores/atuadores
        initNodes();
        
        // Inicializa a lista de pacotes recebidos
        initPackages();
        
        // Inicializa o importador de modulos
        initHandles();
        
        // Inicializa o comunicador
        int server_sockfd, *thread_sock, server_sock, clilen, opt = TRUE;
        struct sockaddr_in cli_addr, serv_addr;
        
        clilen = sizeof(cli_addr);
        
        server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
        
        if (server_sockfd < 0) {
            perror("Erro: Nao foi possivel abrir o socket.");
            exit(1);
        }
        
        // Configuracoes do socket
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htobe32(INADDR_ANY);
        serv_addr.sin_port = htobe16(SERV_PORT);
        
        if (bind(server_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            perror("Erro: Nao foi possivel abrir o socket na porta correta.");
            exit(1);
        }
        
        sleep(2);
        listen(server_sockfd, 5);
        
        while(1) {
            server_sock = accept(server_sockfd, (struct sockaddr *) &cli_addr, &clilen);
            
            if (server_sock < 0) {
                perror("Erro: A conexao do cliente falhou.");
            } else {
                thread_sock = malloc(4);
                thread_sock = server_sock;
                
                pthread_create(NULL, NULL, communication_handler, &thread_sock);
            }
        }
    //}
    
    return 0;
}
