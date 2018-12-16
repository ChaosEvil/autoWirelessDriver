#include "transceivercom.h"
#include "packetHandler.h"
#include "nodes.h"

int radioNodeId = 10;

int timeout = 0;
int highCount = 0;
int normalCount = 0;
int lowCount = 0;

void doPacketService() {
    if (highCount >= 20 && normalCount >= 10 && lowCount >= 2) {
        highCount = 0;
        normalCount = 0;
        lowCount = 0;
    }
    
    struct node * packet = NULL;
    
    if (getTotalNodes(LOW_PRIORITY) > 0 
            && (getTotalNodes(NORMAL_PRIORITY) == 0 || normalCount >= 10) 
            && (getTotalNodes(HIGH_PRIORITY) == 0 || highCount >= 20)) {
        
        packet = processNode(LOW_PRIORITY);
        lowCount++;
        
    } else if(getTotalNodes(NORMAL_PRIORITY) > 0 
            && (getTotalNodes(HIGH_PRIORITY) == 0 || highCount >= 20)) {
        
        packet = processNode(NORMAL_PRIORITY);
        normalCount++;
        
    } else if(getTotalNodes(HIGH_PRIORITY) > 0) {
        packet = processNode(HIGH_PRIORITY);
        highCount++;
        
    }
    
    if (packet != NULL) {
        if (packet->data != NULL) {
            unsigned char payload_size = sizeof(packet->data);
            transceiverSend(packet->address, payload_size, packet->data, radioNodeId);
        }
    }
    
    sleep(2);
    doPacketService();
}