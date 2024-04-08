#include <stdio.h>
#include <stdlib.h>

#include "my402list.h"


int My402ListInit(My402List* list){

    list->num_members = 0;

    My402ListElem* anchor = malloc( sizeof( My402ListElem));

    anchor->next = anchor;
    anchor->prev = anchor;
    anchor->obj = NULL;

    list->anchor = *anchor;

    free(anchor);

    return 1;
}

int  My402ListLength(My402List* list){

    My402ListElem* anchor = &list->anchor;
    int length = 0;

    if( anchor->next == anchor ) list->num_members = 0;
    else{

        My402ListElem* cur = anchor->next;

        while( cur!=anchor ){

            length++;
            cur = cur->next;
        }

        list->num_members = length;
    }

    return list->num_members;
    
} 

int  My402ListEmpty(My402List* list){

    return list->num_members==0;
}

My402ListElem *My402ListNext(My402List* list, My402ListElem* cur){

    if( cur->next == &list->anchor)return NULL;
    else return cur->next;
}

My402ListElem *My402ListPrev(My402List* list, My402ListElem* cur){

    if( cur->prev == &list->anchor ) return NULL;
    else return cur->prev;

}

My402ListElem *My402ListFirst(My402List* list){

    if( My402ListEmpty(list) ) return NULL;
    return list->anchor.next;
}

My402ListElem *My402ListLast(My402List* list){

    if( My402ListEmpty(list) ) return NULL;
    else return list->anchor.prev;
}


int  My402ListAppend(My402List* list, void* obj){
    
    if( My402ListEmpty(list) ){

        My402ListElem* anchor = &(list->anchor);

        My402ListElem* ele = malloc( sizeof( My402ListElem) );
        ele->next = anchor;
        ele->prev = anchor;
        ele->obj = obj;

        anchor->next = ele;
        anchor->prev = ele;

        list->num_members = 1;

        return 1;

    }else{

        My402ListElem* cur = My402ListLast(list);

        My402ListElem* ele = malloc( sizeof( My402ListElem) );
        ele->next = &list->anchor;
        ele->prev = cur;
        ele->obj = obj;

        cur->next = ele;
        list->anchor.prev = ele;

        list->num_members++;

        return 1;

    }
    
}
int  My402ListPrepend(My402List* list, void* obj){

    if( My402ListEmpty(list) ){

        My402ListElem* anchor = &(list->anchor);

        My402ListElem* ele = malloc ( sizeof( My402ListElem) );
        ele->next = anchor;
        ele->prev = anchor;
        ele->obj = obj;

        anchor->next = ele;
        anchor->prev = ele;

        list->num_members = 1;

        return 1;

    }else{

        My402ListElem* cur = My402ListFirst(list);

        My402ListElem* ele =  malloc( sizeof( My402ListElem) );
        ele->prev = &list->anchor;
        ele->next = cur;
        ele->obj = obj;

        cur->prev = ele;
        list->anchor.next = ele;

        list->num_members++;

        return 1;

    }

}
void My402ListUnlink(My402List* list, My402ListElem* ele ){

    if( ele == NULL ){

        printf( "ele is null in unlink");
        exit(1);

    }else if( &list->anchor == ele ){
        
        printf( "unlink anchor in unlink");
        exit(1);

    }else{

        ele->prev->next = ele->next;
        ele->next->prev = ele->prev;

        free(ele);
        My402ListLength(list);

    }
    
}

void My402ListUnlinkAll(My402List* list){

    if( My402ListEmpty(list) ) return;
    else{

        My402ListElem* cur = &list->anchor;
        cur = cur->next;
        
        while( cur!=&list->anchor || !My402ListEmpty(list) ){
            
            My402ListElem* tmp = cur;
            cur = cur->next;
            My402ListUnlink( list, tmp );
        }

        if( My402ListLength(list) != 0 ){

            printf( "list->length is not 0 in unlink all");
            exit(1);

        }

    } 

}

int  My402ListInsertAfter(My402List* list, void* obj, My402ListElem* ele){

    if( ele == NULL ){

        My402ListAppend(list, obj);
        return 1;

    }else if( ele == &list->anchor ){

        printf( "ele is anchor in insert after");
        exit(1);
        return 0;


    }else{

        My402ListElem* new_ele = malloc( sizeof( My402ListElem) );
        new_ele->obj = obj;
        new_ele->prev = ele;
        new_ele->next = ele->next;

        ele->next->prev = new_ele;
        ele->next = new_ele;

        My402ListLength(list);
        return 1;
    }
}
int  My402ListInsertBefore(My402List* list, void* obj, My402ListElem* ele){

     if( ele == NULL ){

        My402ListPrepend(list, obj);
        return 1;

    }else if( ele == &list->anchor ){

        printf( "ele is anchor in insert after");
        exit(1);
        return 0;


    }else{

        My402ListElem* new_ele = malloc( sizeof( My402ListElem) );
        new_ele->obj = obj;
        new_ele->next= ele;
        new_ele->prev = ele->prev;

        ele->prev->next = new_ele;
        ele->prev = new_ele;
        My402ListLength(list);
        return 1;
    }


}


My402ListElem *My402ListFind(My402List* list, void* obj){

    if( My402ListEmpty(list) ) return NULL;
    else{

        My402ListElem *ele=NULL;

        for (ele=My402ListFirst(list); ele != NULL; ele=My402ListNext(list, ele)) {

            if( ele->obj == obj ) return ele;
            else continue;

        }

        return NULL;
    }
}

