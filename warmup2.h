
#ifndef _WARMUP2_H_
#define _WARMUP2_H_

#include "my402list.h"

#define INPUT_SIZE 1024
#define STR_TIME_LEN 15

typedef struct Packet {

    int* id;
    int* token_needed;
    int* service_time;
    int* pkt_arrival_time;
    int* Q1_arrival_time;
    int* Q1_departure_time;
    int* Q2_arrival_time;
    int* Q2_departure_time;

}PKT,Obj;


extern int freeObj( Obj* );
extern int createObj( Obj*, char* );
extern int addObj( My402List*, Obj* );

extern int enqueue_Obj( My402List*, Obj* );
extern int dequeue_Obj( My402List* );
extern void* tail_Obj( My402List* );
extern void* front_Obj( My402List* );

extern PKT* obj_constructor( );
extern int obj_destructor( PKT* );

#endif /*_WARMUP2_H_*/