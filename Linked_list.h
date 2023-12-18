struct LinkedList {
  int id;
  double sensorData;
  struct LinkedList *next;
};

void insertFirst(struct LinkedList **first, struct LinkedList *el);

int isMember(struct LinkedList **first, struct LinkedList *el);

void remover(struct LinkedList **first, struct LinkedList *el);

struct LinkedList *createNode(int i, double data);

struct LinkedList *readSensor(int id);

struct LinkedList *sort(struct LinkedList **first);

int size(struct LinkedList **first);

void release(struct LinkedList **first);

void printList(struct LinkedList **first);

int isMin(struct LinkedList *currentNode, struct LinkedList *minNode);

int isMax(struct LinkedList *currentNode, struct LinkedList *maxNode);

double getSensorData(struct LinkedList *el);

void pointToNext(struct LinkedList *el);