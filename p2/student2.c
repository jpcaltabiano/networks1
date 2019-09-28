#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
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

#define ALPHA 0.125
#define BETA 0.25

enum state {
	wait_l5,
	wait_ack
};

struct sender {
	//0 if pkt already in transmission, 1 if no pkt being currently transmitted
	//if 1 then ok to send another pkt
	int sending;
	int seq; //seqnum of previously sent pkt
	struct pkt last_pkt;
	struct pkt last_ack;
};

struct receiver {
	struct pkt last_ack;
};

struct pkt_node {
	struct pkt pkt;
	struct pkt_node *next;
};

struct pkt_queue {
	struct pkt_node *head, *tail;
};

extern int TraceLevel;
struct sender sender;
struct receiver receiver;
struct pkt_queue *queue;
struct timeval start, end;
double avg_rtt = 15;
double dev_rtt = 0;
double timeout = 15;
int interrupts = 0;

int checksum(struct pkt packet);
void add_to_queue(struct pkt_node* node);
struct pkt_node* get_pkt();
int queue_length();

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {

	//create packet to be added to queue
	struct pkt queue_pkt;
	memmove(queue_pkt.payload, message.data, MESSAGE_LENGTH);

	//create queue node from packet and add to queue
	struct pkt_node *node = (struct pkt_node*)malloc(sizeof(struct pkt_node));
	node->pkt = queue_pkt;
	node->next = NULL;
	add_to_queue(node);
	if (TraceLevel >= 2) printf("A_output: Incoming message added to queue\n");

	//if a packet is in transmission then do not send another
	if (!sender.sending) return;

	//otherwise, get packet from head of queue and send it
	struct pkt_node *send_pkt;
	send_pkt = get_pkt();
	sender.last_pkt = send_pkt->pkt;
	sender.sending = 0;
	tolayer3(AEntity, send_pkt->pkt);
	if (TraceLevel >= 2) printf("A_output: sending message...\n");
	startTimer(AEntity, timeout);
	gettimeofday(&start, NULL);
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

	//update timeout value with data from most recent round trip
	gettimeofday(&end, NULL);
	double sample_rtt = end.tv_usec - start.tv_usec;
	avg_rtt = ((1 - ALPHA) * avg_rtt) + (ALPHA * fabs(sample_rtt));
	//artificially maintain avg minimum of 5 to prevent recursive diminishing to <0.1
	if (avg_rtt < 5) avg_rtt = 5;
	dev_rtt = ((1 - BETA) * dev_rtt) + (BETA * fabs(sample_rtt));
	timeout = avg_rtt + (4 * dev_rtt);
	if (TraceLevel >= 2) printf("timeout: %f, sample: %f, avg RTT: %f, start: %ld, end: %ld\n", timeout, sample_rtt, avg_rtt, start.tv_usec, end.tv_usec);

	//if received packet is corrupted, resend current packet
	if (packet.checksum != checksum(packet)) {
		if (TraceLevel >= 2) printf("A_input: pkt corrupted, resending\n\
	packet.checksum: %d, checksum(packet): %d\n", packet.checksum, checksum(packet));
		tolayer3(AEntity, sender.last_pkt);
		stopTimer(AEntity);
		startTimer(AEntity, timeout);
		return;
	}

	//if received packet is ACK for previously sent packet, ignore
	if (packet.acknum == sender.last_ack.acknum) {
		if (TraceLevel >= 2) printf("A_input: duplicate ACK\n");
		return;
	}
	
	if (TraceLevel >= 2) printf("A_input: Received good ACK\n");
	sender.last_ack = packet;

	stopTimer(AEntity);
	struct pkt_node *send_pkt;
	if (queue_length() == 0) {
		if (TraceLevel >= 2) printf("A_input: Queue is empty, awaiting new message\n");
		sender.sending = 1;
		return;
	}
	send_pkt = get_pkt();
	sender.last_pkt = send_pkt->pkt;
	sender.sending = 0;
	tolayer3(AEntity, send_pkt->pkt);
	if (TraceLevel >= 2) printf("A_input: sending message...\n");
	startTimer(AEntity, timeout);
	gettimeofday(&start, NULL);
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {

	interrupts++;
	if (TraceLevel >= 2) printf("interrupts: %d\n", interrupts);

	if (TraceLevel >= 2) printf("A_interrupt: resending last packet\n");
	tolayer3(AEntity, sender.last_pkt);
	startTimer(AEntity, timeout);
	if (TraceLevel >= 2) printf("A_interrupt: starting timer with timeout: %f\n", timeout);
	gettimeofday(&start, NULL);
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	sender.sending = 1;
	sender.seq = 1; //A_output assigns inverse so set to 1 for first pkt to have seqnum = 0

	struct pkt pkt;
	pkt.seqnum = 0;
	sender.last_pkt = pkt;

	struct pkt ack;
	ack.seqnum = 1;
	ack.acknum = 1;
	//20 char msg identical to B side's initial 'previous ACK' first case of first pkt being incorrect
	struct msg msg;
	int i;
	for (i = 0; i < MESSAGE_LENGTH; i++) msg.data[i] = 'a';
	memmove(ack.payload, msg.data, MESSAGE_LENGTH);
	ack.checksum = checksum(ack);
	sender.last_ack = ack;

	queue = (struct pkt_queue*)malloc(sizeof(struct pkt_queue));
	queue->head = queue->tail = NULL;	
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
	//weird edge case where seqnum and acknum were corrupted but somehow checksum was right
	//&& (packet.seqnum != 0 || packet.seqnum != 1)
	if (TraceLevel >= 2) printf("B_input: checksum: %d, calculated: %d\n", packet.checksum, checksum(packet));
	if (packet.checksum != checksum(packet)) {
		if (TraceLevel >= 2) printf("B_input: Received corrupted pkt, discarding\n\
		B_input: checksum: %d, calculated: %d\n", packet.checksum, checksum(packet));
		// tolayer3(BEntity, receiver.last_ack);
		return;
	}

	//packet has same seq as the last ACK, likley duplicate
	if (packet.seqnum == receiver.last_ack.acknum) {
		if (TraceLevel >= 2) printf("B_input: Received duplicate packet, resending ACK\n");
		tolayer3(BEntity, receiver.last_ack);
		return;
	}

	if (TraceLevel >= 2) printf("B_input: Received packet, sending ACK\n");
	struct pkt ack_pkt;
	struct msg msg;
	ack_pkt.acknum = ack_pkt.seqnum = packet.seqnum;
	int i;
	for (i = 0; i < MESSAGE_LENGTH; i++) {
		msg.data[i] = packet.payload[i];
		ack_pkt.payload[i] = packet.payload[i];
	}
	ack_pkt.checksum = checksum(ack_pkt);
	tolayer3(BEntity, ack_pkt);

	if (TraceLevel >= 2) printf("B_input: Sending message to layer 5\n");
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
	struct pkt ack;
	ack.seqnum = 1;
	ack.acknum = 1;
	//20 char msg identical to B side's initial 'previous ACK' for case of first pkt being incorrect
	struct msg msg;
	int i;
	for (i = 0; i < MESSAGE_LENGTH; i++) msg.data[i] = 'a';
	memmove(ack.payload, msg.data, MESSAGE_LENGTH);
	ack.checksum = checksum(ack);
	receiver.last_ack = ack;
}

/*
 * checksum(packet) computes the checksum for a packet. The checksum is a
 * sum of a character (ASCII value) times it's array position plus values derived 
 * from the ack and seq nums
 */
int checksum(struct pkt packet) {
	int csum = 0;
	int i;
	for (i = 0; i < MESSAGE_LENGTH; i++) {
		csum += (i * (int)(packet.payload[i]));
		csum += (i * (1 + packet.acknum) * (2 + packet.seqnum));
	}
	return csum;
}

//add to tail of queue
void add_to_queue(struct pkt_node* node) {
	node->pkt.seqnum = node->pkt.acknum = !(sender.seq);
	sender.seq = !(sender.seq);
	node->pkt.checksum = checksum(node->pkt);
	if (queue->tail == NULL){
		queue->head = queue->tail = node;
	} else {
		queue->tail->next = node;
		queue->tail = node;
	}
}

// Retrieve head of queue and update
struct pkt_node* get_pkt() {
	struct pkt_node *temp = queue->head;
	queue->head = queue->head->next;
	if (queue->head == NULL) queue->tail = NULL;
	return temp;
}

//returns length of queue
int queue_length() {
	int x = 0;
	struct pkt_node *current = queue->head;
	while (current != NULL) {
		x++;
		current = current->next;
	}
	return x;
}