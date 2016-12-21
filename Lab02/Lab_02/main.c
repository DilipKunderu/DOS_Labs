/*  main.c  - main */

#include <xinu.h>

pid32 process1_pid;
pid32 process2_pid;
pid32 process3_pid;
pid32 process4_pid;

int32 status_check;

process p1(void)
{
	umsg32 sent_mesg;
	sent_mesg = 10;
	status_check = sendMsg(p2_id, sent_mesg);
	if( status_check == OK)
		kprintf("%d message Sent %d\n", p1_id, sent_mesg);
	else
		kprintf("%d message send failed %d\n", p1_id, sent_mesg);
	return OK;
}

process p2(void)
{
	umsg32 recv_mesg;
	recv_mesg = receiveMsg();
	return OK;
}

process p3(void)
{
	umsg32 sent_mesgs;
	sent_mesgs = sendMsgs();
	sent_mesgs = 10;
	
	int32 prcLst[] = {p1_id, p2_id, p4_id};

	int k;

	for(k = 0; k < sizeof(prcLst)/sizeof(prcLst[0]); k++) {
		status_check = sendMsg(prcLst[k], sent_mesg);
		if( status_check == OK)
			kprintf("%d message Sent %d\n", prcLst[k], sent_mesg);
		else
			kprintf("%d message send failed %d\n", prcLst[k], sent_mesg);
	}
	return OK;
}

process p4(void)
{
	umsg32 recv_mesgs;
	recv_mesgs = receiveMsgs();
	return OK;
}

process p5(void)
{
	umsg32 sent_nmesg[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
	sent_nmesg = sendnMsg();

	int k;

	for(k = 0; k < sizeof(sent_nmesg)/sizeof(sent_nmesg[0]); k++) {
		status_check = sendMsg(p6_id, sent_nmesg[k]);
		if( status_check == OK)
			kprintf("%d message Sent %d\n", p6_id, sent_nmesg[k]);
		else
			kprintf("%d message send failed %d\n", p6_id, sent_nmesg[k]);
	}
	return OK;
	
}

process p6(void)
{
	umsg32 recv_nmesg;
	recv_nmesg = receiveMsgs();
	return OK;
}

process	main(void)
{
	recvclr();
	
	p1_id = create(p1, 4096, 50, "Process1", 0);
	p2_id = create(p2, 4096, 50, "Process2", 0);
	p3_id = create(p3, 4096, 50, "Process3", 0);
	p4_id = create(p4, 4096, 50, "Process4", 0);
	p5_id = create(p5, 4096, 50, "Process5", 0);
	p6_id = create(p6, 4096, 50, "Process6", 0);

	resched_cntl(DEFER_START);

	resume(p1_id);
	resume(p2_id);
	resume(p3_id);
	resume(p4_id);
	resume(p5_id);
	resume(p6_id);

	resched_cntl(DEFER_STOP);

	return OK;
}
