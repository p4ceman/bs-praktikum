//Datei für die restlichen (allgemeinen) Subroutinen

#include "sub.h"
#include "keyValStore.h"
#include <string.h>

//invalide -1
// PUT 0
// GET 1
// DEL 2
// QUIT 3
int interpreter(char *input, char **key, char **value) {
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
    return -1;
}
