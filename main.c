#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "keyValStore.h"
#include "sub.h"
#include  <signal.h>

#define KEYVALUELENGTH 1024 // Größe des Buffers
#define OUTPUTBUFFERSIZE 2048
#define PORT 5678

int masterpid;

void handleExit(int sig){
    if(getpid() != masterpid){
        exit(0);
    }
    detachMessageQueue();
    detachSemaphore();
    detachSharedMemory();
    exit(0);
}


int main() {
    masterpid = getpid();

    struct sigaction sa;
    sa.sa_handler = &handleExit;
    sigaction(SIGINT, &sa, NULL);


    int rfd; // Rendevouz-Descriptor
    int cfd; // Verbindungs-Descriptor

    struct sockaddr_in client; // Socketadresse eines Clients
    socklen_t client_len; // Länge der Client-Daten
    char input[KEYVALUELENGTH]; // Daten vom Client an den Server
    long bytes_read; // Anzahl der Bytes, die der Client geschickt hat

    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0) {
        fprintf(stderr, "socket konnte nicht erstellt werden\n");
        exit(-1);
    }

    // Socket Optionen setzen für schnelles wiederholtes Binden der Adresse
    int option = 1;
    setsockopt(rfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, sizeof(int));

    // Socket binden
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    int brt = bind(rfd, (struct sockaddr *) &server, sizeof(server));
    if (brt < 0) {
        fprintf(stderr, "socket konnte nicht gebunden werden\n");
        exit(-1);
    }

    // Socket lauschen lassen
    int lrt = listen(rfd, 5);
    if (lrt < 0) {
        fprintf(stderr, "socket konnte nicht listen gesetzt werden\n");
        exit(-1);
    }

    // Initialize Sharded Memory, Semaphore and Message Queue
    initSharedMemory();
    initSemaphore();
    initMessageQueue();

    while (1) {
        // Verbindung eines Clients wird entgegengenommen
        cfd = accept(rfd, (struct sockaddr *) &client, &client_len);

        int pid = fork();

        if (pid == 0) {
            int newpid = fork();
            if(newpid == 0) {
                newpid = getpid();
                char submessage[2055] = "\0";
                int test = 0;
                while (test != 1) {
                    int status = recieveMessage(newpid, submessage);
                    if (status == 0) {
                        write(cfd, submessage, sizeof (submessage));
                    }
                    test = getppid();
                }
                close(cfd);
            } else if (newpid > 0) {
                // Lesen von Daten, die der Client schickt
                bytes_read = read(cfd, input, KEYVALUELENGTH);
                input[bytes_read - 2] = '\0';
                int semaphoreStatus = 1;

                while (bytes_read > 0) {
                    //überprüfung, ob semaphore aktiv ist
                    if (semaphoreStatus == 1) {
                        beg(1);
                        end(1);
                    }
                    char output[OUTPUTBUFFERSIZE] = "\0";
                    if (isInputValid(input)) {
                        char key[KEYVALUELENGTH];
                        char value[KEYVALUELENGTH];
                        int command = decodeCommand(input, key, value);
                        int error;
                        switch (command) {
                            case 0:
                                error = put(key, value);
                                break;
                            case 1:
                                error = get(key, value);
                                break;
                            case 2:
                                error = del(key);
                                break;
                            case 3:
                                if (semaphoreStatus == 0) {
                                    end(1);
                                }
                                close(cfd);
                                exit(0);
                            case 4:
                                error = beg(1);
                                semaphoreStatus = 0;
                                break;
                            case 5:
                                error = end(1);
                                semaphoreStatus = 1;
                                break;
                            case 6:
                                error = sub(key, newpid);
                                break;
                        }
                        printer(command, key, value, error, output);
                        write(cfd, output, sizeof(output));
                    } else {
                        strcat(output, "Your command is not valid!\n");
                        write(cfd, output, sizeof(output));
                    }
                    bytes_read = read(cfd, input, KEYVALUELENGTH);
                    input[bytes_read - 2] = '\0';
                }
            } else {
                printf("Kindprozess konnte nicht erstellt werden!");
            }
        } else {
            close(cfd);
        }
    }
}