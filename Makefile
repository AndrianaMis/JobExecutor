CC := gcc
CFLAGS=-Iincludes


SERVER_SRC=modules/JobExecutorServer.c modules/queue.c modules/job_functions.c
COMMANDER_SRC=modules/JobCommander.c
EXECUTOR_SRC=modules/JobExecutor.c

SERVER_OBJ=$(SERVER_SRC:.c=.o)
COMMANDER_OBJ=$(COMMANDER_SRC:.c=.o)
EXECUTOR_OBJ=$(EXECUTOR_SRC:.c=.o)

SERVER_EXEC=jobExecutorServer
COMMANDER_EXEC=jobCommander
EXECUTOR_EXEC=executor

.PHONY: clean

all: $(SERVER_EXEC) $(COMMANDER_EXEC)

$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(SERVER_OBJ) -o $@

$(COMMANDER_EXEC): $(COMMANDER_OBJ)
	$(CC) $(COMMANDER_OBJ) -o $@

$(EXECUTOR_EXEC): $(EXECUTOR_OBJ)
	$(CC) $(EXECUTOR_OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

set_permissions:
	chmod +x $(SERVER_EXEC) $(COMMANDER_EXEC)

del_pipes:
	./$(EXECUTOR_EXEC)

clean:
	rm -f $(SERVER_OBJ) $(COMMANDER_OBJ) $(SERVER_EXEC) $(COMMANDER_EXEC)

Commander:
	./$(COMMANDER_EXEC)

ExecutorServer:
	./$(SERVER_EXEC)