#include "keyValStore.h"
#include <string.h>
#include <stdio.h>
#include <regex.h>
#define ARRAYLENGTH 100

typedef struct keyValue{
    char* key;
    char* value;
} KeyValue;

KeyValue dictionary[ARRAYLENGTH];

void initializeDictionary() {

}

/*Prüft, ob übergebener String keine unzulässigen Zeichen, wie Sonderzeichen enthält
Rückgabewert:   1 => String ist valide
                0 => String ist nicht valide*/
int isStringValid(char* string){
    regex_t regex;
    regcomp(&regex,"^([A-Za-z0-9])*$", REG_EXTENDED);
    if(regexec(&regex, string, 0, NULL, 0) == 0){
        return 1;
    } else{
        return 0;
    }
}

//Die put() Funktion soll eine Wert (value) mit dem Schlüsselwert (key) hinterlegen.
//Wenn der Schlüssel bereits vorhanden ist, soll der Wert überschrieben werden.
//Der Rückgabewert der Funktion könnte Auskunft dazu geben.
//Gibt 0 zurück wenn neuer Key hinzugefügt wurde, 1 wenn Value zu bereits existierendem Key überschrieben würde
//und -1 wenn das Array voll ist.
int put(char* key, char* value) {
    //accept(key)
    //accept(value)
    int i = 0;
    while (dictionary[i].key != NULL) {
        if (strcmp(dictionary[i].key,key)==0) {
            dictionary[i].value = value;
            return(1);
        }
        i++;
        if (i == ARRAYLENGTH) {
            return(-1);
        }
    }
    dictionary[i].key = key;
    dictionary[i].value = value;
    return(0);
}


//Die get() Funktion soll einen Schlüsselwert (key) in der Datenhaltung suchen und den hinterlegten Wert (value) zurückgeben.
// Ist der Wert nicht vorhanden, wird durch einen Rückgabewert <0 darauf aufmerksam gemacht.
// Gibt 0 zurück wenn key gefunden wurde , -1 wenn key nicht vorhanden ist
int get(char* key, char* res) {
int i=0;
    while (dictionary[i].key != NULL){
        if(strcmp(dictionary[i].key,key)==0){
            *res = dictionary[i].value;
            return 0;
        }
        i++;
        if (i==ARRAYLENGTH){
            return -1;
        }
    }
    return -1;
}// &res beim aufrufen in main nutzen /beachten... funktuniert sinnst nicht!!!!

int del(char* key) {

}