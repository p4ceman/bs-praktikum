#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "keyValStore.h"
#include "sub.h"

#define BUFFERSIZE 1024 // Größe des Buffers
#define PORT 5678

int main() {
    int rfd; // Rendevouz-Descriptor
    int cfd; // Verbindungs-Descriptor

    struct sockaddr_in client; // Socketadresse eines Clients
    socklen_t client_len; // Länge der Client-Daten
    char input[BUFFERSIZE]; // Daten vom Client an den Server
    long bytes_read; // Anzahl der Bytes, die der Client geschickt hat

    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0 ){
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
    if (brt < 0 ){
        fprintf(stderr, "socket konnte nicht gebunden werden\n");
        exit(-1);
    }

    // Socket lauschen lassen
    int lrt = listen(rfd, 5);
    if (lrt < 0 ){
        fprintf(stderr, "socket konnte nicht listen gesetzt werden\n");
        exit(-1);
    }

    while (1) {
        // Verbindung eines Clients wird entgegengenommen
        cfd = accept(rfd, (struct sockaddr *) &client, &client_len);

        // Lesen von Daten, die der Client schickt
        bytes_read = read(cfd, input, BUFFERSIZE);
        input[bytes_read - 2] = '\0';

        while (bytes_read > 0) {
            char output[50] ="\n";

            if (isInputValid(input)) {
                char *key;
                char *value;
                int command = decodeCommand(input, &key, &value);
                int error;
                switch (command) {
                    case 0:
                        error = put(key, value);
                        break;
                    case 1:
                        error = get(key, &value);
                        break;
                    case 2:
                        error = del(key);
                        break;
                    case 3:
                        close(cfd);
                }
                printer(command, key, value, error, output);
                write(cfd, output, sizeof(output));
            }
            bytes_read = read(cfd, input, BUFFERSIZE);
            input[bytes_read - 2] = '\0';
        }
    }
}