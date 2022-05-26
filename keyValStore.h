#ifndef BS_PRAKTIKUM_KEYVALSTORE_H
#define BS_PRAKTIKUM_KEYVALSTORE_H
#endif //BS_PRAKTIKUM_KEYVALSTORE_H

void initSharedMemory();
void detachSharedMemory();
void initSemaphore();
void detachSemaphore();
void testSem();
int beg();
int end();
int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);