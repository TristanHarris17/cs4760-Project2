#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <cstdio>
#include <cstdlib>

using namespace std;

const int sh_key = ftok("oss.cpp", 0);

int main() {
    int shmid = shmget(sh_key, sizeof(int)*2, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    int* shm_ptr = (int*) shmat(shmid, nullptr, 0);
    if (shm_ptr == (int*) -1) {
        perror("shmat");
        exit(1);
    }

    shm_ptr[0] = 10; // Initialize first integer
    shm_ptr[1] = 400; // Initialize second integer

    cout << "Initialized shared memory with values: "
         << shm_ptr[0] << ", " << shm_ptr[1] << endl;

    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, nullptr);

    cout << "Shared memory segment removed." << endl;

    return 0;
}