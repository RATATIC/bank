#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

struct thr_listen_sock {
	int listen_sock;
};

struct thr_data {
	int id;
	int sock;
};

struct thr_node {
    int id;
    pthread_t thread;
    struct thr_node* next;
};



void create_thread(struct thr_node** thr_top, struct thr_data* data);
void server_accept (struct thr_listen_sock* data_listen);

void bank (int sock);

void begin_work (enum signals* signal, int sock);
void end_work (enum signals* signal, int sock);
void waiting_for_card (enum signals* signal, int sock);
void check_card (enum signals* signal, int sock);
void give_back_card (enum signals* signal, int sock);
void choose_language (enum signals* signal, int sock);
void enter_pass (enum signals* signal, int sock);
void block_card (enum signals* signal, int sock);
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
void encript_data_for_add_money (enum signals* signal, int sock);

void take_money (enum signals* signal, int sock);
void wait_money_count (enum signals* signal, int sock);
void send_request_for_take_money (enum signals* signal, int sock);
void encript_data_for_take_money (enum signals* signal, int sock);
void decript_data (enum signals* signal, int sock);

void end (enum signals* signal, int sock);

int blowfish (int add_sum, int enc_or_dec);
