#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   
   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
       are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
       or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
       (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0 /* change to 1 if you're doing extra credit */
                        /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */


/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

enum SenderState
{
    WAIT_LAYER5,
    WAIT_ACK
};

struct Sender
{
    enum SenderState state;
    int seq;
    float estimated_rtt;
    struct pkt last_packet;
} A;

struct Receiver
{
    int seq;
} B;

int get_checksum(struct pkt *packet)
{
    int checksum = 0;
    checksum += packet->seqnum;
    checksum += packet->acknum;
    for (int i = 0; i < 20; ++i)
        checksum += packet->payload[i];
    return checksum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    if (A.state != WAIT_LAYER5)
    {
        printf("  A_output: not yet acked. drop the message: %s\n", message.data);
        return;
    }
    printf("  A_output: send packet: %s\n", message.data);
    struct pkt packet;
    packet.seqnum = A.seq;
    memmove(packet.payload, message.data, 20);
    packet.checksum = get_checksum(&packet);
    A.last_packet = packet;
    A.state = WAIT_ACK;
    tolayer3(0, packet);
    startTimer(0, A.estimated_rtt);
}

/* need be completed only for extra credit */
void B_output(struct msg message)
{
    printf("  B_output: uni-directional. ignore.\n");
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    if (A.state != WAIT_ACK)
    {
        printf("  A_input: A->B only. drop.\n");
        return;
    }
    if (packet.checksum != get_checksum(&packet))
    {
        printf("  A_input: packet corrupted. drop.\n");
        return;
    }

    if (packet.acknum != A.seq)
    {
        printf("  A_input: not the expected ACK. drop.\n");
        return;
    }
    printf("  A_input: acked.\n");
    stopTimer(0);
    A.seq = 1 - A.seq;
    A.state = WAIT_LAYER5;
}

/* called when A's timer goes off */
void A_timerinterrupt(void)
{
    if (A.state != WAIT_ACK)
    {
        printf("  A_timerinterrupt: not waiting ACK. ignore event.\n");
        return;
    }
    printf("  A_timerinterrupt: resend last packet: %s.\n", A.last_packet.payload);
    tolayer3(0, A.last_packet);
    startTimer(0, A.estimated_rtt);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(void)
{
    A.state = WAIT_LAYER5;
    A.seq = 0;
    A.estimated_rtt = 15;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

void send_ack(int AorB, int ack)
{
    struct pkt packet;
    packet.acknum = ack;
    packet.checksum = get_checksum(&packet);
    tolayer3(AorB, packet);
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    if (packet.checksum != get_checksum(&packet))
    {
        printf("  B_input: packet corrupted. send NAK.\n");
        send_ack(1, 1 - B.seq);
        return;
    }
    if (packet.seqnum != B.seq)
    {
        printf("  B_input: not the expected seq. send NAK.\n");
        send_ack(1, 1 - B.seq);
        return;
    }
    struct msg msgToSend;
    for(int i = 0; i < MESSAGE_LENGTH; i++){
      msgToSend.data[i] = packet.payload[i];
    }
    printf("  B_input: recv message: %s\n", packet.payload);
    printf("  B_input: send ACK.\n");
    send_ack(1, B.seq);
    tolayer5(1, msgToSend);
    B.seq = 1 - B.seq;
}

/* called when B's timer goes off */
void B_timerinterrupt(void)
{
    printf("  B_timerinterrupt: B doesn't have a timer. ignore.\n");
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(void)
{
    B.seq = 0;
}
