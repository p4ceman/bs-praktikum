//Datei für die restlichen (allgemeinen) Subroutinen
#include "sub.h"
#include "keyValStore.h"
#include <string.h>
#include <regex.h>

// isInputValid vorher laufen lassen
// PUT 0
// GET 1
// DEL 2
// QUIT 3
int decodeCommand(char *input, char **key, char **value) {
    char *command;
    command = strtok(input, " ");
    if (strcmp(command, "put") == 0 || strcmp(command, "PUT") == 0) {
        *key = strtok(NULL, " ");
        *value = strtok(NULL, " ");
        return 0;
    } else if (strcmp(command, "get") == 0 || strcmp(command, "GET") == 0) {
        *key = strtok(NULL, " ");
        return 1;
    } else if (strcmp(command, "del") == 0 || strcmp(command, "DEL") == 0) {
        *key = strtok(NULL, " ");
        return 2;
    }
    return 3;
}

/*Prüft, ob übergebener Input zulässig ist
Rückgabewert:   1 => String ist valide
                0 => String ist nicht valide*/
int isInputValid(char input[]) {
    regex_t regex;
    regcomp(&regex,"^((((put|PUT) ([A-Za-z0-9])+|(get|GET)|(del|DEL)) ([A-Za-z0-9])+)|(quit|QUIT))$", REG_EXTENDED);
    if(regexec(&regex, input, 0, NULL, 0) == 0){
        return 1;
    } else{
        return 0;
    }
}


void printer(int command, char* key, char* value, int error, char string []){
    strcat(string, "> ");
    switch(command){
        case 0:
            strcat(string, "PUT:");
            strcat(string, key);
            strcat(string, ":");
            if(error == -1){
                strcat(string, "array_is_full");
            }
            else{
                strcat(string, value);
            }
            break;
        case 1:
            strcat(string, "GET:");
            strcat(string, key);
            strcat(string, ":");
            if(error == -1){
                strcat(string, "key_nonexistent");
            }
            else{
                strcat(string, value);
            }
            break;
        case 2:
            strcat(string, "DEL:");
            strcat(string, key);
            strcat(string, ":");
            if(error == -1){
                strcat(string, "key_nonexistent");
            }
            else{
                strcat(string, "key_deleted");
            }
            break;
    }
}
