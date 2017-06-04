#include "tcp_server.h"

#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "string.h"
#include "../fatfs2/ff.h"

//ICACHE_FLASH_ATTR 
xSemaphoreHandle  sentFlagSemaphore;
xQueueHandle sendQueue;

static struct espconn espconn_struct;
static esp_tcp tcp;
LOCAL uint16_t server_timeover = 300;//60*60*12; // yes. 12h timeout. so what? :)

typedef enum request {GET=0, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE} request_t;
char* request_list[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "NaN" };


typedef enum status {_200=0, _404, _403} status_t;
char const * const status_list[] = {"200 OK", " 404 Not Found", "403 Forbidden"};

typedef enum con_type {HTML=0, CSS, JAVASCRIPT, PNG, XML, JPG, BMP, PLAIN} con_type_t;

char const * const mime_str[] = {
	"text/html",
	"text/css",
	"text/javascript",
	"image/png",
	"text/xml",
	"image/jpeg",
	"image/bmp",
	"text/plain"
};

char const * const f_types[] = {
	"html",
	"css",
	"js",
	"png",
	"xml",
	"jpeg"
};

static char const * const request_content = {
	"HTTP/1.1 %s\r\n"
	"Content-Length: %d\r\n"
	"Content-Type: %s\r\n"
	"Connection: Close\r\n\r\n\0"
};


IRAM_ATTR con_type_t get_mime(char *file_name)
{
	char *ftype = strrchr(file_name, '.');
	ftype++;
	printf("GET MIME: %s\n", ftype);
	uint8_t index;
	for(index=0; index<(sizeof(f_types)/sizeof(f_types[0])); index++){
		if(!memcmp(f_types[index], ftype, strlen(f_types[index]))){
			return (con_type_t) index;
		}
	}
	printf("GET MIME index: %d\n", index);
	return PLAIN;
}

IRAM_ATTR void sender_thread(void *args)
{
	multi_args_t *multiarg = (multi_args_t*)args;
	xQueueHandle *squeue = (xQueueHandle*) multiarg->arg1;
	xSemaphoreHandle *ssemaphore = (xSemaphoreHandle*) multiarg->arg2;
	queue_struct_t data;
	printf("SENDER THREAD started\n");
	while(1)//mutex
	{
		
		if(pdTRUE == xQueueReceive(*squeue, &data, 10))
		{
			printf("\nSender_thread: getting from queue\n");
			while(pdFALSE == xSemaphoreTake(*ssemaphore, 10)); // zamienic na infinite

			{
				printf("Sender_thread: sending data\n");
				printf("Sender_thread: espconn_send return %d",espconn_send(data.espconn, data.data, data.size));
			}
			//xSemaphoreGive(sentFlagSemaphore); callback give it back
		}
	}
}


IRAM_ATTR void send_header(struct espconn *conn, status_t stat, con_type_t type, unsigned long length)
{
	queue_struct_t qstruct;
	qstruct.espconn = conn;
	sprintf(qstruct.data, request_content, status_list[stat], length, mime_str[type]);
	qstruct.size = strlen(qstruct.data);
	xQueueSend(sendQueue, &qstruct, portMAX_DELAY);
}

IRAM_ATTR void send_data(struct espconn *conn, uint8_t *data, uint8_t size)
{
	queue_struct_t qstruct;
	qstruct.espconn = conn;
	memcpy((void*)qstruct.data, data, size);
	qstruct.size = size;
	xQueueSend(sendQueue, &qstruct, portMAX_DELAY);
}

char readbuf[100];
FIL fd;
IRAM_ATTR int8_t send_file(struct espconn *conn, char *file_name){

	
	
	printf("\nTry to send file: %s", file_name);
    if (FR_OK != f_open(&fd, file_name, FA_READ)){
		printf("[ERROR] - cannot open file\n");
		return -1;
	}

	//send_header(conn, _200, get_mime(file_name), (unsigned long)f_size(&fd));
	printf("\nFile opened, size: %d",f_size(&fd));
    size_t readed=0;
    // Read file
	//start file sending task instead -> this above cause watchdog restart
	do {
		//if (FR_OK != (f_read(&fd, &readbuf[0], 100, &readed))) // 100 = readbuf size
		//	return -2; // break and goto f_close()
		if(readed <= 0) break;
		//send_data(conn, &readbuf[0], readed);
		printf("\nSend file: %s", readbuf);
		printf("\nSend file: %d", readed);
	} while(1);

    // Close file
	printf("\n File closed: %d\n",f_close(&fd));
    return 0;
}

char fname[50];
IRAM_ATTR static void data_recv_callback(void *arg, char *pdata, unsigned short len)
{
	//arg contains pointer to espconn struct
	struct espconn *pespconn = (struct espconn *) arg;

	printf("Received data: \"%s\"\n Length: %d\n", pdata, len);
	
	char* chr;
	
	chr = strchr(pdata, ' ');
	
	uint8_t request_str_len = chr-pdata;

	uint8_t i;
	for(i=0; i<sizeof(request_list)/sizeof(request_list[0]); i++){
		if(!strncmp(request_list[i], &pdata[0], request_str_len))
			break;
	}
	
	unsigned int name_len = (strchr(chr+1, ' ') - pdata - request_str_len - 1);
	//char *test = "\n<h1>TEST</h1>";
	switch(i){
		case GET:
			memcpy(&fname[0], chr+1, name_len);
			fname[name_len+1] = '\0';
			printf("\nNAME:%s : %d\n", &fname[0], name_len);

			send_file(pespconn, &fname[0]);
			/*if(strncmp("/favico",&fname[0], 7)){
				send_header(pespconn, _200, HTML, 13);
				//printf("Ret sent: %d\n",espconn_send(pespconn, test, 13));
				send_data(pespconn, test, 13);
				printf("\nPoszlo\n");
			}*/
				
			
		break;
		
		default:
			printf("\nRecv Callback: sending default\n");
			///////////////send_header(pespconn, _200, PLAIN, 0);
			break;
	}
	
}

IRAM_ATTR static void data_sent_callback(void *arg)
{
	printf("\nSend Callback: Data sent\n");
	if(pdTRUE == xSemaphoreGive(sentFlagSemaphore)){
		printf("\nSend Callback: semaphore released\n");
	}
	printf("\nSend Callback: cannot release semaphore\n");
}

static void connect_callback(void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;
	printf("TCP connection established\n");
	espconn_regist_recvcb (pespconn, data_recv_callback);

    // espconn_regist_reconcb(pespconn, tcpserver_recon_cb);
    //espconn_regist_disconcb(pespconn, shell_tcp_disconcb);
    espconn_regist_sentcb(pespconn, data_sent_callback);
}

/*static void reconnect_callback(void *arg, sint8 er)
{
	connect_callback(arg);
}*/


sint8 start_server(void)
{
	tcp.local_port = /*lwip_htons*/(80);
	espconn_struct.type = ESPCONN_TCP;
	espconn_struct.state = ESPCONN_NONE;
	espconn_struct.proto.tcp = &tcp;
	//espconn_struct.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    //espconn_struct.proto.tcp->local_port = 80;


	if(espconn_regist_connectcb (&espconn_struct, connect_callback)) return (sint8)(-1);
	//espconn_regist_reconcb(&espconn_struct, reconnect_callback);
	
	espconn_set_opt(&espconn_struct, ESPCONN_REUSEADDR | ESPCONN_NODELAY);
	//espconn_init();
	if(espconn_accept(&espconn_struct)) return -2;
	if(espconn_regist_time(&espconn_struct, server_timeover, 0)) return -3;
	//if(espconn_tcp_set_max_con(1)) return (sint8)(-3);

	return (sint8)0;
}
