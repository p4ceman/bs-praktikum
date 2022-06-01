#include "keyValStore.h"
#include "sub.h"
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

#define LENGTH 100
#define SIZE sizeof(KeyValue) * LENGTH
#define SIZE2 sizeof(SubStore) * LENGTH
#define KEYVALUELENGTH 1024

typedef struct keyValue{
    char key[KEYVALUELENGTH];
    char value[KEYVALUELENGTH];
} KeyValue;

KeyValue* dictionary;

typedef struct subStore{
    char key[KEYVALUELENGTH];
    int  subscriber[LENGTH];
} SubStore;

SubStore* subStore;

typedef struct message{
    long int mtype;
    char mtext[7+2*KEYVALUELENGTH];
} Message;

//Die initSharedMemory() weißt dem dictionary und dem subStore jeweils einen Shared Memory Bereich zu.
void initSharedMemory() {
    int shm_id = shmget(IPC_PRIVATE, SIZE, IPC_CREAT|0600);
    if (shm_id == -1) {
        perror("Shared Memory für KeyValueStore konnte nicht angelegt werden");
        exit(1);
    }
    dictionary = (KeyValue *) shmat(shm_id, 0,0);

    int shm_id2 = shmget(IPC_PRIVATE, SIZE2, IPC_CREAT|0600);
    if (shm_id2 == -1) {
        perror("Shared Memory für SubStore konnte nicht angelegt werden");
        exit(1);
    }
    subStore = (SubStore *) shmat(shm_id2, 0,0);
}

//Methoden zum Freigeben des Shared Memory
void detachSharedMemory(){
    int result = shmdt(0);
    if(result < 0){
        fprintf(stderr, "Shared Memory konnte nicht detached werden\n");
    }
}
int sem_id, sem_id2;
struct sembuf enter, leave;

//Initialisiert eine Semaphore zum Blockieren von Prozessen
void initSemaphore() {
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (sem_id == -1) {
        perror("Semaphore für KeyValueStore konnte nicht angelegt werden");
        exit(1);
    }

    sem_id2 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (sem_id == -1) {
        perror("Semaphore für SubStore konnte nicht angelegt werden");
        exit(1);
    }
    unsigned short token[1];
    token[0] = 1;
    semctl(sem_id, 1, SETALL, token); //Initialisiere Semaphore mit einem Platz
    semctl(sem_id2, 1, SETALL, token); //Initialisiere Semaphore mit einem Platz
    enter.sem_num = leave.sem_num = 0;
    enter.sem_flg = leave.sem_flg = SEM_UNDO;
    enter.sem_op = -1; //DOWN-Operation
    leave.sem_op = 1; //UP-Operation
}

void detachSemaphore() {
    int result = semctl(sem_id, 0, IPC_RMID);
    if (result == -1) {
        perror("Semaphore für KeyValueStore konnte nicht detached werden");
        exit(1);
    }
    result = semctl(sem_id2, 0, IPC_RMID);
    if (result == -1) {
        perror("Semaphore für SubStore konnte nicht detached werden");
        exit(1);
    }
}

int msg_id;
void initMessageQueue() {
    msg_id = msgget(100, IPC_CREAT | 0666);
}

//Methode für den Eintritt in den kritischen Bereich
int beg(int index) {
    if (index == 1) {
        int result = semop(sem_id, &enter, 1);
        return result;
    }
    if (index == 2) {
        int result = semop(sem_id2, &enter, 1);
        return result;
    }
}

//Methode für den Verlassen des kritischen Bereichs
int end(int index) {
    if (index == 1) {
        int result = semop(sem_id, &leave, 1);
        return result;
    }
    if (index == 2) {
        int result = semop(sem_id2, &leave, 1);
        return result;
    }
}

// Methode zum Subscriben für einen Key
int sub(char* key, int pid) {
    char test[KEYVALUELENGTH];
    if (get(key, test) == -1) {
        return -1;
    }
    beg(2);
    int i = 0;
    while (subStore[i].key[0] != '\0') {
        if (strcmp(subStore[i].key, key) == 0) {
            for (int j = 1; j<LENGTH; j++) {
                if (subStore[i].subscriber[j] == 0) {
                    subStore[i].subscriber[j] = pid;
                    end(2);
                    return 1;
                }
            }
            //subscriber length check
            end(2);
            return -1;
        }
        i++;
        if (i>=LENGTH) {
            end(2);
            return -1;
        }
    }
    strcpy(subStore[i].key, key);
    subStore[i].subscriber[0] = pid;
    end(2);
    return 1;
}

void notifySubscriber(char* key, int command) {
    char string[2055] = "\0";
    char value[KEYVALUELENGTH] = "\0";
    if (command == 1) {
        get(key, value);
        printer(0,key, value,0, string);
        int done = 0;
        for (int i = 0; i<LENGTH; i++) {
            if (strcmp(subStore[i].key, key) == 0) {
                for (int j = 0; j<LENGTH; j++) {
                    if (subStore[i].subscriber[j]!=0) {
                        Message message;
                        message.mtype = subStore[i].subscriber[j];
                        message.mtext[0] = '\0';
                        strcpy(message.mtext,string);
                        msgsnd(msg_id, &message, 2055, 0);
                        done = 1;
                    }
                }
            }
            if (done == 1) {
                break;
            }
        }
    } else {
        printer(2, key, value,0, string);
        int done = 0;
        int i;
        for (i = 0; i<LENGTH; i++) {
            if (strcmp(subStore[i].key, key) == 0) {
                for (int j = 0; j<LENGTH; j++) {
                    if (subStore[i].subscriber[j]!=0) {
                        Message message;
                        message.mtype = subStore[i].subscriber[j];
                        message.mtext[0] = '\0';
                        strcpy(message.mtext,string);
                        msgsnd(msg_id, &message, 2055, 0);
                        done = 1;
                    }
                }
            }
            if (done == 1) {
                break;
            }
        }
        subStore[i].key[0] = '\0';
        for (int j = 0; j<LENGTH; j++) {
            subStore[i].subscriber[j] = 0;
        }
    }
}

int recieveMessage(int pid, char* string) {
    Message receivedMessage;
    int status = msgrcv(msg_id, &receivedMessage, 2055, pid, IPC_NOWAIT);
    if (status < 0) {
        return -1;
    }
    strcpy(string, receivedMessage.mtext);
    return 0;
}


//Die put() Funktion soll eine Wert (value) mit dem Schlüsselwert (key) hinterlegen.
//Wenn der Schlüssel bereits vorhanden ist, soll der Wert überschrieben werden.
//Der Rückgabewert der Funktion könnte Auskunft dazu geben.
//Gibt 0 zurück wenn neuer Key hinzugefügt wurde, 1 wenn Value zu bereits existierendem Key überschrieben würde
//und -1 wenn das Array voll ist. return -2 wenn strings nicht valide sind.
int put(char* key, char* value) {

    int i = 0;
    while (dictionary[i].key[0] != '\0') {
        if (strcmp(dictionary[i].key, key) == 0) {
            strcpy(dictionary[i].value, value);
            beg(2);
            notifySubscriber(key, 1);
            end(2);
            return 1;
        }
        i++;
        if(i >= LENGTH){
            return -1;
        }
    }
    printf("New key-value pair at: %p\n", dictionary+i);
    strcpy(dictionary[i].key, key);
    strcpy(dictionary[i].value, value);
    return 0;
}

//Die get() Funktion soll einen Schlüsselwert (key) in der Datenhaltung suchen und den hinterlegten Wert (value) zurückgeben.
// Ist der Wert nicht vorhanden, wird durch einen Rückgabewert <0 darauf aufmerksam gemacht.
// Gibt 0 zurück wenn key gefunden wurde , -1 wenn key nicht vorhanden ist, -2 wenn key nicht valiede
// &res beim aufrufen in main nutzen /beachten... funktuniert sinnst nicht!!!!
int get(char* key, char* res) {
    int i = 0;
    while (i < LENGTH && dictionary[i].key[0] != '\0') {
        if (strcmp(dictionary[i].key, key) == 0) {
            strcpy(res, dictionary[i].value);
            return 0;
        }
        i++;
    }
    return -1;
}

//Die del() Funktion soll einen Schlüsselwert suchen und zusammen mit dem Wert aus der Datenhaltung entfernen.
// Rückgabewert: -1 -> String is nicht valide
//                0 -> Key nicht vorhanden
//                1 -> Key und Value wurden erfolgreich entfernt
int del(char* key) {
    int i = 0;
    while (i < LENGTH && dictionary[i].key[0] != '\0') {
        if (strcmp(dictionary[i].key, key) == 0) {
            beg(2);
            notifySubscriber(key, 2);
            end(2);
            while (i < LENGTH && dictionary[i].key[0] != '\0') {
                if(i == LENGTH - 1){
                    dictionary[i].key[0] = '\0';
                    dictionary[i].value[0] = '\0';
                }
                else {
                    strcpy(dictionary[i].key, dictionary[i + 1].key);
                    strcpy(dictionary[i].value, dictionary[i + 1].value);
                }
                i++;
            }
            return 0;
        }
        i++;
    }
    return -1;
}