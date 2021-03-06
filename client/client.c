/*
* @file client.c
* @author Renat Kagal <kagal@itspartner.net>
*
* Assembling : gcc -Wall client.c -o client
*
* Description : client in bank
*
* Copyright (c) 2021, ITS Partner LLC.
* All rights reserved.
*
* This software is the confidential and proprietary information of
* ITS Partner LLC. ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered into
* with ITS Partner.
*/

#define _GNU_SOURCE

#include "head_client.h"

#define BUFFER_SIZE 32

#define PORT 2000

int main () {
    int sock;
    struct sockaddr_in  addr;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        puts ("Failed create socket");
        exit (EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons (PORT);
    addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

    if (connect (sock, (struct sockaddr*) &addr, sizeof (addr)) < 0) {
        puts ("Failed connection to server");
        exit (EXIT_FAILURE);
    }
    char buff[BUFFER_SIZE];
    memset (buff, ' ', BUFFER_SIZE);

    while (fgets (buff, BUFFER_SIZE - 1, stdin)) {
        if (strcmp (buff, "stop\n") == 0) {
            break;
        }

        if (strcmp (buff, "bank\n") == 0) {
            bank (sock);
        }
        memset (buff, '\0', BUFFER_SIZE);
    }
    close (sock);
}

void bank (int sock) {
    char buff[BUFFER_SIZE] = "bank";
    
    if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed send");
        exit (EXIT_FAILURE);
    }
    memset (buff, '\0', BUFFER_SIZE);

    while (1) {
        fgets (buff, BUFFER_SIZE - 1, stdin);
        
        if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed send");
            exit (EXIT_FAILURE);
        }
        memset (buff, '\0', BUFFER_SIZE);
        
        if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
        }

        if (strcmp (buff, "right") != 0) {
            return;
        }

        memset (buff, '\0', BUFFER_SIZE);
    }
}