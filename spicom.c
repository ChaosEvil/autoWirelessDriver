#include "spicom.h"
#include <errno.h>
#include <sys/ioctl.h>
#include <glib.h>

int ch; // SPI channel
FILE *inch; // Interrupt channel
void (* i_handler)(void);

void initSPI(void) {
    // Abertura do canal da SPI
    ch = open("/dev/spidev1.0", O_RDWR);
    if (ch < 0) printf("spi failed to open\n");
}

static int bufferize(unsigned char * buffer, int lastPos, unsigned int * values, int arraySize, int valueNBits) {
    int i, j, valueInc;
    
    i = lastPos;
    valueInc = 8;
    
    if (arraySize > 1) {
        for (j = 0; j < arraySize; j++) {
            while (valueInc <= valueNBits - 8) {
                buffer[i] = values[j] >> valueInc;
                valueInc += 8;
                i++;
            }
            
            buffer[i] = values[j] & 0xFF;
            i++;
        }
    } else {
        while (valueInc <= valueNBits - 8) {
            buffer[i] = *values >> valueInc;
            valueInc += 8;
            i++;
        }
        
        buffer[i] = *values & 0xFF;
        i++;
    }
    
    return i;
}

int writeSPI(unsigned char * buff, int arraySize) {
    struct spi_ioc_transfer xfer[1];
    unsigned char empty[1] = {0x00};
    int res;
    
    xfer[0].tx_buf = (unsigned long) buff;
    //xfer[0].rx_buf = (unsigned long) empty;
    xfer[0].len = arraySize;
    xfer[0].speed_hz = SPI_SPEED;
    xfer[0].cs_change = SPI_CS;
    xfer[0].delay_usecs = 0;
    xfer[0].bits_per_word = SPI_NBITS;
    
    res = ioctl(ch, SPI_IOC_MESSAGE(1), &xfer);
    
    return res;
}

int writeSPIBuffer(unsigned int * values, int arraySize, int valueNBits) {
    int size = (valueNBits/8)*arraySize;
    unsigned char buff[size];
    
    bufferize(buff, 0, values, arraySize, valueNBits);
    return writeSPI(buff, size);
}

int writeSPIAddrSingle(unsigned int addr, int addrNBits, unsigned int value, int valueNBits) {
    return writeSPIAddrBuffer(addr, addrNBits, &value, 1, valueNBits);
}

int writeSPIAddrBuffer(unsigned int addr, int addrNBits, unsigned int * values, int arraySize, int valueNBits) {
    int pos, size = (addrNBits/8 + arraySize*valueNBits/8);
    unsigned char buff[size];
    
    pos = bufferize(buff, 0, &addr, 1, addrNBits);
    bufferize(buff, pos, values, arraySize, valueNBits);
    
    return writeSPI(buff, size);
}

/* Método que escreve o conteúdo de "buff", com uma quantidade de bytes "buffSize",
 * e espera uma resposta de tamanho "retSize", que será colocada no buffer "ret".
 */
int readSPI(unsigned char * buff, int buffSize, unsigned char * ret, int retSize) {
    struct spi_ioc_transfer xfer[2];        // Vetor de configuração
    unsigned char empty[1] = {0x00};        // Vetor vazio
    int res;                                // Resposta de sucesso ou falha de envio
    
    // Posição zero é a de transferência (pode ser de recepção full-duplex)
    xfer[0].tx_buf = (unsigned long) buff;  // Buffer de dados de transferência
    xfer[0].rx_buf = (unsigned long) empty;
    xfer[0].len = buffSize;                 // Tamanho do buffer de transferência
    xfer[0].speed_hz = SPI_SPEED;           // Frequência da SPI (5 MHz)
    xfer[0].cs_change = SPI_CS;             // Estado de ativação do chip-select (0)
    xfer[0].delay_usecs = 0;                // Atraso entre pacotes
    xfer[0].bits_per_word = SPI_NBITS;      // Tamanho da palavra transmitida (8 bits)
    
    // Posição um, como não há nada para transferir, irá receber
    xfer[1].tx_buf = (unsigned long) empty;
    xfer[1].rx_buf = (unsigned long) ret;   // Buffer de dados de recepção
    xfer[1].len = retSize;                  // Tamanho do buffer de resposta em bytes
    xfer[1].speed_hz = SPI_SPEED;
    xfer[1].cs_change = SPI_CS;
    xfer[1].delay_usecs = 0;
    xfer[1].bits_per_word = SPI_NBITS;
    
    /* Chama o IOCTL para comunicação SPI (SPI_IOC_MESSAGE), com um vetor de
     * tamanho 2 (xref[2]), utilizando o descritor do dispositivo (ch). Retorna 0 se
     * for sucesso, e -1 se ocorrer um erro.
     */
    res = ioctl(ch, SPI_IOC_MESSAGE(2), &xfer);
    return res;
}

unsigned char readSPIAddrSingle(unsigned int addr, int addrNBits) {
    unsigned char value;
    
    readSPIAddrBuffer(addr, addrNBits, &value, 1);
    return value;
}

int readSPIAddrBuffer(unsigned int addr, int addrNBits, unsigned char * values, int arraySize) {
    int size = (addrNBits/8);
    unsigned char buff[size];
    
    bufferize(buff, 0, &addr, 1, addrNBits);
    return readSPI(buff, size, values, arraySize);
}

/* Método utilizado pela thread de interrupção para identificar quando uma IRQ
 * ocorre. Quando identificada, chama o handler responsável por tratá-la.
 */
void * _interrupt(void * pars) {
    unsigned char buffer[64];
    //int flags = fcntl(inch, F_GETFL, 0), ret;
    
    GMainLoop* loop = g_main_loop_new(0, 0);
    
    int fd = open( "/sys/class/gpio/gpio60/value", O_RDONLY | O_NONBLOCK );
    GIOChannel * channel = g_io_channel_unix_new(fd);
    GIOCondition cond = G_IO_PRI;
    guint id = g_io_add_watch( channel, cond, i_handler, 0 );
    
    g_main_loop_run(loop);
    /*
    while(1) {
        //fread(buffer, 1, sizeof(buffer), inch); // Lê o arquivo de eventos de interrupção
        ret = fcntl(inch, F_SETFL, flags | O_NONBLOCK);
        if (ret > -1) (*i_handler)(); // Executa o método passado ao "getInterrupt" pra tratar a IRQ
    }*/
}

/* Método utilizado para ativar o recebimento de interrupções SPI, utilizando o método
 * (handler) passado como parâmetro para tratar a interrupção.
 */
void getInterrupt(void (* handler)(void)) {
    pthread_t poll;
    long thaddr;
    int thresp;
    FILE *in, *indir, *inval;
    
    // Pino GPIO_60 (IRQ) como entrada em estado baixo
    in = fopen("/sys/class/gpio/export", "w");
    fseek(in, 0, SEEK_SET);
    fprintf(in, "%d", 60);
    fflush(in);
    fclose(in);
    
    indir = fopen("/sys/class/gpio/gpio60/direction", "w");
    fseek(indir, 0, SEEK_SET);
    fprintf(indir, "in");
    fflush(indir);
    fclose(indir);
    
    inval = fopen("/sys/class/gpio/gpio60/value", "w");
    fseek(inval, 0, SEEK_SET);
    fprintf(inval, "%d", 0);
    fflush(inval);
    fclose(inval);
    
    // borda de subida
    indir = fopen("/sys/class/gpio/gpio60/edge", "w");
    fseek(indir, 0, SEEK_SET);
    fprintf(indir, "rising");
    fflush(indir);
    fclose(indir);
    
    // O método passado é guardado em uma variável global, para poder ser chamado de
    // dentro da thread
    i_handler = handler;
    
    // Thread para ler continuamente o descritor, assim monitorando a IRQ
    thresp = pthread_create(&poll, NULL, &_interrupt, (void *) thaddr);
}

int pauseNanoSec(long nano) {
    struct timespec tmr1,tmr2;
    tmr1.tv_sec = 0;
    tmr1.tv_nsec = nano;
    
    if (nanosleep(&tmr1, &tmr2) < 0) {
        printf("Nano second pause failed\n");
        return -1;
    }
    return 0;
}
