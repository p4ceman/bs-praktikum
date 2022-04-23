#ifndef BS_PRAKTIKUM_SUB_H
#define BS_PRAKTIKUM_SUB_H
#endif //BS_PRAKTIKUM_SUB_H

int isInputValid(char* input);
int decodeCommand(char *input, char **key, char **value);
void printer(int command, char* key, char* value, int error, char string []);