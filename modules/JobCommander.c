#include "../includes/JobExecutor.h"





/*H συνάρτηση αυτή πα΄ρινει ως input μια μόνο γραμμή του input file κια το pid του server, σπάει το line στο πρόθεμα ./jobCommander και στο 
actual job που πρέπει να γίνει execute*/
void process_line(const char* line, int server_pid){
    char command[MAX_LINE_LENGTH/2];
    char jc[MAX_LINE_LENGTH/2];
    char* token = strtok((char*)line, " ");
    if (token != NULL) {
        strcpy(jc, token); 
        token = strtok(NULL, "\0");
    }
    if (token != NULL) {

        strcpy(command, token); 

    }

    //fork  commander child για να γραψει 

    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child 
        singal_and_write(command, server_pid);
        exit(EXIT_SUCCESS); //το παιδί τελέιωσε, έστειλε το σήμα και έγραψε
    }
    else{
        int status;                          //ο commander πρεπεί να περιμένει το κάθε παιδί να τελείωσει πριν προχωρήσει
        waitpid(pid, &status, 0);
    }

}
/*Η συνάρτηση αυτή ανοίγει το αρχείο με τα commands,  πα΄ιρνει ένα -ένα τα lines-commands
 και να κάνει fork έναν jobCommander που θα στέλνει την εντολή στον server. 
*/
void file_open(int server_pid){
    int input=open("file.txt", O_RDONLY);            //Ανοίγουμε το input file gγια να διαβάσουμε τα commands

    if(input==-1){
        return;
    }
    char buffer[MAX_LINE_LENGTH];
    char line[MAX_LINE_LENGTH];

    int bytes;

    while ((bytes=read(input, buffer, MAX_LINE_LENGTH)) > 0) {         //Οσο υπάρχει κάτι για διάβασμα, διαβάζω ένα buffer που έχει μεγάλο μέγεθος
        int start=0;

        for(int i=0; i<bytes; i++){                        //Από το buffer που διάβασε, το σπάει σε γραμμές (τελείωνουν με \n)
            if(buffer[i]=='\n'){
                buffer[i]='\0';
                strncpy(line, buffer+start, i-start+1);              //αντιγραφή της γραμμής από το μπαφφερ που διαβαστηκε

                process_line(line, server_pid);
                start=i+1;

                memset(line, 0, sizeof(line));                   //καθαρίζω το buffer line γιατί μπερδευόντουσαν τα γράμματα

            }
               

        }
        //το τελευταίο μάλλον δεν τελείωνει με \n
        if (start < bytes) {
            strncpy(line, buffer + start, bytes - start);
            process_line(line, server_pid);
        }
      
            
        
    }
    if (bytes == -1) {
        perror("Error reading from input file");
        exit(EXIT_FAILURE);
    }
    close(input);
    sleep(1);
    printf("END OF INPUT FILE \n");

    //Όταν δεν υπάρχει άλλη εντολή στο input file, ο jobCommander θα στείλει μέσω pipe το μήνυμα "END OF COMMANDS"
    //Ώστε ο server να ξέρει πως τελείωσε 
    char end[MAX_LINE_LENGTH];                    
    strcpy(end, "./jobCommander END_OF_COMMANDS");
    process_line(end, server_pid );
   // exit(EXIT_SUCCESS);

}

/*Την συνάρτηση αυτλη την χρησιμοποέι κάθε παιδί του commander για να στείλει σημα και να γράψει στο pipe */

void singal_and_write(char* command, int server_pid){
                
    int writedf=-1;


    kill(server_pid, SIGUSR1);                      // ΣΤΕΛΝΟΥΜΕ ΣΗΜΑ 
    usleep(500);

    if((writedf=open(JOBCOMMANDER_SEND, O_WRONLY | O_NONBLOCK ))<0){
        perror("Failed to open com_send pipe");
        return ;
    }



    if((write(writedf, command, strlen(command) ))!= strlen(command)){
        perror("write");
        return;
    }
    //printf("I %d, wrote %s\n", getpid(), command);




   fflush(stdout);
    close(writedf);
    
    return;
    
    


}



int main(int argc, char* argv[]){

//Δημιουργία των pipes :
// JOBCOMMANDER_SEND path για την αποστολή των jobs στον JobExecutorServer
//JOBSERVER_SEND path για την αποστολή όσων μηνυμάτων απαιτείται από τον JobExecutorServer

    if(mkfifo(JOBCOMMANDER_SEND, 0666)<0) perror("can't create fifo com");
   if(mkfifo(JOBSERVER_SEND, 0666)<0) perror("can't create fifo serv");

   int has_line_arguments=0;               //variable - flag που δείχνει αν ο jobCommander ΄΄εχει καλεστεί με line arguments (multijob.sh)
    char line_com[100] ;                   //για το line argument 
    char single[150];                      //για το line argument μαζί με το identifier "SINGLE", του οποιου η χρησιμότητα είναι για να μπορέσει
                                            //ο JobExecutorServer να κάνει exit από το while(1) loop, διαβάζοντας μόνο 1 μήνυμα από το pipe

//Έλεγχος αν έχει καλεστεί με line arguments (multijob.sh)
if(argc>1){
    for(int i = 1; i < argc; i++){
        strcat(line_com, argv[i]);         //cat τα arguments με κενά 
        strcat(line_com, " ");
    }

    line_com[strlen(line_com) - 1] = '\0';
    has_line_arguments=1;            //flag
 

 //ΒΑΖΩ ΜΠΡΟΣΤΑ ΕΝΑ ΑΝΑΓΝΩΡΙΣΤΙΚΌ ΓΙΑ ΝΑ ΜΠΟΡΕΣΕΙ Ο ΣΕΡΒΕΡ ΝΑ ΚΑΝΕΙ EXIT ΑΠΟ ΤΟ ΛΟΟΠ
    strcpy(single, "SINGLE ");        
    strcat(single, line_com);

    printf("Submitting task from command line: %s\n", single);

}
  
    int server_pid;


//Έλεγχος αν υπάρχει το JobExecutorServer.txt που έχει το pid του jobExecutorServer (αυτό σημαίνει πως ο server είναι ενεργοποιημε΄νος)
    if(access("JobExecutorServer.txt", F_OK)!=-1){
        int fd=open("JobExecutorServer.txt", O_RDONLY);                       ///ανοίγει το αρχείο για να διαβάσει το PID του Server
        if(fd==-1){
            perror("cannot open the server file");
            exit(EXIT_FAILURE);
        }
        ssize_t bytes_read = read(fd, &server_pid, 9 );        //low level διαβασμα 

        if(bytes_read==-1){
            perror("cannot read the pid of server");
            close(fd);
            exit(EXIT_FAILURE);
        }

       close(fd);
        printf("PID of server: %d\n", server_pid);
        if(has_line_arguments){                              //τσεκάρει το flag ώστε αν has_line_arguments==1 θα στείλει ο ίδιος ο initial commander
            singal_and_write(single, server_pid);              //μόνο το job που πήρε ως line argument
        }
        else{                                                   //αλλιώς διαβάζει ολόκληρο το input file line by line , και στέλνει την κάθε γραμμή
            file_open(server_pid);                              //στον JobExecutorServer

        }
    }

//Αν το JobExecutorServer.txt file δεν υπάρχει δηλαδή ο server δεν είναι ενεργ΄ός, τότε ο Commander πρέπει να τον ενεργοποίησει με fork() 
//και exec*()

    else{                                                                      
        int pid=fork();
        if(pid==-1){
            printf("failed to fork\n");
            return -1;
        }
        if (pid == 0) {                              //child
            server_pid=getpid();
            printf("child..\n");
            fflush(stdout); 
            
            if (execl("./jobExecutorServer", "jobExecutorServer", NULL) == -1) {     //execute τον server (ο κώδικας του μπαίνει σε αυτή τη θέση)
                perror("Failed to execute jobExecutorServer");
                exit(EXIT_FAILURE); // Exit if execl fails

            }  
            //exit(EXIT_SUCCESS);
          
        }
        else{
            printf("parent...\n");

            //Θα διαβάσω το pid του server που πλέον έχει γραφτεί στο ServerFile.txt

            int fd=-1;                       
            while((fd=open("JobExecutorServer.txt", O_RDONLY))==-1){
                sleep(1);                                       //περίμενω μέχρι να δημιουργηθεί όντως το αρχείο 
                                                                //δηλα΄δη το παιδί - sever να φτάσει στο σημείο δημιουργίας του file
            }
            ssize_t bytes_read = read(fd, &server_pid, 9 ); 

            if(bytes_read==-1){
                perror("cannot read the pid of server");
                close(fd);
                exit(EXIT_FAILURE);
            }

            close(fd);
            printf("COMMANDER: PID of server now is : %d\n", server_pid);

            if(has_line_arguments){
                singal_and_write(single, server_pid);  //τσεκάρει το flag ώστε αν has_line_arguments==1 θα στείλει ο ίδιος ο initial commander
            }                                           //μόνο το job που πήρε ως line argument
            else{                                                       //αλλιώς διαβάζει ολόκληρο το input file line by line , και στέλνει την κάθε γραμμή
                file_open(server_pid);                                   //στον JobExecutorServer

            }
          
        } 
        
    }     


   
//Σε αυτό το σημείο ο commander έχει τελέιωσει την αποστολή των commands στον server και 
//πρεπει να περιμε΄νει για ότι πρεέπι να του επιστρέψει ο server 
//Για αυτό κάνω fork ένα παιδί του commander για διάβασμα

    int reader_pid=fork();

    if(reader_pid==-1){
        perror("FAILED FORKING");
        return -1;
    }
    else if(reader_pid==0){
        char my_message[100];                             //το μήνυμα που λαμβάνει
        int readf=-1;
        while(1){
            int read_fd = open(JOBSERVER_SEND, O_RDONLY);                 //ανοίγει το pipe που γράφει ο sever για διάβασμα 
            if (read_fd < 0) {
                perror("Failed to open pipe for reading");
                exit(EXIT_FAILURE);
            }
            int bytes=read(read_fd, my_message, sizeof(my_message) -1);              //διαβάζει 
            my_message[bytes]='\0';

            if(strcmp(my_message, "NOTHING ELSE")==0 ){                       //Όταν ο jobExecutorServer έχει τελει΄ωσει το execution των commands
                printf("THE END\n");                                          //και έχει στείλει ότι χρειάζεται πίσω σοτν JobCommander, θα του στείλει
                close(read_fd);                                               //το μήνυμα "NOTHING ELSE", ΄΄ωστε να μπορέσει ο jobCommander να βγεί από το loop διαβάσματος
                break;
            }
            printf("\n%s\n", my_message);                                    //εκτυπώνει ότι μήνυμα του έρχεται
            if( strcmp(my_message, "jobExecutorServer terminated.")==0){      //Σε περίπτωση job == exit που ο jobExecutorServer πρέπει να κάνει exit, o JobCommander
                close(read_fd);                                                //πρέπει και να εκτυπώσει το μήνυμα "jobExecutorServer terminated." και να κάνει επίσης exit
                break;
            }
  
            close(read_fd);


            
        }
        exit(EXIT_SUCCESS);                                    //το παι΄δι τελείωσε την δουλεία του (διάβασμα)


    }
    else{                                                    //ο JobCommander πρέπει να περιμένει το παιδί να τελείωσει το διάβασμα και να μην προχωρήσει
        int status;
        waitpid(reader_pid, &status, 0);
    }

   //Διαγράφω τα pipes
    if(unlink(JOBCOMMANDER_SEND)==-1){
        perror("Error deleting FIFO");
        exit(EXIT_FAILURE);
    } else {
        printf("\nFIFO deleted successfully\n");
    
    }
    if(unlink(JOBSERVER_SEND)==-1){
        perror("Error deleting FIFO");
        exit(EXIT_FAILURE);
    } else {
        printf("\nFIFO deleted successfully\n");
    
    }
    sleep(1);


  

    return 0;

}