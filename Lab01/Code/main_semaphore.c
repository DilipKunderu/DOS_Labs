/*  main.c  - main */

#include <xinu.h>

#define sBuff 10

pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

int32 consumed_count = 0;
const int32 CONSUMED_MAX = 100;

/* Define your circular buffer structure and semaphore variables here */
/* */
int32 buffer[sBuff];
int32 head, tail;

sid32 csem;
sid32 psem;

/* Place your code for entering a critical section here */
void mutex_acquire(sid32 mutex)
{
	/* */
}

/* Place your code for leaving a critical section here */
void mutex_release(sid32 mutex)
{
	/* */
}

/* Place the code for the buffer producer here */
process producer(void){
	int32 citem = 0;
	/* */
	while (1){
        wait(psem);
        citem++;                    
        buffer[head++] = citem;          
        head = head % sBuff;            
        kprintf("The produced item here becomes : %d \n", citem);
        signal(csem);
    }
	return OK;
}

/* Place the code for the buffer consumer here */
process consumer(void)
{
	/* Every time your consumer consumes another buffer element,
	 * make sure to include the statement:
	 *   consumed_count += 1;
	 * this will allow the timing function to record performance */
	/* */
	int32 pitem = 0;
    while(1){
        wait(csem);
        pitem = buffer[tail++];                 
        tail = tail % sBuff;
        kprintf("The consumed item here becomes: %d \n", pitem);
        consumed_count += 1;
        signal(psem);
    }
	return OK;
}


/* Timing utility function - please ignore */
process time_and_end(void)
{
	int32 times[5];
	int32 i;

	for (i = 0; i < 5; ++i)
	{
		times[i] = clktime_ms;
		yield();

		consumed_count = 0;
		while (consumed_count < CONSUMED_MAX * (i+1))
		{
			yield();
		}

		times[i] = clktime_ms - times[i];
	}

	kill(producer_id);
	kill(consumer_id);

	for (i = 0; i < 5; ++i)
	{
		kprintf("TIME ELAPSED (%d): %d\n", (i+1) * CONSUMED_MAX, times[i]);
	}
}

process	main(void)
{
	recvclr();

	/* Create the shared circular buffer and semaphores here */
	/* */
	psem = semcreate (sBuff);
	csem = semcreate (0);
	
	head = tail = 0;
	
	producer_id = create(producer, 4096, 50, "producer", 0);
	consumer_id = create(consumer, 4096, 50, "consumer", 0);
	timer_id = create(time_and_end, 4096, 50, "timer", 0);

	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	/* Uncomment the following line for part 3 to see timing results */
	resume(timer_id);
	resched_cntl(DEFER_STOP);

	return OK;
}
