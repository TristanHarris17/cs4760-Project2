#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
    int target_seconds = stoi(argv[1]);
    int target_nano = stoi(argv[2]);

    key_t sh_key = ftok("oss.cpp", 0);

    // create/get shared memory
    int shmid = shmget(sh_key, sizeof(int)*2, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to shm_ptr
    int* clock = (int*) shmat(shmid, nullptr, 0);
    if (clock == (int*) -1) {
        perror("shmat");
        exit(1);
    }

    int *sec = &(clock[0]);
    int *nano = &(clock[1]);

    // Print starting message
    cout << "Worker starting, " << "PID:" << getpid() << " PPID:" << getppid() << endl
         << "Called With:" << endl
         << "Interval: " << target_seconds << " seconds, " << target_nano << " nanoseconds" << endl;

    shmdt(clock);
    return 0;
}