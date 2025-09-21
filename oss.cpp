#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>

using namespace std;

const int increment_amount = 1000;

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

// convert float time interval to seconds and nanoseconds and return nannoseconds
int seconds_conversion(float interval) {
    int seconds = (int)interval;
    float fractional = interval - (float)seconds;
    int nanoseconds = (int)(fractional * 1e9);
    return nanoseconds;
}

pid_t child_Terminated() {
    int status;
    pid_t result = waitpid(-1, &status, WNOHANG);
    if (result > 0) {
        return result;
    }
    return -1;
}

void launch_worker(float time_limit) {
    pid_t worker_pid = fork();
    if (worker_pid < 0) {
        cerr << "fork failed" << endl;
        exit(1);
    }

    if (worker_pid == 0) {
        // keep these std::string objects alive until execv runs
        string arg_sec = to_string((int)time_limit);
        string arg_nsec = to_string(seconds_conversion(time_limit));
        char* args[] = {
            (char*)"./worker",
            const_cast<char*>(arg_sec.c_str()),
            const_cast<char*>(arg_nsec.c_str()),
            NULL
        };
        execv(args[0], args);
        cerr << "Exec failed" << endl;
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    //parse command line args
    int proc = -1;
    int simul = -1;
    float time_limit = -1;
    float launch_interval = -1;
    int opt;

    while((opt = getopt(argc, argv, "hn:s:t:i:")) != -1) {
        switch(opt) {
            case 'h': {
                cout << "HELP MESSAGE" << endl;
                exit(0);
            }
            case 'n': {
                int val = stoi(optarg);
                if (val < 0) {
                    cerr << "Error: -n must be a non-negative integer." << endl;
                    exit(1);
                }
                proc = val;
                break;
            }
            case 's': {
                int val = stoi(optarg);
                if (val < 0) {
                    cerr << "Error: -s must be a non-negative integer." << endl;
                    exit(1);
                }
                simul = val;
                break;
            }
            case 't': {
                float val = stof(optarg);
                if (val < 0) {
                    cerr << "Error: -t must be a non-negative integer." << endl;
                    exit(1);
                }
                time_limit = val;
                break;
            }
            case 'i': {
                float val = stof(optarg);
                if (val < 0) {
                    cerr << "Error: -i must be a non-negative integer." << endl;
                    exit(1);
                }
                launch_interval = val;
                break;
            }
            default:
                cerr << "Error: All options -n, -s, -t, and -i are required and must be non-negative integers." << endl;
                exit(1);
        }
    }

    // create shared memory
    key_t sh_key = ftok("oss.cpp", 0);
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

    // pointers to seconds and nanoseconds in shared memory
    int *sec = &(clock[0]);
    int *nano = &(clock[1]);
    *sec = *nano = 0;

    // oss staring message
    cout << "OSS starting, PID:" << getpid() << " PPID:" << getppid() << endl
         << "Called With:" << endl
         << "-n: " << proc << endl
         << "-s: " << simul << endl
         << "-t: " << time_limit << endl
         << "-i: " << launch_interval << endl;

    int launched_processes = 0;
    int running_processes = 0;

    while (launched_processes < proc || running_processes > 0) {
        increment_clock(sec, nano, increment_amount);
        //cout << "Seconds: " << *sec << " Nano: " << *nano << endl;

        // checking if child has terminated
        pid_t term_pid = child_Terminated();
        if (term_pid > 0) {
            running_processes--;
            cout << "Child process terminated." << term_pid << endl;
        }

        if (launched_processes < proc && running_processes < simul) {
            launch_worker(time_limit);
            launched_processes++;
            running_processes++;
        }
    }

    cout << "Number of processes launched: " << launched_processes << endl;

    shmdt(clock);
    shmctl(shmid, IPC_RMID, nullptr);
    return 0;
}


/*

        if (launched_processes < proc) {
            launched_processes++;
            running_processes++;
            pid_t worker_pid = fork();
            if (worker_pid < 0) {
                perror("fork failed");
                exit(1);
            } else

            if (worker_pid == 0) {
            // keep these std::string objects alive until execv runs
                string arg_sec = to_string((int)time_limit);
                string arg_nsec = to_string(seconds_conversion(time_limit));
                char* args[] = {
                    (char*)"./worker",
                    const_cast<char*>(arg_sec.c_str()),
                    const_cast<char*>(arg_nsec.c_str()),
                    NULL
                };
            execv(args[0], args);
            perror("Exec failed");
            exit(1);
            }
        }
*/