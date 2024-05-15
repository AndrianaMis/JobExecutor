#include <sys/types.h>
#include <sys/stat.h>
#include "../includes/JobExecutor.h"

int main()
{
   
  /* if(mkfifo(JOBCOMMANDER_SEND, 0666)<0) perror("can't create fifo com");
   if(mkfifo(JOBSERVER_SEND, 0666)<0) perror("can't create fifo serv");

    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        execl("./com", "jobCommander", NULL);
        perror("Error executing jobCommander");
        exit(EXIT_FAILURE);
    }

*/

    // Parent process
  //  printf("jobCommander started with PID: %d\n", pid);

    if(unlink(JOBCOMMANDER_SEND)==-1){
        perror("Error deleting FIFO");
        exit(EXIT_FAILURE);
    } else {
        printf("FIFO deleted successfully\n");
    
    }
    if(unlink(JOBSERVER_SEND)==-1){
        perror("Error deleting FIFO");
        exit(EXIT_FAILURE);
    } else {
        printf("FIFO deleted successfully\n");
    
    }
    

}