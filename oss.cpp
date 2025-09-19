#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>

using namespace std;

key_t sh_key = ftok("oss.cpp", 0);
const int increment_amount = 100000000;

void increment_clock(int* sec, int* nano, int amt) {
    const long long NSEC_PER_SEC = 1000000000LL;
    long long total = (long long)(*nano) + (long long)amt;
    if (total >= NSEC_PER_SEC) {
        *sec += (int)(total / NSEC_PER_SEC);
        *nano = (int)(total % NSEC_PER_SEC);
    } else {
        *nano = (int)total;
    }
}

int main() {
    // create shared memory
    int shmid = shmget(sh_key, sizeof(int)*2, IPC_CREAT | 0666);
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
    *sec = *nano = 0;

    pid_t worker_pid = fork();
    if (worker_pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (worker_pid == 0) {
        char* args[] = {(char*)"./worker", (char *)to_string(5).c_str(), (char *)to_string(5000).c_str(), NULL};
        execv(args[0], args);
        perror("Exec failed");
        exit(1);
    }

    while (*sec < 5) {
        increment_clock(sec, nano, increment_amount);
        cout << "Seconds: " << *sec << " Nano: " << *nano << endl;
    }

    // Free shared memory
    shmdt(clock);
    shmctl(shmid, IPC_RMID, nullptr);
    return 0;
}