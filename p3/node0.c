#include <stdio.h>
#include <string.h>
#include "project3.h"

extern int TraceLevel;
extern float clocktime;

struct distance_table
{
    int costs[MAX_NODES][MAX_NODES];
};

void printdt0(int MyNodeNumber, struct NeighborCosts *neighbor,
              struct distance_table *dtptr);
void sendPkt(int *min_arr[MAX_NODES]);
struct distance_table dt0;
struct NeighborCosts *neighbor0;

/* students to write the following two routines, and maybe some others */

//TODO: Make sure there are no loops (revisted nodes) in shortest path

void rtinit0() {
    if (TraceLevel > 1) printf("rinit0: %f\n", clocktime);

    struct RoutePacket pkt;

    neighbor0 = getNeighborCosts(0);

    for (int i = 0; i < MAX_NODES; i++) {
        for (int j = 0; j < MAX_NODES; j++) {
            if (i == j) dt0.costs[i][j] = neighbor0->NodeCosts[j];
            else dt0.costs[i][j] = INFINITY;
        }
    }

    int min_arr[MAX_NODES];
    int lmin;

    for (int i = 0; i < MAX_NODES; i++) {
        lmin = INFINITY;
        for (int j = 0; j < MAX_NODES; j++) {
            if (dt0.costs[i][j] < lmin) {
                lmin = dt0.costs[i][j];
            }
        }
        min_arr[i] = lmin;
    }
    
    if (TraceLevel > 1) printdt0(0, neighbor0, &dt0);

    for (int i = 0; i < neighbor0->NodesInNetwork; i++) {
        if ((i != 0) && (neighbor0->NodeCosts[i] != INFINITY)) {
            pkt.sourceid = 0;
            pkt.destid = i;

            memcpy(pkt.mincost, min_arr, sizeof(pkt.mincost));
            if (TraceLevel > 1) printf("rinit0: sending pkt to node %d @ t: %f\n", i, clocktime);

            toLayer2(pkt);
        }
    }
}

void rtupdate0(struct RoutePacket *rcvdpkt) {
    if (TraceLevel > 1) printf("rtupdate0: %f\n", clocktime);

    int lmin_cost; //local minimum cost
    int changed = 1; //unchanged
    int s = rcvdpkt->sourceid; //sender id
    int min_arr[MAX_NODES];
    struct RoutePacket pkt;

    for (int i = 0; i < neighbor0->NodesInNetwork; i++) {
        lmin_cost = rcvdpkt->mincost[i];
        if (dt0.costs[i][s] > (dt0.costs[s][s] + lmin_cost)) {
            dt0.costs[i][s] = dt0.costs[s][s] + lmin_cost;
            changed = 0; //changed = true
        }
    }

    if (changed) {
        int lmin;
        for (int i = 0; i < MAX_NODES; i++) {
            lmin = INFINITY;
            for (int j = 0; j < MAX_NODES; j++) {
                if (dt0.costs[i][j] < lmin) {
                    lmin = dt0.costs[i][j];
                }
            }
            min_arr[i] = lmin;
        }
    }

    if (TraceLevel > 1) printdt0(0, neighbor0, &dt0);

    // sendPkt(&min_arr);

    for (int i = 0; i < neighbor0->NodesInNetwork; i++) {
        if ((i != 0) && (neighbor0->NodeCosts[i] != INFINITY)) {
            pkt.sourceid = 0;
            pkt.destid = i;

            memcpy(pkt.mincost, min_arr, sizeof(pkt.mincost));
            if (TraceLevel > 1) printf("rtupdate0: sending pkt to node %d @ t: %f\n", i, clocktime);

            toLayer2(pkt);
        } else {
            printf("rtupdate0 106:\n");
            printdt0(0, neighbor0, &dt0);
        }
    }

}

// void sendPkt(int *min_arr[MAX_NODES]) {
//     struct RoutePacket pkt;
//     for (int i = 0; i < neighbor0->NodesInNetwork; i++) {
//         if ((i != 0) && (neighbor0->NodeCosts[i] != INFINITY)) {
//             pkt.sourceid = 0;
//             pkt.destid = i;

//             memcpy(pkt.mincost, min_arr, sizeof(pkt.mincost));
//             if (TraceLevel > 1) printf("rtupdate0: sending pkt to node %d @ t: %f\n", i, clocktime);

//             toLayer2(pkt);
//         } else {
//             printf("rtupdate0 106:\n");
//             printdt0(0, neighbor0, &dt0);
//         }
//     }
// }

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
void printdt0(int MyNodeNumber, struct NeighborCosts *neighbor,
              struct distance_table *dtptr) {
    int i, j;
    int TotalNodes = neighbor->NodesInNetwork; // Total nodes in network
    int NumberOfNeighbors = 0;                 // How many neighbors
    int Neighbors[MAX_NODES];                  // Who are the neighbors

    // Determine our neighbors
    for (i = 0; i < TotalNodes; i++) {
        if ((neighbor->NodeCosts[i] != INFINITY) && i != MyNodeNumber) {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber);
    for (i = 0; i < NumberOfNeighbors; i++)
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for (i = 0; i < TotalNodes; i++) {
        if (i != MyNodeNumber) {
            printf("dest %d|", i);
            for (j = 0; j < NumberOfNeighbors; j++) {
                printf("  %4d", dtptr->costs[i][Neighbors[j]]);
            }
            printf("\n");
        }
    }
    printf("\n");
} // End of printdt0
