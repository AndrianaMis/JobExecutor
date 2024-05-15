#include "queue.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#define JOBCOMMANDER_SEND "/tmp/103fifo1"
#define JOBSERVER_SEND "/tmp/103fifo2"
#include <sys/stat.h>
#include <signal.h>
#define MAX_LINE_LENGTH 2000
#include <sys/errno.h>


extern int concurrency;
void singal_and_write(char* , int );
void file_open(int );
void process_line(const char* , int );
void issueJob(tripl, queue);
void setCon(char*);
void stopJob(char*, queue, queue);
void poll(char* , queue, queue);
void exit_();
void choose_function(queue);