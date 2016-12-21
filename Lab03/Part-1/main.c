#include <xinu.h>
pid32 process1_id;
const int32 TMAX = 256*256;
const int32 SMAX = 8;

typedef struct Queue {
	int head, tail, size, capacity;
	int *elements;
}Queue;

Queue * Qcreate(int n) {
	Queue *Q;
	Q = (Queue *)getmem(sizeof(Queue));
	Q->elements = (int *)getmem(sizeof(int)*n);
	Q->size = 0;
	Q->capacity = n;
	Q->head = 0;
	Q->tail = -1;
	return Q;
}

void deQ(Queue *Q) {
	if(Q->size == 0) {
		kprintf("Queue is empty \n");
		return;
	} else {
		Q->size--;
		Q->head++;
		if(Q->head == Q->capacity) {
			Q->head = 0;
		}
	} 
	return;
}

int head(Queue *Q) {
	if(Q->size==0) {
		kprintf("Queue is empty\n");
		exit(0);
	}
	return Q->elements[Q->head];
}

void enQ(Queue *Q, int element) {
	if(Q-> size == Q-> capacity) 
		kprintf("Queue is full\n");
	else {
		Q->size++;
		Q->tail = Q->tail+1;
		if(Q->tail == Q->capacity) 
			Q->tail =0;

		Q->elements[Q->tail] = element;
	} 
	return;
}
struct Queue *publishQ;

struct TopicData {
	topic16 topic;
	uint32 data;
};

struct TopicData topics_and_data[256*256];

struct Subscribers {
	topic16 topic; 
	int32 handler_p[8];
};

struct Subscribers topics_subscribers [256*256];

void Table_init () {
	int i, j; 

	for(i=0; i<TMAX; i++) {
		topics_subscribers[i].topic = (topic16)i;
		
		for(j=0; j<SMAX; j++) 
			topics_subscribers[i].handler_p[j] = 0;
	}
}

//to avoid panic Exception
void Topic_Data_init () {
	int i;
	
	for(i=0; i<TMAX; i++) 
		topics_and_data[i].topic = (topic16) i;
}

void Full_init() {
	Table_init ();
	publishQ = Qcreate(15); 
}

handlerA(topic16 topic, uint32 data){
	kprintf("Process A handler called on topic %X with data %d \n ", topic, data);
}

handlerB(topic16 topic, uint32 data){
	kprintf("Process B handler called on topic %X with data %d \n ", topic, data);
}

handlerC(topic16 topic, uint32 data){
	kprintf("Process C handler called on topic %X with data %d \n ", topic, data);
}

syscall subscribe(topic16 topic, void (*handler)(topic16, uint32)) {
	int subscribed = 0;
	struct Subscribers *subscribers = &topics_subscribers[(uint16)topic];
	int subscribed_count = sizeof(subscribers->handler_p)/sizeof(subscribers->handler_p[0]);
	if(subscribed_count <= SMAX) {
		int i;
		for(i=0; i<SMAX; i++)
			if(subscribers->handler_p[i] == (*handler)) {
				subscribed = 1;
				break;
			}
		
		if(subscribed == 0) {
			int j;
			for(j = 0; j < SMAX; j++)
				if(subscribers->handler_p[j] == 0) {
					kprintf("subscribers handler_p[%d] : %d", j, subscribers->handler_p[j]);
					subscribers->handler_p[j] = (*handler);
					kprintf(" subscribes to the topic %d", topic);
					kprintf("handler %d", (*handler));
					kprintf("now the value subscribers.handler_p[%d] : %d \n", j, subscribers->handler_p[j]);
					break;
				}
		} 
	}
	else kprintf("ERROR:MaximumLimitReachedException\n");
	return OK;
}

syscall publish(topic16 topic, uint32 data) {
	kprintf("Data : %d published to topic : %X \n", data, topic);
	topics_and_data[(int)topic].data = data;
	enQ(publishQ, topic);	
	return OK;
}

void handler_over_topic(topic16 topic, uint32 data) {
	struct Subscribers subscribers = topics_subscribers[(uint16)topic];
	int subscribed_count = sizeof(subscribers.handler_p)/sizeof(int32);
	typedef void func(topic16, uint32);
	
	int i;
	for(i = 0; i < subscribed_count; i++) 
		if(subscribers.handler_p[i] != 0) {
			func* handler_ptr = (func*)(subscribers.handler_p[i]);
			handler_ptr(topic, data);
			kprintf("Handler called.\n");
		}
}

process broker() {
	int publish_count = publishQ->size;
	
	int i;
	for(i=0; i < publish_count; i++) {	
		topic16 topic = (topic16) head(publishQ);
		uint32 data = topics_and_data[(int)topic].data;

		struct Subscribers subscribers = topics_subscribers[(uint16)topic];
		int subscribed_count = sizeof(subscribers.handler_p)/sizeof(int32);
		typedef void func(topic16, uint32);
		int j;
		for(j = 0; j < subscribed_count; j++) {
			if(subscribers.handler_p[j] != 0) {
				func* handler_ptr = (func*)(subscribers.handler_p[j]);
				handler_ptr(topic, data);
				kprintf("handler called. \n");
			}
		deQ(publishQ);
		}
	}
}

process processA(void) {
	subscribe(0x013F, &handlerA);
	return;
}

process processB(void) {
	subscribe(0x023F, &handlerB);
	return;
}

process processC(void) {
	subscribe(0x013F, &handlerC);
	return;
}

process main(void)
{
	recvclr();
	Full_init();
	process1_id = create(processA, 4096, 50, "processA", 0);
	resume(process1_id);
	kprintf("Process1 ID : %d \n", process1_id);
	resume(create(processB, 4096, 50, "processB", 0));
	resume(create(processC, 4096, 50, "processC", 0));
	publish(0x003F, 77);
	publish(0xFF3F, 88);
	resume(create(broker, 4096, 50, "broker", 0));
	return OK;
}
