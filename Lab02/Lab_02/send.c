/* send.c - send */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  send  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	send(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prhasmsg) {
		restore(mask);
		return SYSERR;
	}
	prptr->prmsg = msg;		/* Deliver message		*/
	prptr->prhasmsg = TRUE;		/* Indicate message is waiting	*/

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}

syscall sendMsg(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	
	if((prptr->cb_f == prptr->cb_r + 1)||(prptr->cb_r == 9 && prptr->cb_f == 0)){
		restore(mask);
		return SYSERR;
	}else{
		 if(prptr->cb_r == 9 && !(prptr->cb_f == 0))
			prptr->cb_r = -1;
		(prptr->cb_r)++;
		prptr->q_mesg[prptr->cb_r] = msg;
		kprintf("%d r %d r %d\n", pid, msg, prptr->cb_r);
		if(prptr->cb_r == -1)
			prptr->cb_f = 0;
	}

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}

uint32 sendMsgs
(
	pid32 pid,
	umsg32* msgs,
	uint32 msg_count		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	if (msg_count > 10) {
		restore(mask);
		return SYSERR;
	}


	prptr = &proctab[pid];
	uint32 i;
	for(i = 0; i < msg_count; i++){
		if((prptr->cb_f == prptr->cb_r + 1) || (prptr->cb_f == 0 && prptr->cb_r == 9)){
			restore(mask);
			return SYSERR;
		}else{
			if(prptr->cb_r == 9 && !(prptr->cb_f == 0))
				prptr->cb_r = -1;
			++(prptr->cb_r);
			prptr->q_mesg[prptr->cb_r] = msgs[i];
			if(prptr->cb_f == -1)
				prptr->cb_f = 0;
		}
	}

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}

uint32 sendnMsg(
	uint32 pid_count,
	pid32* pids,
	umsg32 msg
)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	
	mask = disable();
	if (pid_count > 3) {
		restore(mask);
		return SYSERR;
	}
	uint32 i,count=0;
	for(i=0; i< pid_count; i++){
		if (isbadpid(pids[i])) {
			count++;
			continue;
		}

		prptr = &proctab[pids[i]];
		
		if((prptr->cb_f == prptr->cb_r + 1) || (prptr->cb_f == 0 && prptr->cb_r == 9)){
			count++;
			continue;
		}else{
			if(prptr->cb_r == 9 && !(prptr->cb_f != 0))
				prptr->cb_r = -1;
			(prptr->cb_r)++;
			prptr->q_mesg[prptr->cb_r] = msg;
			 if(prptr->cb_f == -1)
				prptr->cb_f = 0;
		}
		

		/* If recipient waiting or in timed-wait make it ready */

		if (prptr->prstate == PR_RECV) {
			ready(pids[i]);
		} else if (prptr->prstate == PR_RECTIM) {
			unsleep(pids[i]);
			ready(pids[i]);
		}
	}
	restore(mask);		/* Restore interrupts */
	if(pid_count == count)
		return SYSERR;
        else
	return pid_count - count;
}

