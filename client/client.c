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

int flag_work ;

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

        flag_work = 1;
        if (strcmp (buff, "bank\n") == 0) {
            while (flag_work == 1) {
                bank (sock);
            }
          //  break;
        }
        memset (buff, '\0', BUFFER_SIZE);
    }
    close (sock);
}

void bank (int sock) {
    char buff[BUFFER_SIZE] = "bank";
    int  reciept_flag;

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

        if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
        }
        //puts (buff);

        if (strcmp (buff, "confirmed") == 0) {
            break;
        }
        memset (buff, '\0', BUFFER_SIZE);
    }

    memset (buff, '\0', BUFFER_SIZE);
    fgets (buff, BUFFER_SIZE - 1, stdin);

    if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
    }

    for (int i = 0; i < 3; i++) {
        memset (buff, '\0', BUFFER_SIZE);
        fgets (buff, BUFFER_SIZE - 1, stdin);

        if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
        }
        if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
        }

        if (strcmp (buff, "right") == 0) {
            break;
        }
        if (i == 2) {
            return;
        }
    }

    memset (buff, '\0', BUFFER_SIZE);
    fgets (buff, BUFFER_SIZE - 1, stdin);

    if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
    }
    if (strcmp (buff, "add\n") == 0) {
        reciept_flag = add_func (sock);
    }
    else {
        reciept_flag = take_func (sock);
    }

    if (reciept_flag == 1) {
        memset (buff, '\0', BUFFER_SIZE);
        fgets (buff, BUFFER_SIZE - 1, stdin);

        if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
        }
    }

    memset (buff, '\0', BUFFER_SIZE);
    fgets (buff, BUFFER_SIZE - 1, stdin);

    if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    flag_work = 0;
}

int add_func (int sock) {
    char buff[BUFFER_SIZE];
    memset (buff, '\0', BUFFER_SIZE);

    while (1) {
        fgets (buff, BUFFER_SIZE - 1, stdin);

        if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed send");
            exit (EXIT_FAILURE);
        }

        //puts (buff);

        if (strcmp (buff, "end") == 0) {
            break;
        }
        memset (buff, '\0', BUFFER_SIZE);
    }

    return 1;
}

int take_func (int sock) {
    return 1;
}