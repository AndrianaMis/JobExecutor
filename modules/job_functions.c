#include "../includes/JobExecutor.h"

//Εδώ βρίσκονται όλα τα functions-εντολές που εκτελόυν τα jobs 


//exit_() :Τερματίζεται η λειτουργία του jobExecutorServer. Ο jobExecutorServer πριν
//τερματίσει επιστρέφει στον jobCommander το μήνυμα.
// Ανοίγει το pipe για να γράψει - επιστρέψει στον jobCommander το termination message "jobExecutorServer terminated." , 
//ιαγράφει το file JobExecutorServer.txt και μετά κάνει exit
void exit_(){
    int writedf=open(JOBSERVER_SEND, O_WRONLY);
    if(writedf<0){
        perror("ERROR opening pipe for exiting");
        exit(EXIT_FAILURE);

    }
    char termination[100];
    strcpy(termination, "jobExecutorServer terminated.");
    sleep(1);
    write(writedf, termination, strlen(termination));
    fflush(stdout);
    close(writedf);
    if (unlink("JobExecutorServer.txt") == -1) {
        perror("Failed to delete file");
        exit(EXIT_FAILURE);
    } else {
        printf("File '%s' deleted successfully\n", "JobExecutorServer.txt");
    }
    sleep(1);
    exit(EXIT_SUCCESS);
}


/*poll [running,queued]: 
    ● poll running : Για κάθε εργασία που είναι υπό εκτέλεση (running) αυτή τη χρονική
στιγμή επιστρέφει την τριπλέτα <jobID,job,queuePosition> την οποία o
jobCommander εκτυπώνει στην κονσόλα.
    ● poll queued : για κάθε εργασία που είναι σε κατάσταση αναμονής (queued),
επιστρέφει την τριπλέτα <jobID,job,queuePosition> την οποία o jobCommander
εκτυπώνει στην κονσόλα.

.Αρχικά ελέγχει αν το r_q που πήρε ωσ input είναι running ή queued. 
Αν είναι running , ανοίγει το pipe, γράφει τον τίτλο
"RUNNING QUEUE" , αν υπάρχουν τρεχόμενα jobs γράφει την τριπλέτα τους στο pipe. Αν δεν υπάρχουν, γράφει "no running jobs" 
Αντίστοιχα αν είνια queued, γράφει στο pipe, "WAITING QUEUE" και τις τριπλέτες όλων των εργασιών στο qu*/

void poll(char* r_q, queue q, queue running){
    if(strcmp(r_q, "running")==0){
        
        int writef=open(JOBSERVER_SEND, O_WRONLY);
        if(writef<0){
            perror("failed pipe openning");
            exit(EXIT_FAILURE);
        }
        char title[100];
        strcpy(title, "RUNNING QUEUE:");
        sleep(1);
        write(writef, title, strlen(title));
        if(running->size>0){                                 //υπάρχουν running jobs
            for(int i=running->front_in; i<(running->front_in + running->size); i++){        //looop through των εργασιων 
                char triplet[100];
                snprintf(triplet, sizeof(triplet), "<%s,%s,%d>", running->jobs[i].job , running->jobs[i].jobID, running->jobs[i].qu_pos);

                int writef=open(JOBSERVER_SEND, O_WRONLY);
                
                sleep(1);
                write(writef, triplet, strlen(triplet));
                fflush(stdout);

            }
        }
        else{
            char respon[100];                                  //το running_jobs queue είναι άδειο
            strcpy(respon, "No running jobs");
            sleep(1);
            write(writef, respon, strlen(respon));
        }
        close(writef);



    }
    else if(strcmp(r_q, "queued")==0){
        int writef=open(JOBSERVER_SEND, O_WRONLY);
        if(writef<0){
            perror("failed pipe openning");
            exit(EXIT_FAILURE);
        }
        char title[100];
        strcpy(title, "WAITING QUEUE:");
        sleep(1);
        write(writef, title, strlen(title));

        if(q->size>0){                                             //υπάρχουν waiting jobs
 
            for(int i=q->front_in; i<(q->front_in + q->size); i++){            //loop για να γράψει στο pipe την τριπλέτα 
                char triplet[100];
                snprintf(triplet, sizeof(triplet), "<%s,%s,%d>", q->jobs[i].job , q->jobs[i].jobID, q->jobs[i].qu_pos);

                int writef=open(JOBSERVER_SEND, O_WRONLY);
                
                sleep(1);
                write(writef, triplet, strlen(triplet));
                fflush(stdout);

            }
        }
        else{                                                 //δεν υπάρχουν waiting jobs
            char respon[100];
            strcpy(respon, "No queued jobs");
            sleep(1);
            write(writef, respon, strlen(respon));
        
        }
        close(writef);


    }
    else{                                                  //το job μετά το poll ... δεν είναι ούτε "running" ουτε "queued"
        int writef=open(JOBSERVER_SEND, O_WRONLY);
        if(writef<0){
            perror("failed pipe openning");
            exit(EXIT_FAILURE);
        }
        char title[100]; 
        strcpy(title, "WRONG POLL COMMAND");
        sleep(1);
        write(writef, title, strlen(title));                   //Οπότε γράφουμε στο pipe "WRONG POLL COMMAND"
        fflush(stdout);
        close(writef);

    }
}



/* stop <jobID>: Τερματίζεται η εκτέλεση της εργασίας με το συγκεκριμένο αναγνωριστικό
(job_XX). Σε περίπτωση που δεν εκτελείται, αλλά είναι υποψήφια για μελλοντική εκτέλεση,
αφαιρείται από την ουρά των υπό εκτέλεση εργασιών.

Αρχικά με ένα for loop ψάχνει αν το job με το jobID είνια στην ουρά q με τα waiting jobs. Αφού η ου΄ρα έχει data tripl , ελέγχει αν το μέλος
jobID της τριπλέτασ της κάθε εργασίας ταυτίζεται με το jobID που έχει πάρει η stopJob σαν είσοδο.
    Αν το βρει, καλεί την remove_p (βλέπε queue.c) 
για να βγάλει την εργασία με το jobID από την ου΄ρα q. Επίσης ανοίγει το pipe για γράψιμο και επιστρέφει στον jobCommander το "job_XX removed" 
    Αν δεν το βρει, συνεχίζει το ψάξιμο τώρα στην ουρά running, αν το βρει καλεί την remove_p και επιστρέφει στον commander "job_XX terminated"
Αν δεν βρει την εργασία με job_id jobID, δεν επιστρέφει κάτι στον commander, αλλά εκτυπώνει στο terminal "job_XX NOT FOUND IN EITHER OF THE QUEUES"
*/
void stopJob(char* jobID ,queue q, queue running){
   // printf("jobid is: %s \n", jobID);
    int writef;
    char response[100];
    int found=0;                       //flag
  
    //ψαχνουμe αν είναι ετοιμη για εκ΄τελση
    for(int i = q->front_in; i <(q->front_in +  q->size); i++) {              //δεν βάζω int i=0 γιατί το q->front_in δεν είναι 0, έχουν ιαγραφεί εργσίες
        //printf("comparing %s    -    %s\n", q->jobs[i].jobID, jobID);
        if(strcmp(q->jobs[i].jobID, jobID)==0){
            found=1;
            remove_p(q, q->jobs[i].qu_pos);
                printf("\nqu:");
                print_queue(q);
                printf("\n\n");
            
           // printf("\nqu szize: %d\n", q->size);

          
            if((writef=open(JOBSERVER_SEND, O_WRONLY))<0){
                perror("ERROR IN PIPE OPENING FOR STOPJOB");
                exit(EXIT_FAILURE);
            
            }
            snprintf(response, sizeof(response), "%s removed\n", jobID);
            write(writef, response, strlen(response));
            //printf("wrote '%s'\n", response);
            fflush(stdout);
            close(writef);
            return;
        }
    } 
    if(!found){                                                           //δεν βρέθηκε η εργασία με jobID μέσα στην ουρα με τα waiting jobs
        for(int i = 0; i < running->size; i++) {

            if(strcmp(running->jobs[i].jobID, jobID)==0){
               // printf("thesh sth running %d\n", running->jobs[i].qu_pos);
               found=1;                                                     //flag
                remove_p(running, running->jobs[i].qu_pos);
                    
                if((writef=open(JOBSERVER_SEND, O_WRONLY))<0){
                    perror("ERROR IN PIPE OPENING FOR STOPJOB");
                    exit(EXIT_FAILURE);
                
                }
                snprintf(response, sizeof(response), "%s terminated\n", jobID);
                write(writef, response, strlen(response));
                fflush(stdout);
                close(writef);
                return;


            }
        }
    }
    if(!found){                                                        //Δεν βρέθηκε σε καμία ουρά
        printf("%s NOT FOUND IN EITHER OF THE QUEUES\n", jobID);
        return;

    }



}



/*setConcurrency <N>: Η παράμετρος αυτή θέτει το βαθμό παραλληλίας, δηλαδή το
μέγιστο αριθμό ενεργών εργασιών που μπορεί να εκτελεί ανά πάσα χρονική στιγμή η
εφαρμογή (δεδομένου ότι υπάρχουν διαθέσιμες). H προκαθορισμένη τιμή είναι 1. H εντολή
μπορεί να αποσταλεί και κατά τη διάρκεια εκτέλεσης εργασιών και θα αλλάξει την
συμπεριφορά του εξυπηρέτη από την λήψη της και μετά. Δεν χρειάζεται να επιστραφεί κάτι
στον jobCommander
Κάνει int το string job και θέτει την extern μεταβλητή concurrency στην τιμή που δόθηκε.
Επίσης εκτυπώνει στο terminal ότι το concurrency άλλαξε.*/
void setCon(char* job){
    int con=atoi(job);
    concurrency=con;
    printf("\nSetting concurrency to %d\n\n", con);
   // printf("concurrency=%d\n", concurrency);
    return;

}


/* issueJob <job>: Μέσω αυτής της εντολής εισάγονται εργασίες στο σύστημα που πρόκειται
να εκτελεστούν. Το job είναι μια συνηθισμένη γραμμή εντολών Unix. H τριπλέτα <jobID,job,queuePosition>
επιστρέφεται στον jobCommander ό οποίος την εκτυπώνει στην κονσόλα. 
Αρχικά προσθέτει στην λίστα με τα running jobs την τριπλέτα που θα γινει execute, 
δημιουργεί ένα copy της εργασίας (η οποία έχει τη μορφη cat myfiletxt , touch example.txt etc. χωρίς το πρόθεμα issueJob) και το κάνει split,
μετράει τα args (arg_count) ώστε να κάνει  memory allocation. Μετά χωρίζει σε tokens το job και κάνει fork() και exec*() την εργασία. Ο jobExecutorServer
περιμένει μέχρι να τελείωσει το παιδί το execution. Τέλος ανοίγει το pipe, γράφει την τριπέτα , κλείνει το pipe και απελευθερώνει allocated memory
*/
void issueJob(tripl execute, queue running){
    add(running, &execute);



    char* job_copy=strdup(execute.job);
    if (job_copy == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    int arg_count = 0;
    char *token = strtok(job_copy, " ");
    while (token != NULL) {
        arg_count++;
        token = strtok(NULL, " ");
    }

    char **args = (char **)malloc((arg_count + 1) * sizeof(char *));          //memory allocation για τα args
    if (args == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    free(job_copy);
    job_copy=strdup(execute.job);
    if (job_copy == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    token = strtok(job_copy, " ");                      //χωρίζει την εργασία και αναθέτει στο araay args την κάθε "λέξη" 
    for (int i = 0; i < arg_count; i++) {
        args[i] = strdup(token);
        if (args[i] == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, " ");
            
    }
    args[arg_count] = NULL;  
    
    int p=fork();                            //κάνει fork()
    if(p==-1){
        perror("Error forking for issueJob");
        exit(EXIT_FAILURE);

    }
    else if(p==0){                            //child 
       // printf("to execvp: %s\n", args[0]);
        if (execvp(args[0], args) == -1) {              //execution 
            perror("execvp");
        }

    }
    else{                                           //parent περιμένει
        int status;
        if (waitpid(p, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
     
    }
       char triplt[100];
    
       // printf("Executing '%s'\n", execute.job);
        snprintf(triplt, sizeof(triplt), "<%s,%s,%d>", execute.job , execute.jobID, execute.qu_pos);
        int writeff;
        if((writeff=open(JOBSERVER_SEND, O_WRONLY ))<0){
            perror("error in server opening");
            return ;
        }            
        sleep(1);

        if(write(writeff, triplt, strlen(triplt) )!= strlen(triplt)){
            perror("write");
            return ;
        }      
        fflush(stdout);
        close(writeff);


        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
        free(args);
        free(job_copy);
        del(running);
   


}