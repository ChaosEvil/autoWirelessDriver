#ifndef nodes_h
#define nodes_h

extern const int LOW_PRIORITY;
extern const int NORMAL_PRIORITY;
extern const int HIGH_PRIORITY;

// Estrutura de um sensor/atuador da rede
struct node {
    unsigned short address; // endereco do no
    unsigned char * data; // dado a ser enviado
    
    int priority; // prioridade na rede
    
    struct node * next; // proximo no da lista encadeada
    struct node * prev; // anterior no da lista encadeada
};

struct nodeList {
    struct node * firstLowNode;
    struct node * lastLowNode;
    
    struct node * firstNormalNode;
    struct node * lastNormalNode;
    
    struct node * firstHighNode;
    struct node * lastHighNode;
    
    int nodeLowCount;
    int nodeNormalCount;
    int nodeHighCount;
};

struct nodeList nodesList;

void initNodes();
struct node * processNode(int priority);
void newNode(unsigned short address, unsigned char * data, int priority);
void removeNode(struct node * nNode, int freeNode);
int getTotalNodes(int priority);

#endif