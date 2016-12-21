/* receive.c - receive */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receive(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &proctab[currpid];
	if (prptr->prhasmsg == FALSE) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}
	msg = prptr->prmsg;		/* Retrieve message		*/
	prptr->prhasmsg = FALSE;	/* Reset message flag		*/
	restore(mask);
	return msg;
}

umsg32	receiveMsg(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &proctab[currpid];
	if(prptr->cb_f == -1 && prptr->cb_ == -1){
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}
	
	msg = prptr->msgqueue[prptr->cb_f];		/* Retrieve message*/
	kprintf("%d %d %d\n",currpid, msg, prptr->cb_f);
	(prptr->cb_f)++;
	if(prptr->cb_f == 10)
		prptr->cb_f = 0;
	if(prptr->cb_f - 1 == prptr->cb_r){
		prptr->cb_f = -1;
		prptr->cb_r = -1;
	}
	restore(mask);
	return msg;
}

syscall receiveMsgs(
	umsg32* msgs,
	uint32 msg_count
)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (msg_count > 10) {
		restore(mask);
		return SYSERR;
	}
	prptr = &proctab[currpid];
	uint32 i = 0;
	while(i < msg_count){
		if(prptr->cb_f == -1 && prptr->cb_r == -1){
			prptr->prstate = PR_RECV;
			resched();		
		}
		msgs[i] = prptr->msgqueue[prptr->cb_f];	
		(prptr->cb_f)++;
		if(prptr->cb_f == 10)
			prptr->cb_f = 0;
		if(prptr->cb_f - 1 == prptr->cb_r){
			prptr->cb_f = -1;
			prptr->cb_r = -1;
		}
		i++;
	}
	restore(mask);
	return OK;
}
