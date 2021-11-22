/*
* @file server.c
* @author Renat Kagal <kagal@itspartner.net>
*
* Assembling : gcc -Wall server.c -pthread -o server
*
* Description :  bank machine
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

enum states {
    st_begin_work = 0,
    st_waiting_for_card,
    st_check_card,
    st_give_back_card,
    st_choose_language,
    st_enter_pass,
    st_block_card,
    st_wait_action,
    st_request_for_receipt,
    st_request_for_continuation,
    st_end_work
};

enum action_states {
    st_add_money = 0,
    st_wait_for_bill,
    st_check_bill,
    st_give_back_bill,
    st_add_bill_to_account,
   
    st_take_money,
    st_wait_money_count,
    st_send_request_for_take_money,
    st_decript_data,

    st_end,
};

enum signals{
    true_signal = 0,
    false_signal,
};

typedef void (*func_ptr)(enum signals* signal, int sock);

struct transition {
    enum states new_state;
    func_ptr function1;
    func_ptr function2;
};

struct transition_action {
    enum action_states new_state;
    func_ptr function1;
    func_ptr function2;
};

#include "head_server.h"

#define BUFFER_SIZE 32

#define PORT 2000

struct transition sm_table [10][2] = {
    [st_begin_work][true_signal] = {st_waiting_for_card, waiting_for_card, NULL},
    [st_begin_work][false_signal] = {st_end_work, end_work, NULL},

    [st_waiting_for_card][true_signal] = {st_check_card, check_card, NULL},
    [st_waiting_for_card][false_signal] = {st_end_work, NULL, NULL},
    
    [st_check_card][true_signal] = {st_choose_language, choose_language, NULL},
    [st_check_card][false_signal] = {st_give_back_card, give_back_card, NULL},
    
    [st_give_back_card][true_signal] = {st_waiting_for_card, waiting_for_card, NULL},
    [st_give_back_card][false_signal] = {st_end_work, NULL, NULL},   

    [st_choose_language][true_signal] = {st_enter_pass, enter_pass, NULL},
    [st_choose_language][false_signal] = {st_end_work, NULL, NULL},

    [st_enter_pass][true_signal] = {st_wait_action, wait_action, NULL},
    [st_enter_pass][false_signal] = {st_block_card, block_card, NULL},

    [st_block_card][true_signal] = {st_give_back_card, give_back_card, send_data_block_card},
    [st_block_card][false_signal] = {st_end_work, NULL, NULL},

    [st_wait_action][true_signal] = {st_request_for_receipt, request_for_receipt, NULL},
    [st_wait_action][false_signal] = {st_request_for_continuation, request_for_continuation, NULL},

    [st_request_for_receipt][true_signal] = {st_request_for_continuation, request_for_continuation, NULL},
    [st_request_for_receipt][false_signal] = {st_end_work, NULL, NULL},

    [st_request_for_continuation][true_signal] = {st_enter_pass, enter_pass, NULL},
    [st_request_for_continuation][false_signal] = {st_give_back_card, give_back_card, NULL},
};

struct transition_action sm_action_table [9][2] = {
    [st_add_money][true_signal] = {st_wait_for_bill, wait_for_bill, NULL},
    [st_add_money][false_signal] = {st_end, NULL, NULL},

    [st_wait_for_bill][true_signal] = {st_check_bill, check_bill, NULL},
    [st_wait_for_bill][false_signal] = {st_end, end, send_request_to_bank},

    [st_check_bill][true_signal] = {st_add_bill_to_account, add_bill_to_account, NULL},
    [st_check_bill][false_signal] = {st_give_back_bill, give_back_bill, NULL},

    [st_give_back_bill][true_signal] = {st_wait_for_bill, wait_for_bill, NULL},
    [st_give_back_bill][false_signal] = {st_end, NULL, NULL},

    [st_add_bill_to_account][true_signal] = {st_wait_for_bill, wait_for_bill, NULL},
    [st_add_bill_to_account][false_signal] = {st_end, end, NULL},

    //take money

    [st_take_money][true_signal] = {st_wait_money_count, wait_money_count, NULL},
    [st_take_money][false_signal] = {st_end, NULL, NULL},

    [st_wait_money_count][true_signal] = {st_send_request_for_take_money, send_request_for_take_money, NULL},
    [st_wait_money_count][false_signal] = {st_end, NULL, NULL},

    [st_send_request_for_take_money][true_signal] = {st_decript_data, decript_data, NULL},
    [st_send_request_for_take_money][false_signal] = {st_end, NULL, NULL},

    [st_decript_data][true_signal] = {st_end, end, NULL},
    [st_decript_data][false_signal] = {st_end, NULL, NULL},
};

int     electricity = 1;
int     sum;
char    card[BUFFER_SIZE];
char    language[BUFFER_SIZE];
char    bill[BUFFER_SIZE];

char keystoke = 0;

int main () {

    int listen_sock;
    struct sockaddr_in addr;

    if ((listen_sock = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        puts ("Failed create socket");
        exit (EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons (PORT);
    addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

    if (bind (listen_sock, (struct sockaddr*) &addr, sizeof (addr)) < 0) {
        puts ("Failed bind sock");
        exit (EXIT_FAILURE);
    }
    listen (listen_sock, 1);
    
    pthread_t thr;
    
    struct thr_listen_sock* data = (struct thr_listen_sock*)malloc (sizeof (struct thr_listen_sock));
    
    data->listen_sock = listen_sock;


    pthread_create (&thr, NULL, server_accept, (void*) data);

    while (scanf("%c", &keystoke)) {
        if (keystoke == 'p')
            break;
    }

    if (pthread_join (thr, NULL)) {
        printf ("Failed join thread");
        exit (EXIT_FAILURE);
    }

    close (listen_sock);
}

void bank_machine (struct thr_data* data) {
    char buff[BUFFER_SIZE];

    int cond_recv = recv (data->sock, buff, BUFFER_SIZE, 0);

    if (cond_recv < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    if (cond_recv == 0) {
        free (data);
        return;
    }

    if (strcmp (buff, "bank") == 0) {
        bank (data->sock);
    }

    free (data);
}

void bank (int sock) {
    enum            signals signal = true_signal;
    enum            states state = st_begin_work;
    func_ptr        work1;
    func_ptr        work2;

    begin_work (&signal, sock);

    while (state != st_end_work) {
        work1 = sm_table[state][signal].function1;
        work2 = sm_table[state][signal].function2;

        state = sm_table[state][signal].new_state;

        if (work1 != NULL) {
            work1 (&signal, sock);
        }
        if (work2 != NULL) {
            work2 (&signal, sock);
        }
    }
}

void end_work (enum signals* signal, int sock) {
    puts ("end_work");
}

void end (enum signals* signal, int sock) {
    //puts ("end");
}

void decript_data (enum signals* signal, int sock) {
    //puts ("decript_data");

    *signal = true_signal;
}

void send_request_for_take_money (enum signals* signal, int sock) {
   //puts ("send_request_for_take_money");
   
   int new_sum = blowfish (sum, 2);

    if (new_sum == -1) {
        if (strcmp (language, "ru\n") == 0) {
            puts ("Нельзя снять деньги");
        }
        else if (strcmp (language, "en\n") == 0){
            puts ("you cannot withdraw money");
        }
        *signal = true_signal;
        return;
    }

    if (strcmp (language, "ru\n") == 0) {
        printf ("На счете осталось : %d\n", new_sum);
    }
    else if (strcmp (language, "en\n") == 0){
        printf ("Your account has : %d\n", new_sum);
    }
    send_right (sock);

    *signal = true_signal; 
}

void wait_money_count (enum signals* signal, int sock) {
    char buff[BUFFER_SIZE];
    memset (buff, '\0', BUFFER_SIZE);

    //puts ("wait_money_count");

    if (strcmp (language, "ru\n") == 0) {
        puts ("Введите сумму которую хотете снять");
    }
    else if (strcmp (language, "en\n") == 0){
        puts ("Enter the amount you want to withdraw");
    }

    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    sum = atoi (buff);

    *signal = true_signal;
}

void take_money (enum signals* signal, int sock) {
    puts ("take_money");

    *signal = true_signal;
}

void send_request_to_bank (enum signals* signal, int sock) {
    //puts ("send_request_to_bank");

    int new_sum = blowfish (sum, 1);

    if (strcmp (language, "ru\n") == 0) {
        printf ("Счет = %d\n", new_sum);
    }
    else if (strcmp (language, "en\n") == 0) {
        printf ("New sum = %d\n", new_sum);
    }

    *signal = true_signal;
}

void add_bill_to_account (enum signals* signal, int sock) {
    //puts ("add_bill_to_account");
    sum += atoi (bill);

    if (strcmp (language, "ru\n") == 0) {
        printf ("Добавляем к счету сумму = %d\n", sum);
    }
    else if (strcmp (language, "en\n") == 0) {
        printf ("add sum = %d\n", sum);
    }

    *signal = true_signal;
}

void give_back_bill (enum signals* signal, int sock) {
    //puts ("give_back_bill");

    if (strcmp (language, "ru\n") == 0) {
        puts ("Возврат купюры");
    }
    else if (strcmp (language, "en\n") == 0){
        puts ("Give back bill");
    }

    *signal = true_signal;
}

void check_bill (enum signals* signal, int sock) {
    //puts ("check_bill");

    if (strcmp (bill, "1\n") == 0 || strcmp (bill, "10\n") == 0 || strcmp (bill, "5\n") == 0) {
        *signal = true_signal;
        return;
    }
    *signal = false_signal;
}

void wait_for_bill (enum signals* signal, int sock) {
    char buff[BUFFER_SIZE];
    memset (buff, '\0', BUFFER_SIZE);

    //puts ("wait_for_bill");

    if (strcmp (language, "ru\n") == 0) {
        puts ("Ожидание купюры");
    }
    else if (strcmp (language, "en\n") == 0){
        puts ("Wait bill");
    }

    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    send_right (sock);

    if (strcmp (buff, "end\n") == 0) {
        *signal = false_signal;
        return;
    }
    memset (bill, '\0', BUFFER_SIZE);

    strcat (bill, buff);
    *signal = true_signal;
}

void add_money (enum signals* signal, int sock) {
    puts ("add_money");
    sum = 0;

    *signal = true_signal;
}

void request_for_continuation (enum signals* signal, int sock) {
    char buff[BUFFER_SIZE];
    memset (buff, '\0', BUFFER_SIZE);

    //puts ("request_for_continuation");

    if (strcmp (language, "ru\n") == 0) {
        puts ("Желаете продолжить");
    }
    else if (strcmp (language, "en\n") == 0){
        puts ("Woudle you like continue");
    }

    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    send_right (sock);

    if (strcmp (buff, "+\n") == 0) {
        *signal = true_signal;
        return;
    }
    *signal = false_signal;
}

void request_for_receipt (enum signals* signal, int sock) {
    char buff[BUFFER_SIZE];
    memset (buff, '\0', BUFFER_SIZE);

    //puts ("request_for_receipt");

    if (strcmp (language, "ru\n") == 0) {
        puts ("Печать чек?");
    }
    else if (strcmp (language, "en\n") == 0){
        puts ("Print receipt?");
    }

    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }

    if (strcmp (buff, "+\n") == 0) {
        if (strcmp (language, "ru\n") == 0) {
            puts ("Чек");
        }
        else if (strcmp (language, "en\n") == 0){
            puts ("Receipt");
        }
    }
    send_right (sock);

    *signal = true_signal;
}

void wait_action (enum signals* signal, int sock) {
    char            buff[BUFFER_SIZE];
    enum            signals signal_action = true_signal;
    enum            action_states state;
    func_ptr        work1;
    func_ptr        work2;
    
    if (strcmp (language, "ru\n") == 0) {
        puts ("Ожидание действия");
    }
    else if (strcmp (language, "en\n") == 0){
        puts ("Wait action");
    }
    memset (buff, '\0', BUFFER_SIZE);

    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }

    if (strcmp (buff, "add\n") == 0) {
        state = st_add_money;
        add_money (&signal_action, sock);
    }
    else if(strcmp (buff, "take\n") == 0){
        state = st_take_money;
        take_money (&signal_action, sock);
    }
    else {
        state = st_end;
    }
    send_right (sock);

    while (state != st_end) {
        work1 = sm_action_table[state][signal_action].function1;
        work2 = sm_action_table[state][signal_action].function2;

        state = sm_action_table[state][signal_action].new_state;

        if (work1 != NULL) {
            work1 (&signal_action, sock);
        }
        if (work2 != NULL) {
            work2 (&signal_action, sock);
        }
    }

    *signal = true_signal;
}

void send_data_block_card (enum signals* signal, int sock) {
    //puts ("send_data_block_card");
}

void block_card (enum signals* signal, int sock) {
    //puts ("block_card");

    if (strcmp (language, "ru\n") == 0) {
        puts ("Ваша карта заблокиравана");
    }
    else if (strcmp (language, "en\n") == 0){
        puts ("Your card is blocked");
    }
    *signal = true_signal;
}

void enter_pass (enum signals* signal, int sock) {
    //puts ("enter_pass");

    char buff[BUFFER_SIZE];
    char pass[BUFFER_SIZE] = "123\n";

    for (int i = 0; i < 3; i++) {
        if (strcmp (language, "ru\n") == 0) {
            puts ("Введите пароль");
        }
        else if (strcmp (language, "en\n") == 0){
            puts ("Enter password");
        }

        memset (buff, '\0', BUFFER_SIZE);
        if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
        }

        if (strcmp (pass, buff) == 0) {
            if (strcmp (language, "ru\n") == 0) {
                puts ("Правильный пароль");
            }
            else if (strcmp (language, "en\n") == 0){
                puts ("Right password");
            }

            *signal = true_signal;
            send_right (sock);
            return ;
        }
        else  {
            if (strcmp (language, "ru\n") == 0) {
                puts ("Не правильный пароль");
            }
            else if (strcmp (language, "en\n") == 0){
                puts ("bad password");
            }
        }
        send_right (sock);
    }
    memset (buff, '\0', BUFFER_SIZE);
    strcat (buff, "false");

    if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed send");
        exit (EXIT_FAILURE);
    }
    *signal = false_signal;
}

void choose_language (enum signals* signal, int sock) {
    puts ("choose language");

    char buff[BUFFER_SIZE];

    memset (buff, '\0', BUFFER_SIZE);
    memset (language, '\0', BUFFER_SIZE);

    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    if (strcmp (buff, "ru\n") == 0 || strcmp (buff, "en\n") == 0) {
        strcat (language, buff);
        *signal = true_signal;
    }
    send_right (sock);
}

void give_back_card (enum signals* signal, int sock) {
    //puts ("give_back_card");

    memset (card, '\0', BUFFER_SIZE);        ///////////////
    puts ("take card");
    puts ("Заберите карту");

    *signal = true_signal;
}

void check_card (enum signals* signal, int sock) {
    //puts ("check_card");
    char buff[BUFFER_SIZE];

    if (strcmp (card, "bank\n") == 0) {
        send_right (sock);

        *signal = true_signal;
        return;
    }
    memset (buff, '\0', BUFFER_SIZE);
    strcat (buff, "bad");

    if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed send");
            exit (EXIT_FAILURE);
    }

    *signal = false_signal;
}

void waiting_for_card (enum signals* signal, int sock) {
    //puts ("waiting_for_card");

    char buff[BUFFER_SIZE];
   
    memset (buff, '\0', BUFFER_SIZE);
    memset (card, '\0', BUFFER_SIZE);

    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    strcat (card, buff);

    send_right (sock);

    *signal = true_signal;
}

void begin_work (enum signals* signal, int sock) {
    //puts ("begin_work");

    if (electricity == 1) {
        *signal = true_signal;
        return;
    }
    *signal = false_signal;
}


void server_accept (struct thr_listen_sock* data_listen) {
    struct thr_node* thr_top = NULL;
    struct thr_data* data = NULL;
    struct thr_node* tmp;

    int sock;

    while (keystoke != 'p') {
        if ((sock = accept (data_listen->listen_sock, NULL, NULL)) < 0) {
            puts ("Failed accept connection");
            exit (EXIT_FAILURE);
        }
        data = (struct thr_data*)malloc (sizeof (struct thr_data));

        if (data == NULL) {
            puts ("Failed alloc memory data");
            exit (EXIT_FAILURE);
        }
        data->sock = sock;

        create_thread (&thr_top, data);       
    }

    while (thr_top != NULL) {
        if (pthread_join (thr_top->thread, NULL)) {
            puts ("Failed join thread");
            exit (EXIT_FAILURE);
        }
        tmp = thr_top;
        thr_top = thr_top->next;
        free (tmp);
    }
}

void create_thread(struct thr_node** thr_top, struct thr_data* data) {
    if (*thr_top == NULL) {
        (*thr_top) = (struct thr_node*)malloc (sizeof (struct thr_node));
        
        if ((*thr_top) == NULL) {
            puts ("Failed alloc memory for thr_top");
            exit (EXIT_FAILURE);
        }
        (*thr_top)->id = 1;
        (*thr_top)->next = NULL;
        data->id = (*thr_top)->id;

        if (pthread_create (&((*thr_top)->thread), NULL, bank_machine , data)) {
            puts ("Failed create thread top");
            exit (EXIT_FAILURE);
        }
    }
    else {
        struct thr_node* tmp = (struct thr_node*)malloc (sizeof (struct thr_node));
        
        if (tmp == NULL) {
            puts ("Failed alloc memory for tmp_thread");
            exit (EXIT_FAILURE);
        }
        struct thr_node* tmp2 = (*thr_top);

        while (tmp2->next != NULL)
             tmp2 = tmp2->next;
        tmp2->next = tmp;

        tmp->id = tmp2->id + 1;
        data->id = tmp->id;

        tmp->next = NULL;
        
        if (pthread_create (&(tmp->thread), NULL, bank_machine , data)) {
            puts ("Failed create thread");
            exit (EXIT_FAILURE);
        }
    }
}

void send_right (int sock) {
    char buff[BUFFER_SIZE] = "right";

    if (send(sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed send");
        exit (EXIT_FAILURE);
    }
}