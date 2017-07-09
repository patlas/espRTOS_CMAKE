#include "tcp_server.h"

#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "string.h"
#include "../fatfs2/ff.h"

//ICACHE_FLASH_ATTR 


static struct espconn espconn_struct;
static esp_tcp tcp;

#define FILE_BUF_SIZE 100
char readbuf[FILE_BUF_SIZE];
FIL fd;
extern xSemaphoreHandle fs_semaphore;

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

con_type_t get_mime(char *file_name);
static void file_sender_thread(void *args);
int8_t send_header(struct espconn *conn, status_t stat, con_type_t type, unsigned long length);
int8 send_data(struct espconn *conn, uint8_t *data, uint8_t size);
static void data_recv_callback(void *arg, char *pdata, unsigned short len);
static void data_sent_callback(void *arg);
static void reconnect_callback(void *arg, sint8 er);
static void disconnect_callback(void *arg);
static void connect_callback(void *arg);


/*IRAM_ATTR*/ con_type_t get_mime(char *file_name)
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



/*IRAM_ATTR*/ static void file_sender_thread(void *args)
{
	/****
	 * multi_args_t.arg1 -> file name
	 * multi_args_t.arg2 -> esp_conn struct
	 ***/

	multi_args_t *multiarg = (multi_args_t*)args;
	char *file_name = (char*) multiarg->arg1;
	struct espconn *conn = (struct espconn*) multiarg->arg2;

	queue_struct_t data;
	printf("SENDER THREAD started\n");


	printf("\nTry to send file: %s", file_name);
	if (FR_OK != f_open(&fd, file_name, FA_READ)){
		printf("[ERROR] - cannot open file\n");
		vTaskDelete(NULL);
	}

	send_header(conn, _200, get_mime(file_name), (unsigned long)f_size(&fd));
	printf("\nFile opened, size: %d", f_size(&fd));
	size_t readed=0;
//	Read file
//	start file sending task instead -> this above cause watchdog restart
	do {
		if (FR_OK != (f_read(&fd, &readbuf[0], FILE_BUF_SIZE, &readed))) // 100 = readbuf size
		{
			printf("[ERROR] - reading failed\n");
			f_close(&fd);
			vTaskDelete(NULL);
		}
		if(readed <= 0) break;

//		wait for previous data being send
		while(pdFALSE == xSemaphoreTake(fs_semaphore, 10)); // zamienic na infinite
		if(0 > send_data(conn, &readbuf[0], readed))
		{
			printf("[ERROR] - sending failed\n");
			f_close(&fd);
			xSemaphoreGive(fs_semaphore);
			vTaskDelete(NULL);
		}
//		printf("\nSend file: %s", readbuf);
//		printf("\nSend file: %d", readed);
	} while(1);

	// Close file
	printf("\n File closed: %d\n",f_close(&fd));
	xSemaphoreGive(fs_semaphore);
	vTaskDelete(NULL);
}




/*IRAM_ATTR*/ int8_t send_header(struct espconn *conn, status_t stat, con_type_t type, unsigned long length)
{
//	readbuf may be to small -> in that case create new one
	sprintf(&readbuf[0], request_content, status_list[stat], length, mime_str[type]);
	return send_data(conn, &readbuf[0], strlen(&readbuf[0]));

}



/*IRAM_ATTR*/ int8 send_data(struct espconn *conn, uint8_t *data, uint8_t size)
{
	int8_t ret = 0;
	ret = espconn_send(conn, data, size);
	if(0 > ret)
		printf("SENDING error num: %d", ret);
	return ret;
}




char fname[50];
multi_args_t file_sender_multi;
char *test = "\n<h1>TEST</h1>";


/*IRAM_ATTR*/ static void data_recv_callback(void *arg, char *pdata, unsigned short len)
{
	//arg contains pointer to espconn struct
	struct espconn *pespconn = (struct espconn *) arg;
	memset(fname, 0, 50);

	printf("Receive callback\nReceived data: \"%s\"\n Length: %d\n", pdata, len);
	
	char* chr;
	
	chr = strchr(pdata, ' ');
	
	uint8_t request_str_len = chr-pdata;

	uint8_t i;
	for(i=0; i<sizeof(request_list)/sizeof(request_list[0]); i++){
		if(!strncmp(request_list[i], &pdata[0], request_str_len))
			break;
	}
	
	unsigned int name_len = (strchr(chr+1, ' ') - pdata - request_str_len - 1);

	switch(i){
		case GET:
			memcpy(&fname[0], chr+1, name_len);
			fname[name_len+1] = '\0';
			printf("\nNAME:%s : %d\n", &fname[0], name_len);
			if(fname[1] != '\0' && name_len > 2)
			{
				file_sender_multi.arg1 = &fname[0];
				file_sender_multi.arg2 = pespconn;
//				if(strncmp("/favico",&fname[0], 7));
				if(pdPASS == xTaskCreate(file_sender_thread, "sender", 512, &file_sender_multi, 2, NULL))
				{
					printf("File sender task created");
				}
				else
				{
					printf("[ERROR] - sender task creation\n");
				}
			}
			else
			{
				printf("[ERROR] - wrong file name\n");
			}

		break;
		
		default:
			printf("\nRecv Callback: sending default\n");
//			POST for json purpose
			break;
	}
	
}

/*IRAM_ATTR*/ static void data_sent_callback(void *arg)
{
	printf("\nSent Callbackt\n");
	if(pdTRUE == xSemaphoreGive(fs_semaphore))
	{
		printf("\nSend Callback: semaphore released\n");
		return;
	}
	printf("\nSend Callback: cannot release semaphore\n");
}



static void reconnect_callback(void *arg, sint8 er)
{
	//connect_callback(arg);
	printf("Reconnect callback\n");
}



static void disconnect_callback(void *arg)
{
	printf("Disconnct callback");
}



static void connect_callback(void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;
	printf("Connect callback: TCP connection established\n");
	espconn_regist_recvcb (pespconn, data_recv_callback);

    espconn_regist_reconcb(pespconn, reconnect_callback);
    espconn_regist_disconcb(pespconn, disconnect_callback);
    espconn_regist_sentcb(pespconn, data_sent_callback);
}




sint8 start_server(void)
{
	tcp.local_port = /*lwip_htons*/(80);
	espconn_struct.type = ESPCONN_TCP;
	espconn_struct.state = ESPCONN_NONE;
	espconn_struct.proto.tcp = &tcp;
	//espconn_struct.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    //espconn_struct.proto.tcp->local_port = 80;


	if(espconn_regist_connectcb (&espconn_struct, connect_callback)) return (sint8)(-1);
	
	espconn_set_opt(&espconn_struct, ESPCONN_REUSEADDR | ESPCONN_NODELAY/* | ESPCONN_KEEPALIVE*/);
	if(espconn_accept(&espconn_struct)) return -2;
	if(espconn_regist_time(&espconn_struct, server_timeover, 0)) return -3;
	//if(espconn_tcp_set_max_con(1)) return (sint8)(-3);

	return (sint8)0;
}
