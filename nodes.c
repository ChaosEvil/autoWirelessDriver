#include <stdio.h>
#include <stdlib.h>
#include "nodes.h"

const int LOW_PRIORITY = 0;
const int NORMAL_PRIORITY = 1;
const int HIGH_PRIORITY = 2;

void initNodes() {
    nodesList.firstHighNode = NULL;
    nodesList.firstNormalNode = NULL;
    nodesList.firstLowNode = NULL;
    
    nodesList.lastHighNode = NULL;
    nodesList.lastNormalNode = NULL;
    nodesList.lastLowNode = NULL;
    
    nodesList.nodeHighCount = 0;
    nodesList.nodeNormalCount = 0;
    nodesList.nodeLowCount = 0;
}

struct node * processNode(int priority) {
    struct node * last;
    
    if (priority == LOW_PRIORITY && nodesList.nodeLowCount > 0) {
        last = nodesList.lastLowNode;
        removeNode(nodesList.lastLowNode, 0);
        
        return last;
        
    } else if(priority == NORMAL_PRIORITY && nodesList.nodeNormalCount > 0) {
        last = nodesList.lastNormalNode;
        removeNode(nodesList.lastNormalNode, 0);
        
        return last;
        
    } else if(priority == HIGH_PRIORITY && nodesList.nodeHighCount > 0) {
        last = nodesList.lastHighNode;
        removeNode(nodesList.lastHighNode, 0);
        
        return last;
        
    }
    
    return NULL;
}

void newNode(unsigned short address, unsigned char * data, int priority) {
    struct node * nNode = (struct node *) malloc(sizeof(struct node));
    
    nNode->address = address;
    nNode->data = data;
    nNode->priority = priority;
    
    if (priority == LOW_PRIORITY) {
        if (nodesList.nodeLowCount > 0) {
            nNode->next = nodesList.firstLowNode;
            nodesList.firstLowNode->prev = nNode;
        } else {
            nodesList.lastLowNode = nNode;
            nNode->next = NULL;
        }
        
        nNode->prev = NULL;
        nodesList.firstLowNode = nNode;
        
        nodesList.nodeLowCount++;
        
    } else if(priority == NORMAL_PRIORITY) {
        if (nodesList.nodeNormalCount > 0) {
            nNode->next = nodesList.firstNormalNode;
            nodesList.firstNormalNode->prev = nNode;
        } else {
            nodesList.lastNormalNode = nNode;
            nNode->next = NULL;
        }
        
        nNode->prev = NULL;
        nodesList.firstNormalNode = nNode;
        
        nodesList.nodeNormalCount++;
        
    } else if(priority == HIGH_PRIORITY) {
        if (nodesList.nodeHighCount > 0) {
            nNode->next = nodesList.firstHighNode;
            nodesList.firstHighNode->prev = nNode;
        } else {
            nodesList.lastHighNode = nNode;
            nNode->next = NULL;
        }
        
        nNode->prev = NULL;
        nodesList.firstHighNode = nNode;
        
        nodesList.nodeHighCount++;
        
    }
}

void removeNode(struct node * nNode, int freeNode) {
    struct node * listNode;
    int priority = nNode->priority;
    
    if (priority == LOW_PRIORITY) {
        listNode = nodesList.firstLowNode;
        
        if (listNode == nNode) {
            nodesList.firstLowNode = listNode->next;
            nodesList.firstLowNode->prev = NULL;
        } else {
            while(listNode->next != NULL) {
                if (listNode->next == nNode) {
                    if (listNode->next == nodesList.lastLowNode) {
                        nodesList.lastLowNode = listNode;
                        listNode->next = NULL;
                    } else {
                        listNode->next = nNode->next;
                        listNode->next->prev = listNode;
                    }
                    
                    break;
                }
                
                listNode = listNode->next;
            }
        }
        
        nodesList.nodeLowCount--;
        
    } else if(priority == NORMAL_PRIORITY) {
        listNode = nodesList.firstNormalNode;
        
        if (listNode == nNode) {
            nodesList.firstNormalNode = listNode->next;
            nodesList.firstNormalNode->prev = NULL;
        } else {
            while(listNode->next != NULL) {
                if (listNode->next == nNode) {
                    if (listNode->next == nodesList.lastNormalNode) {
                        nodesList.lastNormalNode = listNode;
                        listNode->next = NULL;
                    } else {
                        listNode->next = nNode->next;
                        listNode->next->prev = listNode;
                    }
                    
                    break;
                }
                
                listNode = listNode->next;
            }
        }
        
        nodesList.nodeNormalCount--;
        
    } else if(priority == HIGH_PRIORITY) {
        listNode = nodesList.firstHighNode;
        
        if (listNode == nNode) {
            nodesList.firstHighNode = listNode->next;
            nodesList.firstHighNode->prev = NULL;
        } else {
            while(listNode->next != NULL) {
                if (listNode->next == nNode) {
                    if (listNode->next == nodesList.lastHighNode) {
                        nodesList.lastHighNode = listNode;
                        listNode->next = NULL;
                    } else {
                        listNode->next = nNode->next;
                        listNode->next->prev = listNode;
                    }
                    
                    break;
                }
                
                listNode = listNode->next;
            }
        }
        
        nodesList.nodeHighCount--;
        
    }
    
    if (freeNode != 0)
        free(nNode);
}

int getTotalNodes(int priority) {
    if (priority == LOW_PRIORITY)
        return nodesList.nodeLowCount;
    else if(priority == NORMAL_PRIORITY)
        return nodesList.nodeNormalCount;
    else if(priority == HIGH_PRIORITY)
        return nodesList.nodeHighCount;
    
    return 0;
}
