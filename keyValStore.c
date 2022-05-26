#include "keyValStore.h"
#include <string.h>
#include <sys/shm.h>
#include <stdio.h>

#define LENGTH 100
#define SIZE sizeof(KeyValue) * LENGTH
#define KEYVALUELENGTH 1024

// TODO: LENGTH Check

typedef struct keyValue{
    char key[KEYVALUELENGTH];
    char value[KEYVALUELENGTH];
} KeyValue;

KeyValue* dictionary;

//Die initSharedMemory() weißt dem dictionary einen Shared Memory Bereich zu.
void initSharedMemory() {
    int shm_id = shmget(IPC_PRIVATE, SIZE, IPC_CREAT|0600);
    dictionary = (KeyValue *) shmat(shm_id, 0,0);
}

void detachSharedMemory(){
    int result = shmdt(0);
    if(result < 0){
        fprintf(stderr, "Shared Memory konnte nicht detached werden\n");
    }
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
            return 1;
        }
        i++;
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
    while (dictionary[i].key[0] != '\0') {
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
    while (dictionary[i].key[0] != '\0') {
        if (strcmp(dictionary[i].key, key) == 0) {
            while (dictionary[i].key[0] != '\0') {
                strcpy(dictionary[i].key, dictionary[i + 1].key);
                strcpy(dictionary[i].value, dictionary[i + 1].value);
                i++;
            }
            return 1;
        }
        i++;
    }
    return -1;
}