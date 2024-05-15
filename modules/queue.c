#include "../includes/queue.h"
#include <string.h>



//Εδώ βρίσκονται οι υλοποιήσεις των queue functions

//Δημιουργεί και επιστρέφει το queue 
queue create_q(unsigned int capacity){
    queue q=(queue)malloc(sizeof(struct Queue));
    q->capacity=capacity;

    q->jobs = (tripl*)malloc(q->capacity * sizeof(tripl));
    q->front_in=q->size=0;
    q->rear_in=capacity-1;
    q->size=0;
    return q;
}

//επιστρέφει 0 αν δεν είναι γεμάτη η ουρά και 1 αν είναι
int full(queue q){
    return (q->size==q->capacity);

}

//προσθέτει στην ουρά την τριπλέτα αν δεν είνια γεμάτη η ουρά. Κάνει allocate memory για το jobID και για το job. 
//Επιστρέφει την θέση που μπήκε η τριπλέτα σε επιτυχία
int add(queue q, tripl *value){
    if(full(q)) return -1;
    (q->size)++;
    q->rear_in=(q->rear_in +1)% q->capacity;
    q->jobs[q->rear_in].job =strdup(value->job);
    if(value->jobID!=NULL){
            q->jobs[q->rear_in].jobID=strdup(value->jobID);

    }
    else{
        q->jobs[q->rear_in].jobID=NULL;
    }

    q->jobs[q->rear_in].qu_pos=value->qu_pos;                               //θέση που μπήκε στην ουρά
  //  printf("added: <%s , %s ,%d>\n", value->job , value->jobID , value->qu_pos);
    return q->rear_in;        

}

//Διαγράφει από την ουρά το front στοιχείο. Επιστρέφει 0 σε επιτυχία
int del(queue q){

    if(q->size==0) return -1;

    q->size--;
    //printf("megethos queue: %d\n me deleted item:%s\n", q->size, q->jobs[q->front_in].job);
    int temp=q->front_in;
    q->front_in=(q->front_in+1)%q->capacity;
    return 0;  //epityxia

}

//Εκτυπώνει την ουρά (για debugging χρήση)
void print_queue(queue q) {
    if (q->size == 0) {
        printf("Queue is empty\n");
        return;
    }

    int index = q->front_in;
    for (int i = 0; i < q->size; i++) {
        printf("<%s , %s ,%d>\n", q->jobs[index].job, q->jobs[index].jobID, q->jobs[index].qu_pos);
        index = (index + 1) % q->capacity;
    }
}

//Επιστρέφει το front στοιχείο της ουράς, ή ένα empty tripl αν δεν υπάρχουν τριπλέτες στην ουρά
tripl front(queue q){
    if(q->size==0) {
        tripl empty_tripl;
        empty_tripl.jobID = NULL;
        empty_tripl.job = NULL;
        empty_tripl.qu_pos = -1;
        return empty_tripl;
    }      
    return q->jobs[q->front_in];
}

//Επιστρέφει το rear στοιχείο της ουράς, ή ένα empty tripl αν δεν υπάρχει τριπλέτα στην ουρά
tripl rear(queue q){
    if(q->size==0){
        tripl empty_tripl;
        empty_tripl.jobID = NULL;
        empty_tripl.job = NULL;
        empty_tripl.qu_pos = -1;
        return empty_tripl;
    }
    return q->jobs[q->rear_in];
}

//Η συνάρτηση αυτή διαγράφει από την ουρά ένα συγκεκριμένο element από την θέση point. Χρησιμοποιείται για την υλοποίηση της stopJob.
void remove_p(queue q, int point){
    if (q == NULL || point < 0 || point >= (q->front_in + q->size)) {
        printf("Invalid queue or point\n");
        return;
    }
    

    //Μετακινούμε τα elements μετά από την θέση που έχει πάρει ως input
    for (int i = (point ); i < (q->size + q->front_in)  ; i++) {

        int current_index = (i) % q->capacity;
      //  printf("the poit is %d , the index is %d, front is %d\n", point, current_index, q->front_in);
        int next_index = (current_index + 1) % q->capacity;
        q->jobs[current_index] = q->jobs[next_index];
        printf(" %d  ->  %d \n", current_index, next_index);
    }

    q->size--;

    if (q->size == 0) {
        q->front_in = 0;  // Αν η ουρά αδειάσει
    } 


}


