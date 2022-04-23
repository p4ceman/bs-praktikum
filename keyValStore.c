#include "keyValStore.h"
#include <string.h>
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
        if (i == 100) {
            return(-1);
        }
    }
    dictionary[i].key = key;
    dictionary[i].value = value;
    return(0);
}

int get(char* key, char* res) {

}

//Die del() Funktion soll einen Schlüsselwert suchen und zusammen mit dem Wert aus der Datenhaltung entfernen.
// Rückgabewert: 0 -> Key nicht vorhanden
//               1 -> Key und Value wurden erfolgreich entfernt
int del(char* key) {
    accept(key);
    int i = 0;
    while (dictionary[i].key != NULL) {
        if (strcmp(dictionary[i].key, key) == 0) {
            while (dictionary[i].key != NULL) {
                if (i == ARRAYLENGTH - 1) {
                    dictionary[i].key = NULL;
                    dictionary[i].value = NULL;
                }
                dictionary[i].key = dictionary[i + 1].key;
                dictionary[i].value = dictionary[i + 1].value;
                i++;
            }
            return 1;
        }
        i++;
        if (i == ARRAYLENGTH) {
            return 0;
        }
    }
    return 0;
}