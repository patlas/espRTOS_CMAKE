#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "esp_common.h"
#include "espressif/c_types.h"
#include "lwip/stats.h"
#include "espconn.h"

sint8 start_server(void);
void sender_thread(void *args);

typedef struct queue_struct_t{
    struct espconn *espconn;
	uint8_t data[100];
	uint8_t size;
} queue_struct_t;

typedef struct multi_args_t{
    void *arg1;
    void *arg2;
} multi_args_t;

#endif
