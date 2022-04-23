//Datei für die restlichen (allgemeinen) Subroutinen
#include "sub.h"
#include <string.h>
#include <regex.h>

/*Prüft, ob übergebener Input zulässig ist
Rückgabewert:   1 => String ist valide
                0 => String ist nicht valide*/
int isInputValid(char* input){
    regex_t regex;
    regcomp(&regex,"^((((put|PUT) ([A-Za-z0-9])+|(get|GET)|(del|DEL)) ([A-Za-z0-9])+)|(quit|QUIT))$", REG_EXTENDED);
    if(regexec(&regex, input, 0, NULL, 0) == 0){
        return 1;
    } else{
        return 0;
    }
}
