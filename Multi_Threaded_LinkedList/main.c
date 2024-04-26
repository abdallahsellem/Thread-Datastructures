
#include <stdio.h>
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <semaphore.h>
struct  NODE {

    struct NODE * next;
    int value;
    pthread_mutex_t lock;
} *HEAD;
 struct  READER_WRITER_LOCK {
    sem_t general_lock;
    sem_t writer_lock;
    int readers ;
} ;
struct ThreadArgs {
    struct READER_WRITER_LOCK lock_args;
    int val;

};
void init_reader_writer_lock(struct READER_WRITER_LOCK * rw)
{
    sem_init(&rw->general_lock,0,1) ;
    sem_init(&rw->writer_lock,0,1) ;
    rw->readers=0 ;
}
struct ThreadArgs init_thread_args(struct READER_WRITER_LOCK * rw,int val)
{
    struct ThreadArgs args ;
    args.lock_args=*rw ;
    args.val=val ;
    return args ;
}
void acquire_reader_lock( struct READER_WRITER_LOCK * rw)
{
    sem_wait(&rw->general_lock);
    rw->readers++ ;
    if(rw->readers==1)
    {
        sem_wait(&rw->writer_lock);
    }
    sem_post(&rw->general_lock);

}
void release_reader_lock( struct READER_WRITER_LOCK * rw)
{
    sem_wait(&rw->general_lock);
    rw->readers-- ;
    if(rw->readers==0)
    {
        sem_post(&rw->writer_lock);
    }
    sem_post(&rw->general_lock);

}
void acquire_writer_lock( struct READER_WRITER_LOCK * rw)
{
    sem_wait(&rw->writer_lock);

}
void release_writer_lock( struct READER_WRITER_LOCK * rw)
{
    sem_post(&rw->writer_lock);

}
struct NODE * initiate_node()
{
    struct NODE* node  =(struct NODE *)malloc(sizeof(struct NODE));
    if(node==NULL)
        return NULL ;

    node->next = NULL;
    node->value = 0;
    pthread_mutex_init(&node->lock, NULL);
    return node ;
}

void* insert_node(void* arg) {
    struct ThreadArgs*  args = (struct ThreadArgs *)arg ;
    struct READER_WRITER_LOCK lock = args->lock_args ;
    int val =args->val ;
    acquire_writer_lock(&lock) ;

    struct NODE* node = initiate_node();

    if (node == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&HEAD->lock) ;
    struct NODE * currNode =HEAD ;
    struct NODE * prevNode =NULL ;

    while(currNode->next!=NULL)
    {
        if(currNode!=HEAD)
        pthread_mutex_lock(&currNode->lock) ;

        prevNode=currNode ;
        currNode=currNode->next ;

        if(prevNode!=NULL)
            pthread_mutex_unlock(&prevNode->lock) ;
    }
    node->next=NULL ;
    node->value=val ;
    currNode->next=node ;
    pthread_mutex_unlock(&currNode->lock) ;
    release_writer_lock(&lock) ;

}
void* traverse_nodes(void* arg) {
    struct ThreadArgs*  args = (struct ThreadArgs *)arg ;
    struct READER_WRITER_LOCK lock = args->lock_args ;
    acquire_reader_lock(&lock) ;
    struct NODE * currNode =HEAD ;
    printf("this is spartaaa 2 \n");

    if(currNode==NULL)
        printf("List is Empty \n");
    while(currNode!=NULL)
    {
        printf("%d \n",currNode->value) ;
        currNode=currNode->next ;
    }
    release_reader_lock(&lock) ;
}

int main(int argc, char *argv[]) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    HEAD=initiate_node();
    pthread_t p1,p2,p3 ;
    struct READER_WRITER_LOCK* readerWriterLock1 =(struct READER_WRITER_LOCK *)malloc(sizeof(struct READER_WRITER_LOCK)) ;
    init_reader_writer_lock(readerWriterLock1) ;


    struct ThreadArgs arg1=init_thread_args(readerWriterLock1,5);

    pthread_create(&p1,NULL,insert_node,    (void *)&arg1);

    struct ThreadArgs arg2=init_thread_args(readerWriterLock1,4);

    pthread_create(&p1,NULL,insert_node,(void*)&arg2);
    struct ThreadArgs arg3=init_thread_args(readerWriterLock1,2);

    pthread_create(&p2,NULL,insert_node,(void*)&arg3);
    struct ThreadArgs arg4=init_thread_args(readerWriterLock1,7);
    pthread_create(&p3,NULL,traverse_nodes,(void*)&arg4);
//    arg1=init_thread_args(readerWriterLock1,30);
//    pthread_create(&p1,NULL,insert_node,(void*)&arg1);


    clock_gettime(CLOCK_MONOTONIC, &end);
    long long int elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
    printf("Program elapsed time: %lld nanoseconds\n",elapsed_ns);

    return 0;
}
