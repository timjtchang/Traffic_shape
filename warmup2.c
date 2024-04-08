#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/time.h>
#include<limits.h>
#include<math.h>
#include<signal.h>

#include"warmup2.h"

pthread_mutex_t em=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tm=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sm=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

pthread_t pthread;
pthread_t tthread;
pthread_t s1thread;
pthread_t s2thread;
pthread_t cthread;

sigset_t set, oldset;


int num = 20; // number to arrive
FILE* fptr;
int B = 10; // token bucket capacity
int P = 3; // required number of token

double lambda = 1;
double mu = 0.35;
double r = 1.5;

int pkt_time; //us (1/lambda)*1000000
int token_time; //us (1/r)*1000000

My402List* Q1;
My402List* Q2;

int kill_status = 0;
int mode = 0; // 0 Deterministic
            // 1 Trace-driven

struct timeval begin_time;
int bucket_token_num;

int pkt_counter = 0;
int drop_pkt_num = 0;
int pkt_inter_time = 0;

int token_counter = 0;
int drop_token_num = 0;

int Q1_pkt_counter = 0;
int Q1_pkt_time = 0;

int Q2_pkt_counter = 0;
int Q2_pkt_time = 0;

int S1_pkt_counter = 0;
int S1_in_sys_time = 0;

int S2_pkt_counter = 0;
int S2_in_sys_time = 0;

int S1_service_time = 0;
int S2_service_time = 0;

double S1_sqr_in_sys_time = 0;
double S2_sqr_in_sys_time = 0;

int end_time = 0;

int prog_end = 0 ;

int input_line = 1;

int destructList(My402List* list){

    while( My402ListEmpty(list)!=1 ){

        My402ListElem* ele = My402ListFirst(list);
        Obj* obj = (Obj*)ele->obj;

        My402ListUnlink(list,ele);

        free(obj);
    }
    
    My402ListUnlinkAll(list);

    free(list);

    return 1;
}


void err( char* msg ){

    pthread_mutex_lock( &em );
    fprintf( stderr, "[ERROR]%s\n", msg );
    pthread_mutex_unlock( &em );

    exit(1);
}

void cout( char* msg, int op ){

    pthread_mutex_lock( &tm );

    if( op == 0 ){

        fprintf( stderr, "[TEST]%s", msg );
    }else if( op ==1 ){

        fprintf( stderr, "%s", msg );

    }else if( op ==2 ){

        fprintf( stderr, "%s\n", msg );

    }else{

        fprintf( stderr, "[TEST]%s\n", msg );
    }
    
    pthread_mutex_unlock( &tm );

    
}

int check_para( char* str_lambda, char* str_mu, char* str_r, char* str_B, char* str_P, char* str_num ){

    //lambda

    int point_check = 0;
    if( str_lambda != NULL ){

        for( int i=0 ;; i++ ){

            if( str_lambda[i] == '\0' ) break;

            if( point_check > 1 ) err("lambda should be a positive real number");

            if( str_lambda[i] == '.' ){

                point_check++ ;
                continue;
            }

            if( str_lambda[i]<'0' || str_lambda[i]>'9' ) err("lambda should be a positive real number");

        }
        lambda = atof(str_lambda);


        point_check = 0;

    }

    //mu

    if( str_mu != NULL ){

        for( int i=0 ;; i++ ){

            if( str_mu[i] == '\0' ) break;

            if( point_check > 1 ) err("mu should be a positive real number");

            if( str_mu[i] == '.' ){

                point_check++ ;
                continue;
            }

            if( str_mu[i]<'0' || str_mu[i]>'9' ) err("mu should be a positive real number");

        }

        mu = atof(str_mu);

    }

    //r

    if( str_r != NULL ){

        point_check = 0;
        for( int i=0 ;; i++ ){

            if( str_r[i] == '\0' ) break;

            if( point_check > 1 ) err("r should be a positive real number");

            if( str_r[i] == '.' ){

                point_check++ ;
                continue;
            }

            if( str_r[i]<'0' || str_r[i]>'9' ){

                err("r should be a positive real number");
                return 1;
            }

        }
        r = atof(str_r);
    }

    //
    
    //B

    if( str_B != NULL ){

        for( int i=0 ;; i++ ){

            if( str_B[i] == '\0' ) break;

            if( str_B[i]<'0' || str_B[i]>'9' ){

                err("B should be a positive interger");
                return 1;
            }

        }

        long tmp = atol(str_B);

        if( tmp>2147483647 ){

            err("value of B is too large");
        }

        else B = (int)tmp;

    }

    //end B

    //P

    if( str_P != NULL ){

        for( int i=0 ;; i++ ){

            if( str_P[i] == '\0' ) break;

            if( str_P[i]<'0' || str_P[i]>'9' ){

                err("P should be a positive interger");
                return 1;
            }

        }

        long tmp = atol(str_P);

        if( tmp>2147483647 ){

            err("value of P is too large");
        }

        else P = (int)tmp;

    }

    //end P

    //num

    if( str_num!=NULL ){
        for( int i=0 ;; i++ ){

            if( str_num[i] == '\0' ) break;

            if( str_num[i]<'0' || str_num[i]>'9' ){

                err("n should be a positive interger");
                return 1;
            }

        }

        long tmp = atol(str_num);

        if( tmp>2147483647 ){

            err("value of n is too large");
        }

        else num = (int)tmp;
    }

    // end num

    return 0;

}

int getNum(){

    char input[INPUT_SIZE+1];
    int check = 0;

    for( int i=0 ; i<INPUT_SIZE ; i++){

        char ch = fgetc(fptr);

        if( ch == '\n' || ch == ' '){

            if( ch == '\n' ) input_line++;

            check = 1;
            input[i] = '\0';
            break;

        }else if( ch<'0' || ch>'9' ){

            fprintf(stderr,"[ERROR Line:%d] Input invalid\n", input_line);
            exit(1);

        }else{

            input[i] = ch;

        }

    }

    if( input[0] == '\0') return -1;

    long tmp_num=INT_MAX;
    int res = -1;

    if( check == 0 ){

        fprintf(stderr,"[ERROR Line:%d] Input invalid\n", input_line);
        exit(1);
    } 
    else tmp_num = atol(input);

    if( tmp_num>INT_MAX ){

        fprintf(stderr,"[ERROR Line:%d] Input invalid\n", input_line);
        exit(1);

    }else res = tmp_num;

    return res;

}

static
void Usage(int argc, char *argv[]){

    char* str_lambda = NULL;
    char* str_mu = NULL;
    char* str_r = NULL;
    char* str_B = NULL;
    char* str_P = NULL;
    char* str_num = NULL;

    char* tsfile = NULL;


    for( int i=1 ; i<argc-1 ; i++ ){

        if( strcmp( *(argv+i), "-t") == 0 ){
            
            if( i+1>argc-1 ){

                    err("command line syntax error\nusage: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]");

            }

            tsfile = (char*)*(argv+i+1);

            if( tsfile[0] == '-' ){

                err("command line syntax error\nusage: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]");
            }
            i++;
            continue;
            
        }

        if( strcmp( *(argv+i), "-lambda") == 0 ){

            str_lambda = (char*)*(argv+i+1);
            i++;
            continue;
        }

        if( strcmp( *(argv+i), "-mu") == 0 ){

            str_mu = (char*)*(argv+i+1);
            i++;
            continue;
        }

        if( strcmp( *(argv+i), "-r") == 0 ){

            str_r = (char*)*(argv+i+1);
            i++;
            continue;
        }

        if( strcmp( *(argv+i), "-B") == 0 ){

            str_B = (char*)*(argv+i+1);
            i++;
            continue;
        }

        if( strcmp( *(argv+i), "-P") == 0 ){

            str_P = (char*)*(argv+i+1);
            i++;
            continue;
        }

        if( strcmp( *(argv+i), "-n") == 0 ){

            str_num = (char*)*(argv+i+1);
            i++;
            continue;
        }

        err("command line syntax error\nusage: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]");

    }

    //fprintf(stderr, "str_lambda = %s \nstr_mu=%s \nstr_r=%s \nstr_B=%s \nstr_P=%s \nstr_num=%s\n", str_lambda, str_mu, str_r, str_B, str_P, str_num );

    if( check_para( str_lambda, str_mu, str_r, str_B, str_P, str_num )!=0 ){

        err("command line syntax error\nusage: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]");
    }


    if( tsfile != NULL ){

        if( ( fptr = fopen( tsfile, "r")) == NULL ){

            err("can not open file!\n");

        }else{

            mode = 1; 

            while(mode == 1){

                num = getNum();
                if( num!=-1) break;
            }  
    
        }

    }else mode = 0;

    fprintf( stdout, "Emulation Parameters:\n" );

    if( mode == 0 ){

        if( str_lambda == NULL ) fprintf( stdout, "\tlambda = %f\n", lambda);
        else fprintf( stdout, "\tlambda = %s\n", str_lambda);

        if( str_mu == NULL ) fprintf( stdout, "\tmu = %f\n", mu);
        else fprintf( stdout, "\tmu = %s\n", str_mu);

        if( str_r == NULL ) fprintf( stdout, "\tr = 1.5\n");
        else fprintf( stdout, "\tr = %s\n", str_r);

        if( str_B == NULL ) fprintf( stdout, "\tB = %d\n", B);
        else fprintf( stdout, "\tB = %s\n", str_B);

        if( str_P == NULL ) fprintf( stdout, "\tP = %d\n", P);
        else fprintf( stdout, "\tP = %s\n", str_P);

    }else if( mode == 1 ){

        if( str_num == NULL ) fprintf( stdout, "\tnumber to arrive = %d\n", num);
        else fprintf( stdout, "\tnumber to arrive = %s\n", str_num);

        if( str_r == NULL ) fprintf( stdout, "\tr = 1.5\n");
        else fprintf( stdout, "\tr = %s\n", str_r);

        if( str_B == NULL ) fprintf( stdout, "\tB = %d\n", B);
        else fprintf( stdout, "\tB = %s\n", str_B);

        fprintf( stdout, "\ttsfile = %s\n", tsfile);

    }

}

int setTime(){

    gettimeofday(&begin_time, NULL);
    return 0;

}

int getTime(){

    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    int sec = current_time.tv_sec;
    int us = current_time.tv_usec;

    int ssec = begin_time.tv_sec;
    int sus = begin_time.tv_usec;

    int stamp = 0;
    if( ssec != sec ){

        stamp = sec*1000000+us -  ssec*1000000+sus;
       
    }else{

        stamp = us-sus;
    }
    
    
    return stamp;

}

void get_str_time( char* str_time, int stamp, int mode ){

    int size;

    if( mode == 0 ){

        size = 15;
        char zero_time[] = "00000000.000ms";
        strcpy( str_time, zero_time );

    }else{

        int counter = 0;
        int tmp = stamp;

        while( tmp != 0 ){

            tmp = tmp/10;
            counter++;

        }

        if( counter<=4 ) size = 8;
        else size = counter+4;
        

        char zero_time[size];

        for( int i=0 ; i<size ; i++ ) zero_time[i] = '0';

        zero_time[size-1] = '\0';
        zero_time[size-2] = 's';
        zero_time[size-3] = 'm';
        zero_time[size-7] = '.';

        strcpy( str_time, zero_time );
        

    }

    int tmp_stamp = stamp;

    for( int i=size-4 ; i>=0 ; i-- ){

        if(i==size-7) continue;
        else{

            str_time[i] = (char)(tmp_stamp%10+'0');
            tmp_stamp/=10;
        }
    }

    if( mode == 3 ){

        str_time[size-7] = 'm';
        str_time[size-6] = 's';
        str_time[size-5] = '\0';

    }
    
    
}

PKT* obj_constructor(){

    PKT* pkt = malloc(sizeof(PKT));

    pkt->id = malloc(sizeof(int));
    pkt->pkt_arrival_time = malloc(sizeof(int));
    pkt->Q1_arrival_time = malloc(sizeof(int));
    pkt->Q1_departure_time = malloc(sizeof(int));
    pkt->Q2_arrival_time = malloc(sizeof(int));
    pkt->Q2_departure_time = malloc(sizeof(int));
    pkt->service_time = malloc(sizeof(int));
    pkt->token_needed = malloc(sizeof(int));

    return pkt;

}
int obj_destructor( PKT* pkt){

    free(pkt->id);
    free(pkt->pkt_arrival_time);
    free(pkt->Q1_arrival_time);
    free(pkt->Q1_departure_time);
    free(pkt->Q2_arrival_time);
    free(pkt->Q2_departure_time);
    free(pkt->service_time);
    free(pkt->token_needed);

    free(pkt);

    return 0;
}

int enqueue_Obj( My402List* list, Obj* obj ){

    My402ListPrepend(list, obj );
    return 0;

}

int dequeue_Obj( My402List* list){

    My402ListElem* ele =  My402ListLast(list);
    My402ListUnlink(list,ele);

    return 0;

}

void* tail_Obj( My402List* list){

    return My402ListLast(list)->obj;
}

void* front_Obj( My402List* list){

    return My402ListFirst(list)->obj;
}


int getPara( int* service_time ){

    if( mode == 1 ){

        while(1){

            pkt_time = getNum();
            if( pkt_time == -1 ) continue;
            else pkt_time*=1000;

            break;

        }

        while(1){

            P = getNum();
            if( P == -1 ) continue;
            break;

        }

        while(1){

            *service_time = getNum();
            if( *service_time == -1 ) continue;
            else *service_time*=1000;

            break;

        }
        
    }else{

        pkt_time = 1000000/lambda;
        *service_time = 1000000/mu;

        if( pkt_time > 10000000 ) pkt_time = 10000000;
        else{

            pkt_time = round(pkt_time/1000)*1000;
        }

        if( *service_time > 10000000 ) *service_time = 10000000;
        else{

            *service_time = round(*service_time/1000)*1000;
        }

    }

    return 0;
}

void leaveQ1(){

    PKT* pkt;

    char str_timestamp[STR_TIME_LEN];
    char str_intertime[STR_TIME_LEN];

    pkt = (PKT*)tail_Obj(Q1);
    dequeue_Obj(Q1);

    *pkt->Q1_departure_time = getTime();               // time when pkt leave Q1
    bucket_token_num-=*pkt->token_needed;
    get_str_time(str_timestamp, *pkt->Q1_departure_time, 0 );
    get_str_time(str_intertime, *pkt->Q1_departure_time-*pkt->Q1_arrival_time, 1);

    fprintf(stdout, "%s: p%d leaves Q1, time in Q1 = %s, token bucket now has %d token\n", str_timestamp, *pkt->id, str_intertime, bucket_token_num );

    enqueue_Obj(Q2, pkt );
    *pkt->Q2_arrival_time = getTime();
    get_str_time(str_timestamp, *pkt->Q2_arrival_time, 0 );

    fprintf(stdout, "%s: p%d enters Q2\n", str_timestamp, *pkt->id);

    Q1_pkt_counter++;
    Q1_pkt_time+=(*pkt->Q1_departure_time-*pkt->Q1_arrival_time);

    pthread_cond_broadcast(&cv);


}



void* pthread_func(void* arg){

    pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, NULL );
    pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );

    int last_arrival_time = 0;
    int interval_time = 0;

    int pkt_id = 0;

    int counter = 0;

    int service_time = 0;

    char str_timestamp[STR_TIME_LEN];
    char str_intertime[STR_TIME_LEN];

    while( counter<num ){

        getPara( &service_time );

        // Packet Arrive

        interval_time = last_arrival_time+pkt_time;

        int current_time = getTime();

        if( current_time<interval_time ){

            pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );
            pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );

            usleep( interval_time-current_time );

            pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, NULL );
            pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );

        } 

        pthread_mutex_lock(&m);

        counter++;
        PKT* pkt = obj_constructor();
        
        pkt_id++;
        *(pkt->id) = pkt_id;
        *pkt->token_needed = P;
        *(pkt->service_time) = service_time;

        *(pkt->pkt_arrival_time) = getTime();

        pkt_inter_time+=( *(pkt->pkt_arrival_time)-last_arrival_time );

        get_str_time( str_timestamp, *(pkt->pkt_arrival_time), 0);
        get_str_time( str_intertime, *(pkt->pkt_arrival_time)-last_arrival_time, 1);

        pkt_counter++;
        last_arrival_time = pkt_inter_time;

        if( *pkt->token_needed > B ){

            fprintf(stdout, "%s: p%d arrives, needs %d tokens, inter-arrival time = %s, dropped\n", str_timestamp, *(pkt->id), P, str_intertime );
            drop_pkt_num++;
            obj_destructor(pkt);

        }else{

            fprintf(stdout, "%s: p%d arrives, needs %d tokens, inter-arrival time = %s\n", str_timestamp, *(pkt->id), P, str_intertime );
            // push packet into Q1

            int is_Q1_empty = My402ListEmpty(Q1);

            *pkt->Q1_arrival_time = getTime();
            get_str_time( str_timestamp, *pkt->Q1_arrival_time, 0);
            enqueue_Obj(Q1,pkt);
            fprintf(stdout, "%s: p%d enters Q1\n", str_timestamp, *pkt->id);
            
            if( is_Q1_empty == 1 && bucket_token_num>=*pkt->token_needed ){

                leaveQ1();

            }
        }

        pthread_mutex_unlock(&m);

    }

    return NULL;
    
}

void* tthread_func(void* arg){

    pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, NULL );
    pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );

    int counter = 0;
    int token_id = 0;
    int last_arrival_time = 0;

    char str_timestamp[STR_TIME_LEN];



    while( pkt_counter<num || My402ListEmpty(Q1)!=1 ){

        token_time = 1000000/r;

        if( token_time > 10000000 ) token_time = 10000000;
        else{

            token_time = round( token_time/1000 )*1000;
        }

        int current_time = getTime();

        if( current_time<last_arrival_time+token_time ){ 
            
            pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );
            pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );

            usleep(last_arrival_time+token_time-current_time);

            pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, NULL );
            pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );
            
        }

        pthread_mutex_lock(&m);

        token_id++;
        counter++;
        bucket_token_num++;
        token_counter++;

        current_time = getTime();
        last_arrival_time=current_time;
        get_str_time( str_timestamp, current_time, 0);

        if( bucket_token_num > B ){

            bucket_token_num--;
            drop_token_num++;
            fprintf(stdout, "%s: token t%d arrives, dropped\n", str_timestamp, token_id );

        }else{

            fprintf(stdout, "%s: token t%d arrives, token bucket now has %d tokens\n", str_timestamp, token_id, bucket_token_num );

        }
        

        if( My402ListEmpty(Q1)!=1 ){
            
            PKT* pkt = (PKT*)tail_Obj(Q1);

            if( bucket_token_num>= *(pkt->token_needed) ){

                leaveQ1();

            }

        }

        pthread_mutex_unlock(&m);
        
    }
    
    return NULL;
}

void printStatistic(){

    pthread_mutex_lock(&sm);
     
    double avg_in_sys_time = ((double)(S1_in_sys_time+S2_in_sys_time)/1000000)/(double)(S1_pkt_counter+S2_pkt_counter);
    double avg_sqr_in_sys_time = (S1_sqr_in_sys_time+S2_sqr_in_sys_time)/(double)(S1_pkt_counter+S2_pkt_counter);
    double deviation = sqrt( avg_sqr_in_sys_time-avg_in_sys_time*avg_in_sys_time );

    fprintf( stdout, "\nStatistics:\n" );

    if( pkt_counter == 0 ) fprintf( stdout, "\taverage packet inter-arrival time = N/A\n");
    else fprintf( stdout, "\taverage packet inter-arrival time = %.6g\n",  ((double)pkt_inter_time/1000000)/(double)pkt_counter);

    if(S1_pkt_counter+S2_pkt_counter == 0)fprintf( stdout, "\taverage packet service time = N/A\n\n");
    else fprintf( stdout, "\taverage packet service time = %.6g\n\n",  ((double)(S1_service_time+S2_service_time)/1000000)/(double)(S1_pkt_counter+S2_pkt_counter) );

    fprintf( stdout, "\taverage number of packets in Q1 = %.6g\n", (double)Q1_pkt_time/(double)end_time );
    fprintf( stdout, "\taverage number of packets in Q2 = %.6g\n", (double)Q2_pkt_time/(double)end_time );
    fprintf( stdout, "\taverage number of packets at S1 = %.6g\n", (double)S1_service_time/(double)end_time );
    fprintf( stdout, "\taverage number of packets at S2 = %.6g\n\n", (double)S2_service_time/(double)end_time );

    if(S1_pkt_counter+S2_pkt_counter == 0)fprintf( stdout, "\taverage packet service time = N/A\n\n");
    else fprintf( stdout, "\taverage time a packet spent in system = %.6g:\n", ((double)(S1_in_sys_time+S2_in_sys_time)/1000000)/(double)(S1_pkt_counter+S2_pkt_counter)  );
    
    if( S1_pkt_counter+S2_pkt_counter == 0 || avg_sqr_in_sys_time-avg_in_sys_time*avg_in_sys_time<0 ) fprintf( stdout, "\tstandard deviation for time spent in system = N/A\n\n" );
    else fprintf( stdout, "\tstandard deviation for time spent in system = %.6g:\n\n", deviation );

    if( token_counter == 0 )fprintf( stdout, "\ttoken drop probability = N/A\n" );
    else fprintf( stdout, "\ttoken drop probability = %.6g\n", (double)drop_token_num/(double)token_counter );

    if( pkt_counter == 0 ) fprintf( stdout, "\tpacket drop probability = N/A\n");
    else fprintf( stdout, "\tpacket drop probability = %.6g\n", (double)drop_pkt_num/(double)pkt_counter);

    // fprintf(stdout, "Statistic: \npkt_counter = %d \ndrop_pkt_num = %d \npkt_inter_time = %d \ntoken_counter = %d \ndrop_token_num = %d \nQ1_pkt_counter = %d \nQ1_pkt_time = %d \nQ2_pkt_counter = %d\nQ2_pkt_time = %d \nS1_pkt_counter = %d \nS1_in_sys_time = %d  \nS2_pkt_counter = %d \nS2_in_sys_time = %d \nS1_servcie_time = %d \nS2_service_time = %d\n", pkt_counter, drop_pkt_num,pkt_inter_time,token_counter,drop_token_num,Q1_pkt_counter,Q1_pkt_time,Q2_pkt_counter,Q2_pkt_time,S1_pkt_counter,S1_in_sys_time,S2_pkt_counter,S2_in_sys_time,S1_service_time,S2_service_time);
    // fprintf(stdout, "\tdouble S1_sqr_in_sys_time = %f\n", S1_sqr_in_sys_time);
    // fprintf(stdout, "\tdouble S2_sqr_in_sys_time = %f\n", S2_sqr_in_sys_time);
    // printf("\tavg_in_sys_time: %f\n", avg_in_sys_time);
    // printf("\tavg_sqr_in_sys_time: %f\n", avg_sqr_in_sys_time);

    pthread_mutex_unlock(&sm);

}

void* s1thread_func(void* arg){

    char str_timestamp[STR_TIME_LEN];
    char str_intertime[STR_TIME_LEN];

    while(1){

        pthread_mutex_lock( &m );

        while( My402ListEmpty(Q2) == 1 ){

            if( prog_end == 1 ){

                pthread_mutex_unlock(&m);
                return NULL;

            }else{

                pthread_cond_wait( &cv, &m );

            } 

        }

        PKT* pkt = tail_Obj(Q2);
        dequeue_Obj(Q2);

        *pkt->Q2_departure_time = getTime();

        get_str_time(str_timestamp, *pkt->Q2_departure_time, 0 );
        get_str_time( str_intertime, *pkt->Q2_departure_time-*pkt->Q2_arrival_time, 1 ); 

        fprintf(stdout, "%s: p%d leaves Q2, time in Q2 = %s\n", str_timestamp, *pkt->id, str_intertime );
        Q2_pkt_time += (*pkt->Q2_departure_time-*pkt->Q2_arrival_time);

        Q2_pkt_counter++;

        int service_begin_time = getTime();

        pthread_mutex_unlock(&m);

        get_str_time( str_intertime, *pkt->service_time, 3 ); 
        get_str_time(str_timestamp, service_begin_time, 0 );

        fprintf(stdout, "%s: p%d begins service at S1, requesting %s of service\n", str_timestamp, *pkt->id, str_intertime );
  
        usleep( *pkt->service_time );

        char str_in_sys_time[STR_TIME_LEN];

        int current_time = getTime();

        get_str_time( str_timestamp, current_time, 0 );
        get_str_time( str_intertime, current_time-service_begin_time, 1 );
        get_str_time( str_in_sys_time, current_time-*pkt->pkt_arrival_time, 1 );

        fprintf(stdout, "%s: p%d departs from S1, service time = %s, time in system = %s\n", str_timestamp, *pkt->id, str_intertime, str_in_sys_time );

        int in_sys_time = (current_time-*pkt->pkt_arrival_time);
        S1_pkt_counter++;
        S1_in_sys_time += in_sys_time;
        S1_service_time+= current_time-service_begin_time;

        double d_in_sys_time = ((double)in_sys_time)/1000000;
        S1_sqr_in_sys_time+=(d_in_sys_time*d_in_sys_time);

        obj_destructor(pkt);

        //printStatistic( "S1");

    }

}

void* s2thread_func(void* arg){

    char str_timestamp[STR_TIME_LEN];
    char str_intertime[STR_TIME_LEN];

    while(1){

        pthread_mutex_lock( &m );

        while( My402ListEmpty(Q2) == 1 ){

            if( prog_end == 1 ){

                pthread_mutex_unlock(&m);
                return NULL;

            }else{

                pthread_cond_wait( &cv, &m );
            }

        }

        PKT* pkt = tail_Obj(Q2);
        dequeue_Obj(Q2);

        *pkt->Q2_departure_time = getTime();

        get_str_time(str_timestamp, *pkt->Q2_departure_time, 0 );
        get_str_time( str_intertime, *pkt->Q2_departure_time-*pkt->Q2_arrival_time, 1 ); 
        
        fprintf(stdout, "%s: p%d leaves Q2, time in Q2 = %s\n", str_timestamp, *pkt->id, str_intertime );
        Q2_pkt_time += (*pkt->Q2_departure_time-*pkt->Q2_arrival_time);

        Q2_pkt_counter++;

        int service_begin_time = getTime();

        pthread_mutex_unlock(&m);

        get_str_time( str_intertime, *pkt->service_time, 3 ); 
        get_str_time(str_timestamp, service_begin_time, 0 );

        fprintf(stdout, "%s: p%d begins service at S2, requesting %s of service\n", str_timestamp, *pkt->id, str_intertime );
  
        usleep( *pkt->service_time );

        *pkt->Q2_departure_time = service_begin_time;

        char str_in_sys_time[STR_TIME_LEN];

        int current_time = getTime();

        get_str_time( str_timestamp, current_time, 0 );
        get_str_time( str_intertime, current_time-service_begin_time, 1 );
        get_str_time( str_in_sys_time, current_time-*pkt->pkt_arrival_time, 1 );

        fprintf(stdout, "%s: p%d departs from S2, service time = %s, time in system = %s\n", str_timestamp, *pkt->id, str_intertime, str_in_sys_time );

        int in_sys_time = (current_time-*pkt->pkt_arrival_time);

        S2_pkt_counter++;
        S2_in_sys_time += in_sys_time;
        S2_service_time+= (current_time-service_begin_time);

        double d_in_sys_time = ((double)in_sys_time)/1000000;
        S2_sqr_in_sys_time+=(d_in_sys_time*d_in_sys_time);

        obj_destructor(pkt);

        //printStatistic( "S2");

    }

}

void* cthread_func(){

    int sig;
    char str_timestamp[STR_TIME_LEN];

    sigwait(&set,&sig);

    pthread_mutex_lock(&m);

    get_str_time( str_timestamp, getTime(), 0);
    fprintf(stdout, "%s: SIGINT caught\n", str_timestamp );
    fprintf(stderr, "SIGINT caught\n");

    pthread_cancel(pthread);
    pthread_cancel(tthread);

    kill_status = 1;

    pthread_mutex_unlock(&m);

    return NULL;

}

int main( int argc, char* argv[]){


    sigemptyset(&set);
    sigaddset(&set, SIGINT );
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    
    Usage( argc, argv );

    Q1 = malloc( sizeof(My402List) );
    Q2 = malloc( sizeof(My402List) );

    My402ListInit(Q1);
    My402ListInit(Q2);

    fprintf(stdout, "00000000.000ms: emulation begins\n");
    setTime();

    pthread_create(&pthread, 0, pthread_func, NULL );
    pthread_create(&tthread, 0, tthread_func, NULL );
    pthread_create(&s1thread, 0, s1thread_func, NULL );
    pthread_create(&s2thread, 0, s2thread_func, NULL );
    pthread_create(&cthread, 0, cthread_func, NULL );

    pthread_join(pthread, 0);
    pthread_join(tthread, 0);

    
    prog_end = 1;
    

    pthread_cond_broadcast(&cv);
    
    pthread_join(s1thread, 0);
    pthread_join(s2thread, 0);

    destructList(Q1);
    destructList(Q2);

    char str_timestamp[STR_TIME_LEN];

    end_time = getTime();

    get_str_time(str_timestamp, end_time, 0 );
    fprintf(stdout, "%s: emulation ends\n", str_timestamp );

    printStatistic();

    return 0;

}

