#include <stdio.h>
#include "operacje.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/sem.h>

#define MAX 10 //ilosc buforow
#define MAX2 12 //dwa ostatnie indeksy na wskazniki odczytu i zapisu
#define SEM_NUM 3
#define zapis pam[MAX+1] 
#define WRITE_SEM 0
#define READ_SEM 2
#define MUTEX_SEM 1

int main()
{
    key_t kluczSemafor, kluczPamiec;
    int semID, shmID;
    int *pam;
    int tab1[1], tab2[1];
    pid_t pidNr;


    //dolaczenie do zestawu semaforow
    if((kluczSemafor = ftok(".", 'A')) == -1) //generowanie klucza
    {
        printf("blad ftok A (write)\n");
        exit(1);
    }
    semID = semget(kluczSemafor, SEM_NUM, IPC_CREAT | 0666);
    if(semID == -1)
    {
        printf("blad semafora(write)\n");
        exit(1);
    }

    //pamiec dzielona
    if((kluczPamiec = ftok(".", 'B')) == -1)
    {
        printf("blad ftok B(write)\n");
        exit(1);
    }
    shmID = shmget(kluczPamiec, MAX2 * sizeof(int), IPC_CREAT | 0666);
    if(shmID == -1)
    {
        printf("Blad pamieci dzielonej (write)\n");
        exit(1);
    }

    //dolacanie pam. dzielonej
    pam = (int*)shmat(shmID, NULL, 0);
    
    tab1[0]=WRITE_SEM;
    signalSemaforVE(semID, tab1, 1); //signal(write)

    tab1[0] = MUTEX_SEM;
    tab2[0] = READ_SEM;
    waitSemaforPE(semID, tab1, 1, tab2, 1); //wait(mutex, read)

    //pisz
    pidNr=getpid();
    pam[zapis] = pidNr;
    printf("PISARZ pam[%d]: %d\n", zapis, pam[zapis]);
    zapis = (zapis+1) % MAX;
    
    signalSemaforVE(semID, tab1, 1);//signal(mutex)

    tab1[0]= WRITE_SEM;
    waitSemaforPE(semID, tab1,1, tab2, 0); //wait(write)


    exit(0);
}