#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <pthread.h>
#include <poll.h>

#define WR 0x01 // Write bit
#define RD 0xFE // Read bit
#define WR_L 0x0010 // Write bit
#define RD_L 0xFFEF // Read bit

#define LONGR 0x8000 // Long registers
#define SHORTR 0x7F // Short registers

#define WAKE_UP 1
#define WAKE_DOWN 0
#define RESET_ON 0
#define RESET_OFF 1

#define SPI_SPEED 5000000 // SPI Speed
#define SPI_NBITS 8 // SPI Number of Bits per Word
#define SPI_CS 0 // SPI Chip Select Active (0 - Low, 1 - High)

// Inicializa o descritor da SPI
void initSPI(void);

// Habilita a interrupção, executando o "handler" quando ocorrer
void getInterrupt(void (* handler)(void));

// Escreve o conteúdo de "buff", com um tamanho "arraySize" em bytes, na SPI
int writeSPI(unsigned char * buff, int arraySize);

// Transforma os valores de "values" em bytes, com uma divisão de "valueNBits", e
// escreve na SPI utilizando "writeSPI"
int writeSPIBuffer(unsigned int * values, int arraySize, int valueNBits);

// Escreve o endereço "addr", dividindo-o em bytes com divisão de "addrNBits", e 
// depois escreve o valor "value", dividindo-o em bytes com divisão de "valueNBits"
int writeSPIAddrSingle(unsigned int addr, int addrNBits, unsigned int value, int valueNBits);

// Escreve o endereço "addr", dividindo-o em bytes com divisão de "addrNBits", e 
// depois escreve o vetor "values", com tamanho de "arraySize" e dividindo-o em
// bytes com divisão de "valueNBits"
int writeSPIAddrBuffer(unsigned int addr, int addrNBits, unsigned int * values, int arraySize, int valueNBits);

// Escreve o vetor de bytes "buff", e lê a resposta pro endereço "ret"
int readSPI(unsigned char * buff, int buffSize, unsigned char * ret, int retSize);

// Escreve o endereço "addr", dividindo-o em bytes com divisão de "addrNBits", e 
// depois lê a resposta do tamanho de um byte, e a retorna
unsigned char readSPIAddrSingle(unsigned int addr, int addrNBits);

// Escreve o endereço "addr", dividindo-o em bytes com divisão de "addrNBits", e 
// depois lê a resposta pra "values", com tamanho de "arraySize"
int readSPIAddrBuffer(unsigned int addr, int addrNBits, unsigned char * values, int arraySize);

// Pausa a execução do programa atual em um tempo de "nano" nano segundos
int pauseNanoSec(long nano);
