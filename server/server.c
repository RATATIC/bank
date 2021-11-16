/*
* @file server.c
* @author Renat Kagal <kagal@itspartner.net>
*
* Assembling : gcc -Wall server.c -pthread -o server
*
* Description : coffee machine
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

#include "head_server.h"

#define BUFFER_SIZE 32

#define PORT 2000


enum states {
    st_begin_work = 0,
    st_waiting_for_card,
    st_check_card,
    st_give_back_card,
    st_choose_language,
    st_enter_pass,
    st_block_card,
    st_give_back_card,
    st_send_data_block_card,
    st_encript_data,
    st_wait_action,
    st_request_for_receipt,
    st_request_for_continuation,
    st_end_work
};

enum action_state {
	st_add_money = 0,
	st_wait_for_bill,
	st_check_bill,
	st_give_back_bill,
	st_add_bill_to_account,
	st_send_request_to_bank,

	st_take_money,
	st_wait_money_count,
	st_send_request_for_take_money,
	st_encript_data_for_take_money,
	st_decript_data,
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

void begin_work (enum signals* signal, int sock);
void end_work (enum signals* signal, int sock);
void waiting_for_card (enum signals* signal, int sock);
void check_card (enum signals* signal, int sock);
void give_back_card (enum signals* signal, int sock);
void choose_language (enum signals* signal, int sock);
void enter_pass (enum signals* signal, int sock);
void block_card (enum signals* signal, int sock);
void give_back_card (enum signals* signal, int sock);
void send_data_block_card (enum signals* signal, int sock);
void encript_data (enum signals* signal, int sock);
void wait_action (enum signals* signal, int sock);
void request_for_continuation (enum signals* signal, int sock);
void request_for_receipt (enum signals* signal, int sock);

void add_money (enum signals* signal, int sock);
void wait_for_bill (enum signals* signal, int sock);
void check_bill (enum signals* signal, int sock);
void give_back_bill (enum signals* signal, int sock);
void add_bill_to_account (enum signals* signal, int sock);
void send_request_to_bank (enum signals* signal, int sock);

void take_money (enum signals* signal, int sock);
void wait_money_count (enum signals* signal, int sock);
void send_request_for_take_money (enum signals* signal, int sock);
void encript_data_for_take_money (enum signals* signal, int sock);
void decript_data (enum signals* signal, int sock);

struct transition sm_table [7][2] = {
	[st_begin_work][true_signal] = {st_waiting_for_card, waiting_for_card, NULL},
	[st_begin_work][false_signal] = {st_end_work, end_work, NULL},

	[st_waiting_for_card][true_signal] = {st_check_card, check_card, NULL},
	[st_waiting_for_card][false_signal] = {st_end_work, NULL, NULL},
	
	[st_check_card][true_signal] = {st_choose_language, choose_language, NULL},
	[st_check_card][false_signal] = {st_give_back_card, give_back_card, NULL},
	
	[st_give_back_card][true_signal] = {st_waiting_for_card, waiting_for_card, NULL},
	[st_give_back_card][false_signal] = {st_end_work NULL, NULL},	

	[st_choose_language][true_signal] = {st_enter_pass, enter_pass, NULL},
	[st_choose_language][false_signal] = {st_end_work, NULL, NULL},

	[st_enter_pass][true_signal] = {st_wait_action, wait_action, NULL},
	[st_enter_pass][false_signal] = {st_block_card, block_card, NULL},

	[st_block_card][true_signal] = {st_give_back_card, give_back_card, send_data_block_card},
	[st_block_card][false_signal] = {st_end_work, NULL, NULL},

	[st_give_back_card][true_signal] = {st_waiting_for_card, waiting_for_card, NULL},
	[st_give_back_card][false_signal] = {st_end_work, NULL, NULL},

	[st_send_data_block_card][true_signal] = {st_encript_data, encript_data, NULL},
	[st_send_data_block_card][false_signal] = {st_end_work, NULL, NULL},

	[st_encript_data][true_signal] = {st_end_work, end_work, NULL},
	[st_encript_data][false_signal] = {st_end_work, NULL, NULL},

	[st_wait_action][true_signal] = {st_request_for_receipt, request_for_receipt, NULL},
	[st_wait_action][false_signal] = {st_request_for_continuation, request_for_continuation, NULL},

	[st_request_for_receipt][true_signal] = {st_request_for_continuation, request_for_continuation, NULL},
	[st_request_for_receipt][false_signal] = {st_end_work, NULL, NULL},

	[st_request_for_continuation][true_signal] = {st_enter_pass, enter_pass, NULL},
	[st_request_for_continuation][false_signal] = {st_give_back_card, give_back_card, NULL},
};

int electic_eq = 1;
int water_level = 100;
int equip = 1;
int litter = 0;
int work_coffee = 1;

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

void coffee_machine(struct thr_data* data) {
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
		coffee (data->sock);
	}

	free (data);
}

void coffee (int sock) {
	enum signals signal = true_signal;
	enum states state = st_begin_work;

	func_ptr work;

	begin_work (&signal, sock);

	while (work_coffee) {
		work = sm_table[state][signal].function;
		state = sm_table[state][signal].new_state;

		if (work != NULL) {
			work (&signal, sock);
		}
	}
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

        if (pthread_create (&((*thr_top)->thread), NULL, coffee_machine, data)) {
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
        
        if (pthread_create (&(tmp->thread), NULL, coffee_machine, data)) {
            puts ("Failed create thread");
            exit (EXIT_FAILURE);
        }
    }
}