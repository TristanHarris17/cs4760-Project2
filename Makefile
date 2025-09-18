CC = g++
CFLAGS = -Wall -g

OSS_SRC = oss.cpp
WORKER_SRC = worker.cpp

OSS_BIN = oss
WORKER_BIN = worker

all: $(OSS_BIN) $(USER_BIN)

$(OSS_BIN): $(OSS_SRC)
	$(CC) $(CFLAGS) -o $(OSS_BIN) $(OSS_SRC)

$(USER_BIN): $(USER_SRC)
	$(CC) $(CFLAGS) -o $(USER_BIN) $(USER_SRC)

clean:
	rm -f $(OSS_BIN) $(USER_BIN) *.o

.PHONY: all clean