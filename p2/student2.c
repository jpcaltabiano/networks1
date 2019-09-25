#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

	 This code should be used for unidirectional or bidirectional
	 data transfer protocols from A to B and B to A.
	 Network properties:
	 - one way network delay averages five time units (longer if there
		 are other messages in the channel for GBN), but can be larger
	 - packets can be corrupted (either the header or the data portion)
		 or lost, according to user-defined probabilities
	 - packets may be delivered out of order.

	 Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/



/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

enum state {
	wait_l5,
	wait_ack
};

struct sender {
	enum state state;
	struct pkt last_pkt;
	struct pkt last_ack;
};

struct receiver {
	struct pkt last_ack;
};

extern int TraceLevel;
struct sender sender;
struct receiver receiver;

int checksum(struct pkt packet);

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {

	//add msg to queu
	//if 

	//if A is still waiting for an ACK for a packet it has sent
	// if (sender.state != wait_l5) {
	// 	//TODO: buffer message somehow until ACK is received
	// 	if (TraceLevel == 2) {
	// 		printf("A_output: not yet ACKed, drop message: %s\n", message.data);
	// 	}
	// 	// return;
	// } else if (TraceLevel == 2) printf("A_output: sending packet: %s\n", message.data);

	//semaphore, wait while A is still expecting to recv an ACK
	while (sender.state == wait_ack);

	struct pkt send_pkt;
	int seq = (sender.last_pkt.seqnum == 0) ? 1 : 0;
	send_pkt.seqnum = seq;
	memmove(send_pkt.payload, message.data, MESSAGE_LENGTH);
	send_pkt.checksum = checksum(send_pkt);

	sender.last_pkt = send_pkt;
	sender.state = wait_ack;
	tolayer3(AEntity, send_pkt);
	startTimer(AEntity, 1000); //TODO: change this value, look at RTT slides in 14
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {

	//if received packet is corrupted, resend current packet
	if (packet.checksum != checksum(packet)) {
		if (TraceLevel == 2) printf("A_input: pkt corrupted, resending");
		//TODO: Does stoptimer need to be called before starttimer here?
		tolayer3(AEntity, sender.last_pkt);
		// startTimer(AEntity, 1000);
		return;
	}

	//if received packet is ACK for previously sent packet, resent current packet
	if (packet.acknum == sender.last_ack.acknum) {
		//TODO: Does stoptimer need to be called before starttimer here?
		tolayer3(AEntity, sender.last_pkt);
		// startTimer(AEntity, 1000);
		return;
	}
	
	//set the last received ack to this one
	stopTimer(AEntity);
	sender.last_ack = packet;
	sender.state = wait_l5; //now ready to recv another packet
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {

	if (TraceLevel == 2) printf("A_interrupt: resending last packet\n");
	tolayer3(AEntity, sender.last_pkt);
	//TODO: Does stoptimer need to be called before starttimer here?
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	sender.state = wait_l5;
	struct pkt pkt = {0};
	sender.last_pkt = pkt;
	sender.last_ack = pkt;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {

	//packet is corrupted
	if (packet.checksum != checksum(packet)) {
		if (TraceLevel == 2) printf("B_input: Received corrupted pkt, sending prev ACK\n");
		tolayer3(BEntity, receiver.last_ack);
		return;
	}

	//packet has same seq as the last ACK, likley duplicate
	if (packet.seqnum == receiver.last_ack.acknum) {
		if (TraceLevel == 2) printf("B_input: Received duplicate packet, resending ACK\n");
		tolayer3(BEntity, receiver.last_ack);
		return;
	}

	if (TraceLevel == 2) printf("B_input: Received packet, sending ACK\n");
	struct pkt ack_pkt;
	struct msg msg;
	// char* pload;
	ack_pkt.acknum = packet.seqnum;
	// ack_pkt.payload = pload;
	ack_pkt.checksum = checksum(ack_pkt); //TODO: does checksum work when no msg defined?
	tolayer3(BEntity, ack_pkt);

	for (int i = 0; i < MESSAGE_LENGTH; i++) {
		msg.data[i] = packet.payload[i];
	}

	if (TraceLevel == 2) printf("B_input: Sending message to layer 5\n");
	tolayer5(BEntity, msg);

	receiver.last_ack = ack_pkt;

}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
	struct pkt pkt = {0};
	receiver.last_ack = pkt;
}

/*
 * checksum(packet) computes the checksum for a packet. The checksum is a
 * sum of a character (ASCII value) times it's array position
 */
int checksum(struct pkt packet) {
	int csum = 0;
	for (int i = 0; i < MESSAGE_LENGTH; i++) {
		csum += (i * (int)(packet.payload[i]));
		csum += (i * packet.acknum);
	}
	return csum;
}