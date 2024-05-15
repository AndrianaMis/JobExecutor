#include "../includes/JobExecutor.h"



char message[MAX_LINE_LENGTH/2];                           
volatile sig_atomic_t message_incoming = 0;        //φλαγ που αλλάζει ο handler του σήματος
int concurrency=2;                             //αρχικό concurrency
int jobCounterr=1;

//Η choose_function παρνει ως input την ουρά με τα jobs και ανάλογα με τον ΄τύπου του job διλάγει και εκτελεί την συγκεκριμένη συνάρτηση που πρέπει

void choose_function(queue qu){
    int size =qu->size;

    queue running_jobs=create_q(MAX_LINE_LENGTH);       //Δημιουργεί την νέα ουρά running_jobs με τις τρεχούμενες διεργασίες


    int i=0;
    while(i<size){
        
        if((running_jobs->size)<concurrency){         //FLOW CONTROL
 
           

            int to_execute=qu->front_in;             //Το job που πρέπει να εκτελεστεί είναι το π΄ρωτο στην ουρά 
            char type[MAX_LINE_LENGTH/2];              //Ο τύπος την εντολής (issueJob, setConcurrency etc.)
            char the_job[MAX_LINE_LENGTH/2];          //η actual εντολή  (ls path/to/my/directory ,  2 etc.)

            tripl whole_job=qu->jobs[to_execute];        //πα΄ρινει από την ου΄ρα το job που πρέπει να εκτελεστεί. Παίρνει ολοκληρη την τριπλέτα.
            char name[100];
            strcpy(name, whole_job.job);               //το name είναι το job από την τριπλέτα. Το σπαω σε type και the_job

            char *token=strtok((char*)name, " ");          
            
            if(token!=NULL){
                strcpy(type, token);
                token=strtok(NULL, "\0");
            }
            if(token!=NULL){
                strcpy(the_job, token);
            }        

            /*Ελέγχει το ΄type του job, βγάζει το job από την ουρά και καλεί την κατάλληλη συνάρτηση*/

            if(strcmp(type,"issueJob")==0){         
                strcpy(whole_job.job, the_job);
                del(qu);
                issueJob(whole_job, running_jobs);
                             
                

            }
            else if(strcmp(type, "setConcurrency")==0){
                del(qu);
                setCon(the_job);
                
                
               
            }
           else if(strcmp(type, "stop")==0){
                del(qu);
                stopJob(the_job, qu, running_jobs);
                size--;                                       //σε περίπτωση που αφαιρεί μία εργασία που κανονικά θα ήταν για εκτέλεσει, πρεπει να μειώσει κατά 1 τις επαναλήψεις του loop

            }
            else if(strcmp(type, "poll")==0){
                del(qu);
                poll(the_job, qu, running_jobs);
                sleep(1);
                            
            }
            else if(strcmp(type, "exit")==0){
                printf("TERMINATION\n");                 
                exit_();
            }
            
        }
        else{
            wait(NULL);                      //Αν το πλήθος των running jobs είναι μεγαλύτερο του concurrency, πρέπει να περιμένει μέχρι έστω ένα παιδί να τελείωσει
        }
        i++;

    }
}

//Ο handler για το σήμα που θα στείλει ο commander όταν πρέπει να ειδοποιήσει τον server πως θα του στείλει jobs 
void handler(int signum){
    if(signum==SIGUSR1){
        message_incoming=1;                 //flag 
       // printf("Received SIGUSR1!\n");
        return;
    }

}
int main(){


    pid_t mypid=getpid();                               //το pid του jobExecutorServer

    printf("SERVER starting with pid: %d\n\n", mypid);

    queue qu=create_q(MAX_LINE_LENGTH);                           //Δημιουργώ την ουρά που θα έχει τα waiting jobs
    int thesh=0;
    
    int fd=open("JobExecutorServer.txt", O_WRONLY | O_CREAT, 0644);          //Δημιουργεί ή/και ανοίγει το JobExecutorServer.txt 
    if(fd==-1){
        perror("failed file creation for server\n");
        exit(EXIT_FAILURE);
    }
    else{
        printf("SERVER file created\n");
        write(fd, &mypid, sizeof(mypid));                                //και γράφει το pid του μέσα
    }



    if (signal(SIGUSR1, handler) == SIG_ERR) {
        perror("Failed to register signal handler");
        exit(EXIT_FAILURE);
    }


//όσο δεν έρχεται σήμα από τον commander , ο  server δεν μπορεί να ανοίξει το pipe για διάβασμα, οπότε περιμένει
    while(!message_incoming){usleep(500);}    

    int readff;
    int single_command=0;                          //variable για το αν η εντολή έχει μπροστά το identifier SINGLE δηλαδή είναι η μόνη που πρέπει να διαβστεί
    
    while(single_command==0){                 
       // printf("opening for reading..\n");
       // sleep(1);                                              



        if((readff=open(JOBCOMMANDER_SEND, O_RDONLY  ) )<0){                   //Ανοίγουμε το pipe για διάβασμα 
           perror("Failed to open com_send pipe IN SEVER"); 
        }
        //printf("opened..\n");
       // sleep(2);
    
        int bytes_read = read(readff, message, sizeof(message));
        message[bytes_read] = '\0';
        usleep(100);
        if(strcmp("END_OF_COMMANDS", message)==0){                           //Αν λάβει το μήνυμσ "END OF COMMANDS", σημαίνει πως ο commander έφτασε στο τέλος
            break;                                                                  //του input file και δεν έχει να γράψει κάτι άλλο 
        }
        if (bytes_read == -1) {
            perror("Error reading from FIFO");

                        
        } else if (bytes_read == 0) {
            usleep(1000);                                                      //Αν δεν διαβάσει τίποτα από το pipe , περιμένει και ξαναπροσπαθεί
        } else {
            if (strncmp(message, "SINGLE ", 7) == 0) {              //Ελέγχει αν το message που έλαβε έχει ως identifier το "SINGLE", που σημαίνει ότι:
                single_command=1;                                   //πρέπει να θέση το flag single_command =1 ώστε να κσνει exit από το loop αφού διαβάσει μόνο αυτή την εντολη 
                char* message_2;
                message_2=message + 8;                        //βγάζει από το message το πρόθεμα SINGLE  (7 για WSL , 8 για Linux σχολής)
                strcpy(message, message_2);
                printf("\nmessage now is: %s\n", message);
            }


            tripl *intial=(tripl*)malloc(sizeof(struct tripleta));       
            intial->job=message;               //για να βάλω στηνουρά το job που πήρα, πρέπει να το βάλει ωσ τριπλέτα (η ουρά έχει data tripl ) 
            intial->jobID=NULL;                //αναθετω αρχκα στοιχεία
            intial->qu_pos=-1;

            thesh=add(qu, intial);                  //η add επιστρέφει τη θέση που μπήκε το νέο στοιχείο

            qu->jobs[thesh].qu_pos=thesh;                        //Οπότε στην τριπλέτα ως queueposition είναι η thesh
            int id = snprintf(NULL, 0, "job_%d", jobCounterr);         //Φτιάχνω το job_XX 
            qu->jobs[thesh].jobID = malloc(id + 1);
            sprintf(qu->jobs[thesh].jobID, "job_%d", jobCounterr);
            jobCounterr++;                                            

           printf("<%s , %s , %d>\n", qu->jobs[thesh].job , qu->jobs[thesh].jobID , qu->jobs[thesh].qu_pos);


       

            

        }
        close(readff);
    }
   

    choose_function(qu);           //Καλείται η choose function για να βγάζεικαι να εκτελεί τα jobs
    

    sleep(2);


    //Μετά την εκτέλεση και αφου η ουρά με τα waiting jobs έχει αδειάσει, πρέπει να ανοίξουμε πάλι το pipe για να γράψουμρ το "NOTHING ELSE"
    int write_end=-1;
    if((write_end=open(JOBSERVER_SEND, O_WRONLY))<0){
        perror("Failed opening for ending message");
        exit(EXIT_FAILURE);
    }
    char* end=malloc(sizeof(char*));
    strcpy(end, "NOTHING ELSE");
    sleep(1);
    int bytes= write(write_end, end, strlen(end) );
    if(bytes!=strlen(end)){
        perror("failed writing end message");
    }

    free(end);    




   // print_queue(qu);


    //Διαγραφή του JobExecutorServer.txt αρχείου

    if (unlink("JobExecutorServer.txt") == -1) {
        perror("Failed to delete file");
        exit(EXIT_FAILURE);
    } else {
        printf("\nFile '%s' deleted successfully\n", "JobExecutorServer.txt");
    }
    sleep(1);
    return 0;



}