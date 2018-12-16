#include <stdlib.h>
#include "transceivercom.h"

int currentTxPower = 1;
unsigned char SequenceNumber = 0;
unsigned char currentTxChannel;
unsigned char currentRxChannel;

void (* irqTXCall)(int);
void (* irqRXCall)(int);

unsigned char RXBuffer[RX_BUFF_SIZE];
int RXBufferSize;

unsigned short macPANId = 0x4742;
sem_t resp_sem;

pthread_mutex_t sendMutex;

typedef union _MRF24J40_IFR {
    unsigned char Val;
    
    struct _MRF24J40_IFR_bits {
        unsigned char RF_TXIF :1;
        unsigned char :2;
        unsigned char RF_RXIF :1;
        unsigned char :4;
    } bits;
} MRF24J40IF_t;

typedef union _BYTE {
    struct {
    	unsigned char	b0:     1;
    	unsigned char	b1:     1;
    	unsigned char	b2:     1;
    	unsigned char	b3:     1;
    	unsigned char	b4:     1;
    	unsigned char	b5:     1;
    	unsigned char	b6:     1;
    	unsigned char	b7:     1;
    } bits;
    
    unsigned char Val;
} BYTE;

void writeLong(unsigned short addr, unsigned char value) {
    int res = 0;
    
    addr = addr << 5;
    addr |= LONGR;
    addr |= WR_L;
    
    res = writeSPIAddrSingle(addr, 16, value, 8);
}

void writeShort(unsigned char addr, unsigned char value) {
    int res = 0;
    
    /*addr = addr << 1;
    addr &= SHORTR;
    addr |= WR;*/
    
    res = writeSPIAddrSingle(addr, 8, value, 8);
}

unsigned char readLong(unsigned short addr) {
    addr = addr << 5;
    addr |= LONGR;
    addr &= RD_L;
    
    return readSPIAddrSingle(addr, 16);
}

unsigned char readShort(unsigned char addr) {
    /*addr = addr << 1;
    addr &= SHORTR;
    addr &= RD;*/
    
    return readSPIAddrSingle(addr, 8);
}

int transceiverInit(unsigned char * mac64Address) {  
    unsigned char i;
    int j;
    FILE *in, *indir, *inval;
    
    // Executa comandos no linux para fazer o deploy dos MUX dos pinos
    system("cd /lib/firmware/ \n echo BBB_SPI0 > /sys/devices/bone_capemgr.9/slots \n");
    system("cd /lib/firmware/ \n echo BBB-GPIO-KEYS > /sys/devices/bone_capemgr.9/slots \n");
    
    // Pinos GPIO_30 e GPIO_31 ativados
    in = fopen("/sys/class/gpio/export", "w");
    fseek(in, 0, SEEK_SET);
    fprintf(in, "%d", 30);
    fflush(in);
    fprintf(in, "%d", 31);
    fflush(in);
    fclose(in);
    
    // Pino GPIO_30 (Wake) como saida em estado baixo
    indir = fopen("/sys/class/gpio/gpio30/direction", "w");
    fseek(indir, 0, SEEK_SET);
    fprintf(indir, "out");
    fflush(indir);
    fclose(indir);
    
    inval = fopen("/sys/class/gpio/gpio30/value", "w");
    fseek(inval, 0, SEEK_SET);
    fprintf(inval, "%d", 0);
    fflush(inval);
    fclose(inval);
    
    // Pino GPIO_31 (Reset) como saida em estado alto
    indir = fopen("/sys/class/gpio/gpio31/direction", "w");
    fseek(indir, 0, SEEK_SET);
    fprintf(indir, "out");
    fflush(indir);
    fclose(indir);
    
    inval = fopen("/sys/class/gpio/gpio31/value", "w");
    fseek(inval, 0, SEEK_SET);
    fprintf(inval, "%d", 1);
    fflush(inval);
    fclose(inval);
    
    transceiverReset(RESET_ON);
    pauseNanoSec(5000000);
    
    transceiverReset(RESET_OFF);
    pauseNanoSec(5000000);
    
    /* do a soft reset */
    writeShort(WRITE_SOFTRST, 0x07); 
    pauseNanoSec(1000000);
    
    writeShort(WRITE_RFCTL, 0x04);
    writeShort(WRITE_RFCTL, 0x00);
    pauseNanoSec(1000000);
    
    
    // Enable FIFO and TX Enable
    writeShort(WRITE_FFOEN, 0x98);
    writeShort(WRITE_TXPEMISP, 0x95);    
    
    /* Interrupt rising edge*/
    writeLong(RFRXCTRL, 0x02);
    
    /* program the RF and Baseband Register */
    //writeLong(RFCTRL4, 0x02);
    
    /* Enable the RX */
    writeLong(RFRXCTRL, 0x01);
    
    /* setup */  
    writeLong(RFCTRL0, 0x03);
    writeLong(RFCTRL1, RFCTRL1_VAL);
    
    /* enable the RF-PLL */
    writeLong(RFCTRL2, 0x80);
    
    /* set TX output power */
    writeLong(RFCTRL3, TX_POWER_LEVEL);
    
    /* program RSSI ADC with 2.5 MHz clock */
    writeLong(RFCTRL6, 0x90);
    writeLong(RFCTRL7, 0x80);
    writeLong(RFCTRL8, 0x10);     
    
    // Disable clockout
    writeLong(SCLKDIV, 0x21);


    if (DEVICE_TYPE != PAN_COORDINATOR) {
    // Join network as router
    // and Automatic Acknowledgment enabled
        writeShort(WRITE_RXMCR, 0x04);
    } else {
    // Join network as PAN Coordinator
    // and Automatic Acknowledgment enabled
        writeShort(WRITE_RXMCR, 0x0C);
    }
    
    /* flush the RX fifo */
    writeShort(WRITE_RXFLUSH, 0x01);    
    
    // Configure Unslotted CSMA-CA Mode
    i = readShort(READ_TXMCR);
    i = i & 0xDF;
    writeShort(WRITE_TXMCR, i);
    
    // Define Nonbeacon-Enabled Network
    writeShort(WRITE_ORDER, 0xFF); 


    /* Program CCA mode using RSSI */
    // 0x80 � o valor default, 0xB8 � o recomendado
    writeShort(WRITE_BBREG2, 0xB8);
    
    /* Program CCA, RSSI threshold values */
    // Valor Default � 0x00, recomendado � 0x60
    // Determina o n�ve de sinal que indica o canal como ocupado
    // 0x60 = -69 dBm
    writeShort(WRITE_RSSITHCCA, 0x60);
    
    /* Enable the packet RSSI */
    writeShort(WRITE_BBREG6, 0x40);
    
    /* Program the short MAC Address, 0xffff */
    if (DEVICE_TYPE != PAN_COORDINATOR) {
        writeShort(WRITE_SADRL, 0xFF);
        writeShort(WRITE_SADRH, 0xFF);
    } else {
        writeShort(WRITE_SADRL, 0x00);
        writeShort(WRITE_SADRH, 0x00);    
    }
    
    /* and the short PanId Identifier, 0xffff */
    if (DEVICE_TYPE != PAN_COORDINATOR) {
        writeShort(WRITE_PANIDL, 0xFF);
        writeShort(WRITE_PANIDH, 0xFF);
    } else {
        writeShort(WRITE_PANIDL, (PANID_INIT_VALUE & 0xFF));
        writeShort(WRITE_PANIDH, ((PANID_INIT_VALUE >> 8) & 0xFF));      
    }
    
    /* Test radio */
    writeShort(WRITE_EADR0, 0xCA);
    i = readShort(READ_EADR0);
    if (i != 0xCA) return RADIO_ERROR;
    
    writeShort(WRITE_EADR1, 0xF1);
    i = readShort(READ_EADR1);
    if (i != 0xF1) return RADIO_ERROR;
    
    writeShort(WRITE_EADR2, 0x05);
    i = readShort(READ_EADR2);
    if (i != 0x05) return RADIO_ERROR;
    
    writeShort(WRITE_EADR3, 0xBA);
    i = readShort(READ_EADR3);
    if (i != 0xBA) return RADIO_ERROR;
    
    writeShort(WRITE_EADR4, 0xFF);
    i = readShort(READ_EADR4);
    if (i != 0xFF) return RADIO_ERROR;
    
    writeShort(WRITE_EADR5, 0x10);
    i = readShort(READ_EADR5);
    if (i != 0x10) return RADIO_ERROR;
    
    writeShort(WRITE_EADR6, 0x4E);
    i = readShort(READ_EADR6);
    if (i != 0x4E) return RADIO_ERROR;
    
    writeShort(WRITE_EADR7, 0x11);
    i = readShort(READ_EADR7);
    if (i != 0x11) return RADIO_ERROR;
    
    
    
    /* Program Long MAC Address*/
    for (i = 0; i < 8; i++) {
        writeShort(WRITE_EADR0 + i*2, mac64Address[7 - i]);
    }
    
    setChannel(CHANNEL_INIT_VALUE);
    pauseNanoSec(1000000);
    
    //i = readShort(READ_ISRSTS);
    writeShort(WRITE_FFOEN, 0x98);
    
    // Habilita interrup��es
    writeShort(WRITE_INTMSK, 0xF6);
    
    // bypass the errata to make RF communication stable
    writeLong(RFCTRL1, RFCTRL1_VAL);
    
    getInterrupt((void *) interrupt);
    
    //sem_init(&resp_sem, 0, 1);
    
    return RADIO_OK;
}

int transceiverReset(int state) {
    FILE *inval = fopen("/sys/class/gpio/gpio31/value", "w");
    
    fseek(inval, 0, SEEK_SET);
    fprintf(inval, "%d", state);
    fflush(inval);
    
    fclose(inval);
}

/*********************************************************************
 * Function:        void MRF24J40Reset(void)
 *
 * PreCondition:    BoardInit (or other initialzation code is required)
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    MRF24J40 is initialized
 *
 * Overview:        This function initializes the MRF24J40 and is required
 *                  before stack operation is available
 ********************************************************************/
void MRF24J40Reset(unsigned char * mac64Address, unsigned char macAddr, unsigned char macPANId) {  
    unsigned char i;
    int j;
    
    transceiverReset(RESET_ON);
    for(j=0;j<7000;j++);

    transceiverReset(RESET_OFF);
    for(j=0;j<7000;j++);  
    
  
    /* do a soft reset */
    writeShort(WRITE_SOFTRST, 0x07); 
    for(j=0;j<1000;j++);
    
    writeShort(WRITE_RFCTL, 0x04);
    writeShort(WRITE_RFCTL, 0x00);
    for(j=0;j<1000;j++);    

    
    // Enable FIFO and TX Enable
    //writeShort(WRITE_FFOEN, 0x98);
    writeShort(WRITE_FFOEN, 0x98);
    writeShort(WRITE_TXPEMISP, 0x95);    
    
    /* setup */  
    writeLong(RFCTRL0, 0x03);
          
    writeLong(RFCTRL1, RFCTRL1_VAL);
    
    /* enable the RF-PLL */
    writeLong(RFCTRL2, 0x80);
    
    /* set TX output power */
    writeLong(RFCTRL3, currentTxPower);
    
    /* program RSSI ADC with 2.5 MHz clock */
    //writeLong(RFCTRL6, 0x04);
    writeLong(RFCTRL6, 0x90);

    writeLong(RFCTRL7, 0x80);
    
    writeLong(RFCTRL8, 0x10);
    
    
    // Disable clockout
    writeLong(SCLKDIV, 0x21);
    
    if (DEVICE_TYPE != PAN_COORDINATOR) {
    // Join network as router
    // and Automatic Acknowledgment enabled
        writeShort(WRITE_RXMCR, 0x04);
    } else {
    // Join network as PAN Coordinator
    // and Automatic Acknowledgment enabled
        writeShort(WRITE_RXMCR, 0x0C);
    }
    
    /* flush the RX fifo */
    writeShort(WRITE_RXFLUSH, 0x01);    
    
    // Configure Unslotted CSMA-CA Mode
    i = readShort(READ_TXMCR);
    i = i & 0xDF;
    writeShort(WRITE_TXMCR, i);
    
    // Define Nonbeacon-Enabled Network
    writeShort(WRITE_ORDER, 0xFF);



    /* Program CCA mode using RSSI */
    // 0x80 � o valor default, 0xB8 � o recomendado
    //writeShort(WRITE_BBREG2,0xB8);
    writeShort(WRITE_BBREG2, 0xB8);
    //writeShort(WRITE_BBREG2, 0x80);
    
    /* Program CCA, RSSI threshold values */
    // Valor Default � 0x00, recomendado � 0x60
    // Determina o n�ve de sinal que indica o canal como ocupado
    // 0x60 = -69 dBm
    writeShort(WRITE_RSSITHCCA, 0x60);
    //writeShort(WRITE_RSSITHCCA, 0x00);            
    
    
    /* Enable the packet RSSI */
    writeShort(WRITE_BBREG6, 0x40);    
    
    /* Program the short MAC Address, 0xffff */
    if (DEVICE_TYPE != PAN_COORDINATOR) {
        writeShort(WRITE_SADRL, (unsigned char) (macAddr & 0xFF));
        writeShort(WRITE_SADRH, (unsigned char) (macAddr >> 8));
    } else {
        writeShort(WRITE_SADRL, 0x00);
        writeShort(WRITE_SADRH, 0x00);    
    }
    
    /* and the short PanId Identifier, 0xffff */
    writeShort(WRITE_PANIDL, (unsigned char) (macPANId & 0xFF));
    writeShort(WRITE_PANIDH, (unsigned char) (macPANId >> 8));
    
    // Program Long MAC Address
    for (i = 0; i < 8; i++) {
        writeShort(WRITE_EADR0 + i*2, mac64Address[7 - i]);
    }
    
    if (MULTICHANNEL_SUPPORT == 1) {
        setChannel(currentRxChannel);
    } else {
        setChannel(CHANNEL_INIT_VALUE);
    }
      
    for(j=0;j<1000;j++);
    
    writeShort(WRITE_FFOEN, 0x98);
    
    // Habilita interrup��es
    writeShort(WRITE_INTMSK, 0xF6);
    
    // bypass the errata to make RF communication stable
    writeLong(RFCTRL1, RFCTRL1_VAL);
}



/*********************************************************************
 * Function:        void setChannel(BYTE channel)
 *
 * PreCondition:    MRF24J40 is initialized
 *
 * Input:           BYTE channel - this is the channel that you wish
 *                  to operate on.  This should be CHANNEL_11, CHANNEL_12,
 *                  ..., CHANNEL_26. 
 *
 * Output:          None
 *
 * Side Effects:    the MRF24J40 now operates on that channel
 *
 * Overview:        This function sets the current operating channel
 *                  of the MRF24J40
 ********************************************************************/
void setChannel(unsigned char channel) {
    int z;
    
    writeLong(RFCTRL0, (channel | 0x02));
    writeShort(WRITE_RFCTL, 0x04);
    pauseNanoSec(1000000);
    
    writeShort(WRITE_RFCTL, 0x00);
    pauseNanoSec(1000000);
    
    if (MULTICHANNEL_SUPPORT == 1) {
        currentTxChannel = channel;
        //currentRxChannel = channel;
    }
}

unsigned char getChannel(void){ 
    return currentRxChannel;
}

void setRxChannel(unsigned char channel) {
    currentRxChannel = channel;
}

void setTxPower(unsigned char power_level) {
    currentTxPower = power_level;
    writeLong(RFCTRL3, currentTxPower);
}

unsigned char getTxPower(void) {
    return  currentTxPower;   
}

/*********************************************************************
 * Function:        void transceiverSetDeviceAddress(WORD PANID, WORD shortAddress)
 *
 * PreCondition:    Communication port to the MRF24J40 initialized
 *
 * Input:           WORD PANID is the PANID you want to operate on
 *                  WORD shortAddress is the short address you want to use on that network
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function sets the short address for the IEEE address
 *                      filtering
 ********************************************************************/
void transceiverSetDeviceAddress(int PANID, int shortAddress) {
    writeShort(WRITE_SADRL, (unsigned char) (PANID & 0x00FF));
    writeShort(WRITE_SADRH, (unsigned char) (PANID >> 8));
    writeShort(WRITE_PANIDL, (unsigned char) (shortAddress & 0x00FF));
    writeShort(WRITE_PANIDH, (unsigned char) (shortAddress >> 8));
}


void transceiverAutoACK(unsigned char autoack) {
    unsigned char reg;
    
    if (AUTO_ACK_CONTROL == 1) {
        reg = readShort(READ_RXMCR);
        
        if (autoack) {
            writeShort(WRITE_RXMCR, reg | 0x04);
        } else {
            writeShort(WRITE_RXMCR, reg & ~0x04);
        }
    } else {
        (void) autoack;  
    }
}

void transceiverPing(unsigned short macAddress) {
    unsigned char i = 0, j = 0, address = 0, HeaderSize = 0, PayloadSize = 0, basis = 0;
    
    // Inicia montagem do pacote Data
    
    // Indicação de Beacon no Frame Control
    writeLong(2, 0x41);
    HeaderSize++;
    
    // Indicação de Dest e Source Address de 16b, no Frame Control    
    writeLong(3, 0x88);
    HeaderSize++;  
    
    // Sequence Number
    writeLong(4, SequenceNumber++);
    HeaderSize++;
    
    // PanId do coordenador que gera o data packet
    writeLong(5, (unsigned char)(macPANId & 0xFF));
    writeLong(6, (unsigned char)(macPANId >> 8));
    
    // Endereço de destino broadcast do data packet
    writeLong(7, (unsigned char)0xFF);
    writeLong(8, (unsigned char)0xFF);
    
    // Endereço fonte do data packet
    writeLong(9, (unsigned char)(macAddress & 0xFF));
    writeLong(10, (unsigned char)(macAddress >> 8));
    HeaderSize += 6;  
    
    
    // Enviar dados do pacote de vizinhança padrão da rede
    
    // Tipo de pacote de dados
    writeLong(11, DATA_PING);
    PayloadSize++;
    
    // Base stations info  
    basis = 1;
    
    writeLong(12, (unsigned char)((basis & 0x0F) | 0xF0));
    PayloadSize++;
    
    // Informação do tamanho do MAC header em bytes
    // No modo não seguro é ignorado
    writeLong(0x000, HeaderSize);
    // Informação do tamanho em bytes do MAC header + Payload
    writeLong(0x001, (unsigned char) (HeaderSize + PayloadSize));
    
    //transmit packet without ACK requested
    //mac_tasks_pending.bits.PacketPendingAck = 1;
    writeShort(WRITE_TXNMTRIG, 0b00000001);
}

void readComplete () {
    printf("RX Response Received!!\n");
    fflush(stdin);
}

void sentComplete (int success) {
    printf("TX Response Received!!\n");
    fflush(stdin);
    printf("%s \n",RXBuffer);
    fflush(stdin);
}

void transceiverSend(unsigned short macAddress, unsigned char payload_size, unsigned char *payload, unsigned char node_id) {
    pthread_mutex_lock(&sendMutex);
    
    unsigned char FrameIndex = 0, HeaderSize = 0, PayloadSize = 0, tmp = 0;
    int i = 0, s, sval;
    struct timespec ts;
    
    irqTXCall = (void *) sentComplete;
    
    ts.tv_sec = 0;
    ts.tv_nsec = 500000;
    
    // Inicia montagem do pacote NWK Command
    // Inicia montagem do pacote Data p/ roteamento
    
    // Indicação de Beacon no Frame Control
    writeLong(2, 0x41);
    HeaderSize++;
    
    // Indicação de Dest e Source Address de 16b, no Frame Control
    writeLong(3, 0x88);
    HeaderSize++;
    
    // Sequence Number
    writeLong(4, SequenceNumber++);
    HeaderSize++;
    
    // PanId do coordenador que gera o data packet
    writeLong(5, (unsigned char)(macPANId & 0xFF));
    writeLong(6, (unsigned char)(macPANId >> 8));
    
    // Endereço de destino do data packet
    writeLong(7, (unsigned char)(macAddress & 0xFF));
    writeLong(8, (unsigned char)(macAddress >> 8));
    
    // Não precisa PANId da fonte pq
    // o pacote é intrapan
    
    // Endereço fonte do data packet (coordenador com endereco 0)
    writeLong(9, 0);
    writeLong(10, 0);
    
    HeaderSize += 6;
    FrameIndex = 11;
    
    // Tipo de pacote de dados
    writeLong(FrameIndex++, DATA_PACKET);
    PayloadSize++;  
    
    // Tipo de pacote de dados
    writeLong(FrameIndex++, node_id);
    PayloadSize++;
    
    for (i = 0; i < payload_size; i++) {
        writeLong(FrameIndex++, *payload++);
        PayloadSize++;
    }
    
    // Informação do tamanho do MAC header em bytes
    // No modo não seguro é ignorado
    writeLong(0x000, HeaderSize);
    
    // Informação do tamanho em bytes do MAC header + Payload
    writeLong(0x001, (unsigned char) (HeaderSize + PayloadSize));
    
    // transmit packet with ACK requested
    // Para solicitar ACK, bit2 = 1
    writeShort(WRITE_TXNMTRIG, 0b00000101);
    
    printf("Sent! Waiting Response...\n");
    pthread_mutex_unlock(&sendMutex);
}

static gboolean interrupt(GIOChannel *channel, GIOCondition condition, gpointer user_data) {
    int i, j, s, k;
    unsigned char ReceivedByte;
    unsigned short nodeid;
    unsigned short macaddr;
    unsigned short moduleid;
    
    gchar * buf[1];
    MRF24J40IF_t flags;
    struct timespec ts;
    
    // Lê o GPIO da interrupcao
    GError *error = 0;
    gsize bytes_read = 0;
    g_io_channel_seek_position(channel, 0, G_SEEK_SET, 0);
    GIOStatus rc = g_io_channel_read_chars(channel,
                                           buf, sizeof(buf) - 1,
                                           &bytes_read,
                                           &error);
    
    // Se for borda de descida.. ignora
    if (atoi(buf) == 0) return 1;
    
    ts.tv_sec = 0;
    ts.tv_nsec = 500000;
    
    flags.Val = 0;
    printf("Interrupted!\n");
    // read the interrupt status register to see what caused the interrupt
    flags.Val = readShort(READ_ISRSTS);
    
    if (flags.bits.RF_TXIF) {
        //if the TX interrupt was triggered
        BYTE results;
        
        //read out the results of the transmission
        results.Val = readShort(READ_TXSR);
        
        if (results.bits.b0 == 1) {
            if (irqTXCall) irqTXCall(0);
        } else {
            if (irqTXCall) irqTXCall(1);
        }
        
        // Reseta a funcao de tratamento da interrupcao de escrita
        irqTXCall = NULL;
    }
    
    /* TESSST 
    if (flags.bits.RF_RXIF) {
       END TEST */
        
        // Se for a primeira interrupcao do RX, entao desativa a recepcao de pacotes e le o buffer
        if (irqRXCall == NULL) {
            // packet received
            // Need to read it out
            // Disable receiving packets off air
            writeShort(WRITE_BBREG1, 0x04);
            
            irqRXCall = readComplete;
            
            /* TESSST 
            //first byte of the receive buffer is the packet length
            RXBufferSize = readLong(0x300);
            
            // Salva o novo pacote
            for (j = 0; j <= RXBufferSize; j++) {
                RXBuffer[j] = readLong((unsigned short)0x300 + j);
            }
            */
            RXBufferSize = 20;
            for (i = 0; i < 20; i++) {
                RXBuffer[i] = i;
            }
            
            /* END TEST */
            
        // Se for a segunda interrupcao, entao a leitura acabou, entao ativa a recepcao de pacotes novamente
        } else {
            irqRXCall = NULL;
            
            // Reinicia o ponteiro do buffer RX
            writeShort(WRITE_RXFLUSH, 0x01);
            // bypass MRF24J40 errata 5
            writeShort(WRITE_FFOEN, 0x98);
            // Enable receiving packets off air
            writeShort(WRITE_BBREG1, 0x00);
            
            nodeid   = ((RXBuffer[5] << 8)  & 0xFF00) | (RXBuffer[6]  & 0xFF);
            macaddr  = ((RXBuffer[9] << 8)  & 0xFF00) | (RXBuffer[10] & 0xFF);
            moduleid = ((RXBuffer[11] << 8) & 0xFF00) | (RXBuffer[12] & 0xFF);
            
            unsigned char buffCopy[RX_BUFF_SIZE];
            for (i = 0; i < RXBufferSize; i++) {
                buffCopy[i] = RXBuffer[i];
            }
            
            insertPackage(moduleid, macaddr, nodeid, buffCopy);
        }
    /* TESSST 
    }
       END TEST */
    
    return 1;
}