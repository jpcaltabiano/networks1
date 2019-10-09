#include <stdio.h>
#include <string.h>
#include "project3.h"

extern int TraceLevel;
extern float clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};

void printdt3( int MyNodeNumber, struct NeighborCosts *neighbor, 
		struct distance_table *dtptr );
struct distance_table dt3;
struct NeighborCosts   *neighbor3;

/* students to write the following two routines, and maybe some others */

void rtinit3() {
    printf("At time t=%f, rtinit3() called.\n", clocktime);

    struct RoutePacket pkt;

    neighbor3 = getNeighborCosts(3);

    for (int i = 0; i < MAX_NODES; i++) {
        for (int j = 0; j < MAX_NODES; j++) {
            if (i == j) dt3.costs[i][j] = neighbor3->NodeCosts[j];
            else dt3.costs[i][j] = INFINITY;
        }
    }

    int min_arr[MAX_NODES];
    int lmin;

    for (int i = 0; i < MAX_NODES; i++) {
        lmin = INFINITY;
        for (int j = 0; j < MAX_NODES; j++) {
            if (dt3.costs[i][j] < lmin) {
                lmin = dt3.costs[i][j];
            }
        }
        min_arr[i] = lmin;
    }

    printf("At time t=%f, node 3 initial distance vector: ", clocktime);
    for(int j = 0; (j < MAX_NODES) && (j < 4); j++){
        printf("%d   ", min_arr[j]);
    }
    printf("\n");
    
    if (TraceLevel > 1) printdt3(3, neighbor3, &dt3);

    for (int i = 0; i < neighbor3->NodesInNetwork; i++) {
        if ((i != 3) && (neighbor3->NodeCosts[i] != INFINITY)) {
            pkt.sourceid = 3;
            pkt.destid = i;

            memcpy(pkt.mincost, min_arr, sizeof(pkt.mincost));

            printf("At time t=%f, node %d sends packet to node %d with: ", clocktime, pkt.sourceid, pkt.destid);
            for(int j = 0; (j < MAX_NODES) && (j < 4); j++){
                printf("%d   ", min_arr[j]);
            }
            printf("\n");

            toLayer2(pkt);
        }
    }

}


void rtupdate3( struct RoutePacket *rcvdpkt ) {
    printf("At time t=%f, rtupdate3() called, by a packet received from Sender id: %d\n", clocktime, rcvdpkt->sourceid);

    int lmin_cost; //local minimum cost
    int changed = 0; //unchanged
    int s = rcvdpkt->sourceid; //sender id
    int min_arr[MAX_NODES];
    struct RoutePacket pkt;

    for (int i = 0; i < neighbor3->NodesInNetwork; i++) {
        lmin_cost = rcvdpkt->mincost[i];
        if (dt3.costs[i][s] > (dt3.costs[s][s] + lmin_cost)) {

            //only mark update pkt to be sent if new path
            //is less than 3's current shortest path to i
            int min = INFINITY;
            for (int j = 0; j < neighbor3->NodesInNetwork; j++) {
                if (dt3.costs[i][j] < min) min = dt3.costs[i][j];
            }
            if ((dt3.costs[s][s] + lmin_cost) < min) changed = 1;

            //update 3's path to i via s if shorter than current,
            //even if it may not be the shortest path to i
            dt3.costs[i][s] = dt3.costs[s][s] + lmin_cost;
        }
    }

    if (changed) {
        int lmin;
        for (int i = 0; i < MAX_NODES; i++) {
            lmin = INFINITY;
            for (int j = 0; j < MAX_NODES; j++) {
                if (dt3.costs[i][j] < lmin) {
                    lmin = dt3.costs[i][j];
                }
            }
            min_arr[i] = lmin;
        }

        printf("At time t=%f, node 3 current distance vector: ", clocktime);
        for(int j = 0; (j < MAX_NODES) && (j < 4); j++){
            printf("%d   ", min_arr[j]);
        }
        printf("\n");

        if (TraceLevel > 1) printdt3(3, neighbor3, &dt3);

        for (int i = 0; i < neighbor3->NodesInNetwork; i++) {
            if ((i != 3) && (neighbor3->NodeCosts[i] != INFINITY)) {
                pkt.sourceid = 3;
                pkt.destid = i;

                memcpy(pkt.mincost, min_arr, sizeof(pkt.mincost));

                printf("At time t=%f, node %d sends packet to node %d with: ", clocktime, pkt.sourceid, pkt.destid);
                for(int j = 0; (j < MAX_NODES) && (j < 4); j++){
                    printf("%d   ", min_arr[j]);
                }
                printf("\n");

                toLayer2(pkt);
            }
        }
    } else {
        if (TraceLevel > 1) {
            printf("rtupdate3 106:\n");
            printdt3(3, neighbor3, &dt3);
        }
    }
}


/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure 
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is 
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt3( int MyNodeNumber, struct NeighborCosts *neighbor, 
		struct distance_table *dtptr ) {
    int       i, j;
    int       TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int       NumberOfNeighbors = 0;                     // How many neighbors
    int       Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors 
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt3

