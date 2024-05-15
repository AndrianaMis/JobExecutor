#include <stdio.h>
#include <stdlib.h>

struct tripleta{
    char* jobID;                       //job_XX
    char *job;                          //actual job
    int qu_pos;                         //θέση στην ουρά

};
typedef struct tripleta tripl;


struct Queue{
    int front_in , rear_in,size;
    tripl* jobs;                                       //array apo tripletes
    unsigned int capacity;                        //Η ουρά έχει περιορισμό μεγέθους :(
    
};
typedef struct Queue* queue;

queue create_q(unsigned int capacity);
int add(queue q, tripl *);                   //επιστεφει θέση αν είναι επιτυχές και -1 αν δεν είναι
int del(queue q);                                  //επιστεφει 0 αν είναι επιτυχές και -1 αν δεν είναι
tripl front(queue q );                         
tripl rear(queue q);                    
int full(queue q);                                 //0 if it is not full 1 if it is 
void print_queue(queue q);
void remove_p(queue, int);               //διαγράφει από συγκεκριμένη θέση

