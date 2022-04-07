
#pragma once

#define SEM_A 0
#define SEM_B 1

extern int semid;
extern int shmid;
extern int * shm_pointer;

void signalHandler(int sig);

void errExit(char * msg);

void semOp (int semid, unsigned short sem_num, short sem_op);

key_t getKey();

int getShmId();

void semWait(int semid, int sem_num);

void semSignal(int semid, int sem_num);

void semSetVal(int semid, int sem_num, int val);

void semSetAll(int semid, short unsigned int values[]);

void semDelete(int semid);

struct semid_ds semGetStats(int semid);

void semSetPerm(int semid, struct semid_ds arg);

void shmDetach(int * shm_pointer);

void shmDelete(int shmid);

void shmDetachAndDelete(int shmid, int * shm_pointer);
