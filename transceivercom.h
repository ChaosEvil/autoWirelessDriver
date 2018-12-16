#ifndef transceivercom_h
#define transceivercom_h

#include "spicom.h"
#include "semaphore.h"
#include "errno.h"
#include <glib.h>

// define for packet types
#define DATA_PING 0x01
#define DATA_PACKET 0x03

#define CHANNEL_11 0x00
#define CHANNEL_12 0x10
#define CHANNEL_13 0x20
#define CHANNEL_14 0x30
#define CHANNEL_15 0x40
#define CHANNEL_16 0x50
#define CHANNEL_17 0x60
#define CHANNEL_18 0x70
#define CHANNEL_19 0x80
#define CHANNEL_20 0x90
#define CHANNEL_21 0xA0
#define CHANNEL_22 0xB0
#define CHANNEL_23 0xC0
#define CHANNEL_24 0xD0
#define CHANNEL_25 0xE0
#define CHANNEL_26 0xF0

#define CHANNEL_INIT_VALUE CHANNEL_26

// error codes

#define RADIO_OK 0
#define RADIO_ERROR 1

//long address registers

#define RFCTRL0 0x200
#define RFCTRL1 0x201
#define RFCTRL2 0x202
#define RFCTRL3 0x203
#define RFCTRL4 0x204
#define RSSI 0x205
#define RFCTRL6 0x206
#define RFCTRL7 0x207
#define RFCTRL8 0x208
#define CAL1 0x209
#define CAL2 0x20A
#define CAL3 0x20B
#define SFCNTRH 0x20C
#define SFCNTRM 0x20D
#define SFCNTRL 0x20E
#define RFSTATE 0x20F
#define BATTERYTH 0x210
#define CLKIRQCR 0x211
#define SRCADRMODE 0x212
#define SRCADDR0 0x213
#define SRCADDR1 0x214
#define SRCADDR2 0x215
#define SRCADDR3 0x216
#define SRCADDR4 0x217
#define SRCADDR5 0x218
#define SRCADDR6 0x219
#define SRCADDR7 0x21A
#define RXFRAMESTATE 0x21B
#define SECSTATUS 0x21C
#define STCCMP 0x21D
#define HLEN 0x21E
#define FLEN 0x21F
#define SCLKDIV 0x220
//#define reserved 0x221
#define WAKETIMEL 0x222
#define WAKETIMEH 0x223
#define TXREMCNTL 0x224
#define TXREMCNTH 0x225
#define TXMAINCNTL 0x226
#define TXMAINCNTM 0x227
#define TXMAINCNTH0 0x228
#define TXMAINCNTH1 0x229
#define RFMANUALCTRLEN 0x22A
#define RFMANUALCTRL 0x22B
#define RFRXCTRL RFMANUALCTRL
#define TxDACMANUALCTRL 0x22C
#define RFMANUALCTRL2 0x22D
#define TESTRSSI 0x22E
#define TESTMODE 0x22F

#define MAC0 0x230
#define MAC1 0x231
#define MAC2 0x232
#define MAC3 0x233
#define MAC4 0x234
#define MAC5 0x235
#define MAC6 0x236
#define MAC7 0x237

#define NORMAL_TX_FIFO 0x000
#define BEACON_TX_FIFO 0x080
#define GTS1_TX_FIFO 0x100
#define GTS2_TX_FIFO 0x180
#define RX_FIFO 0x300
#define SECURITY_FIFO 0x280

//short address registers for reading

#define READ_RXMCR 0x00
#define READ_PANIDL 0x02
#define READ_PANIDH 0x04
#define READ_SADRL 0x06
#define READ_SADRH 0x08
#define READ_EADR0 0x0A
#define READ_EADR1 0x0C
#define READ_EADR2 0x0E
#define READ_EADR3 0x10
#define READ_EADR4 0x12
#define READ_EADR5 0x14
#define READ_EADR6 0x16
#define READ_EADR7 0x18
#define READ_RXFLUSH 0x1A
#define READ_TXSTATE0 0x1C
#define READ_TXSTATE1 0x1E
#define READ_ORDER 0x20
#define READ_TXMCR 0x22
#define READ_ACKTMOUT 0x24
#define READ_SLALLOC 0x26
#define READ_SYMTICKL 0x28
#define READ_SYMTICKH 0x2A
#define READ_PAONTIME 0x2C
#define READ_PAONSETUP 0x2E
#define READ_FFOEN 0x30
#define READ_CSMACR 0x32
#define READ_TXBCNTRIG 0x34
#define READ_TXNMTRIG 0x36
#define READ_TXG1TRIG 0x38
#define READ_TXG2TRIG 0x3A
#define READ_ESLOTG23 0x3C
#define READ_ESLOTG45 0x3E
#define READ_ESLOTG67 0x40
#define READ_TXPEND 0x42
#define READ_TXBCNINTL 0x44
#define READ_FRMOFFSET 0x46
#define READ_TXSR 0x48
#define READ_TXLERR 0x4A
#define READ_GATE_CLK 0x4C
#define READ_TXOFFSET 0x4E
#define READ_HSYMTMR0 0x50
#define READ_HSYMTMR1 0x52
#define READ_SOFTRST 0x54
#define READ_BISTCR 0x56
#define READ_SECCR0 0x58
#define READ_SECCR1 0x5A
#define READ_TXPEMISP 0x5C
#define READ_SECISR 0x5E
#define READ_RXSR 0x60
#define READ_ISRSTS 0x62
#define READ_INTMSK 0x64
#define READ_GPIO 0x66
#define READ_GPIODIR 0x68
#define READ_SLPACK 0x6A
#define READ_RFCTL 0x6C
#define READ_SECCR2 0x6E
#define READ_BBREG1 0x72
#define READ_BBREG2 0x74
#define READ_BBREG3 0x76
#define READ_BBREG4 0x78
#define READ_BBREG5 0x7A
#define READ_BBREG6 0x7C
#define READ_RSSITHCCA 0x7E

//short address registers for writing

#define WRITE_RXMCR 0x01
#define WRITE_PANIDL 0x03
#define WRITE_PANIDH 0x05
#define WRITE_SADRL 0x07
#define WRITE_SADRH 0x09
#define WRITE_EADR0 0x0B
#define WRITE_EADR1 0x0D
#define WRITE_EADR2 0x0F
#define WRITE_EADR3 0x11
#define WRITE_EADR4 0x13
#define WRITE_EADR5 0x15
#define WRITE_EADR6 0x17
#define WRITE_EADR7 0x19
#define WRITE_RXFLUSH 0x1B
#define WRITE_TXSTATE0 0x1D
#define WRITE_TXSTATE1 0x1F
#define WRITE_ORDER 0x21
#define WRITE_TXMCR 0x23
#define WRITE_ACKTMOUT 0x25
#define WRITE_SLALLOC 0x27
#define WRITE_SYMTICKL 0x29
#define WRITE_SYMTICKH 0x2B
#define WRITE_PAONTIME 0x2D
#define WRITE_PAONSETUP 0x2F
#define WRITE_FFOEN 0x31
#define WRITE_CSMACR 0x33
#define WRITE_TXBCNTRIG 0x35
#define WRITE_TXNMTRIG 0x37
#define WRITE_TXG1TRIG 0x39
#define WRITE_TXG2TRIG 0x3B
#define WRITE_ESLOTG23 0x3D
#define WRITE_ESLOTG45 0x3F
#define WRITE_ESLOTG67 0x41
#define WRITE_TXPEND 0x43
#define WRITE_TXBCNINTL 0x45
#define WRITE_FRMOFFSET 0x47
#define WRITE_TXSR 0x49
#define WRITE_TXLERR 0x4B
#define WRITE_GATE_CLK 0x4D
#define WRITE_TXOFFSET 0x4F
#define WRITE_HSYMTMR0 0x51
#define WRITE_HSYMTMR1 0x53
#define WRITE_SOFTRST 0x55
#define WRITE_BISTCR 0x57
#define WRITE_SECCR0 0x59
#define WRITE_SECCR1 0x5B
#define WRITE_TXPEMISP 0x5D
#define WRITE_SECISR 0x5F
#define WRITE_RXSR 0x61
#define WRITE_ISRSTS 0x63
#define WRITE_INTMSK 0x65
#define WRITE_GPIO 0x67
#define WRITE_GPIODIR 0x69
#define WRITE_SLPACK 0x6B
#define WRITE_RFCTL 0x6D
#define WRITE_SECCR2 0x6F
#define WRITE_BBREG0 0x71
#define WRITE_BBREG1 0x73
#define WRITE_BBREG2 0x75
#define WRITE_BBREG3 0x77
#define WRITE_BBREG4 0x79
#define WRITE_BBREG5 0x7B
#define WRITE_BBREG6 0x7D
#define WRITE_RSSITHCCA 0x7F

#define TX_POWER_LEVEL 0x00
#define RFCTRL1_VAL 0x02
#define PAN_COORDINATOR 0
#define DEVICE_TYPE PAN_COORDINATOR
#define PANID_INIT_VALUE 0x4742
#define AUTO_ACK_CONTROL 0
#define MULTICHANNEL_SUPPORT 0
#define RX_BUFF_SIZE 144

extern unsigned short macPANId;

void writeLong(unsigned short addr, unsigned char value);
void writeShort(unsigned char addr, unsigned char value);
unsigned char readLong(unsigned short addr);
unsigned char readShort(unsigned char addr);
int transceiverReset(int state);
int transceiverWake(int state);
int transceiverInit(unsigned char * mac64Address);
void transceiverSetDeviceAddress(int PANID, int shortAddress);
void transceiverAutoACK(unsigned char autoack);
void transceiverPing(unsigned short macAddress);
void transceiverSend(unsigned short macAddress, unsigned char payload_size, unsigned char *payload, unsigned char node_id);
static gboolean interrupt(GIOChannel *channel, GIOCondition condition, gpointer user_data);

void setChannel(unsigned char channel);
unsigned char getChannel(void);
void setRxChannel(unsigned char channel);
void setTxPower(unsigned char power_level);
unsigned char getTxPower(void);

#endif