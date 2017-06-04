

#include "esp_common.h"
#include "esp_misc.h"
#include "espressif/esp_softap.h"
#include "espressif/esp_timer.h"
#include "../driver/gpio.h"
#include "../driver/spi.h"

#include "../fatfs2/ff.h"

#include "access_point.h"
#include "tcp_server.h"

#include "freertos/queue.h"
#include "freertos/semphr.h"

#define CS_GPIO_PIN 2
#define TEST_FILENAME "/test_loooong_filename.txt"
#define TEST_CONTENTS "Hello! It's FatFs on esp8266 with ESP Open RTOS!"
#define READBUF_SIZE 100
#define DELAY_MS 3000
#define MEMLEAK_DEBUG


// DISABLE PRINTF
//#define NO_DEBUG
void user_printf(char c) {}



//MISSING IN NEW RTOS API
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

/////////////////////////////////////



extern char readbuf[100];
extern xQueueHandle sendQueue;
extern xSemaphoreHandle  sentFlagSemaphore;


static const char contents[] = TEST_CONTENTS;

#if 0
static const char *results[] = {
    [FR_OK]                  = "Succeeded",
    [FR_DISK_ERR]            = "A hard error occurred in the low level disk I/O layer",
    [FR_INT_ERR]             = "Assertion failed",
    [FR_NOT_READY]           = "The physical drive cannot work",
    [FR_NO_FILE]             = "Could not find the file",
    [FR_NO_PATH]             = "Could not find the path",
    [FR_INVALID_NAME]        = "The path name format is invalid",
    [FR_DENIED]              = "Access denied due to prohibited access or directory full",
    [FR_EXIST]               = "Access denied due to prohibited access",
    [FR_INVALID_OBJECT]      = "The file/directory object is invalid",
    [FR_WRITE_PROTECTED]     = "The physical drive is write protected",
    [FR_INVALID_DRIVE]       = "The logical drive number is invalid",
    [FR_NOT_ENABLED]         = "The volume has no work area",
    [FR_NO_FILESYSTEM]       = "There is no valid FAT volume",
    [FR_MKFS_ABORTED]        = "The f_mkfs() aborted due to any problem",
    [FR_TIMEOUT]             = "Could not get a grant to access the volume within defined period",
    [FR_LOCKED]              = "The operation is rejected according to the file sharing policy",
    [FR_NOT_ENOUGH_CORE]     = "LFN working buffer could not be allocated",
    [FR_TOO_MANY_OPEN_FILES] = "Number of open files > _FS_LOCK",
    [FR_INVALID_PARAMETER]   = "Given parameter is invalid"
};



static bool failed(FRESULT res)
{
    bool fail = res != FR_OK;
    if (fail)
        printf("\n  Error: ");
    printf("\n  %s\n", results[res]);
    return fail;
}

void check_fatfs()
{
    const char *vol = "0:";//f_gpio_to_volume(CS_GPIO_PIN);//////////////

    printf("\nCreating test file\n----------------------------\n");

    FATFS fs;
    // Mount filesystem
    printf("f_mount(&fs, \"%s\")", vol);
    if (failed(f_mount(&fs, vol, 1)))
        return;

    // Set default drive
    printf("f_chdrive(\"%s\")", vol);
    if (failed(f_chdrive(vol)))
        return;

    FIL f;
    // Create test file
    printf("f_open(&f, \"%s\", FA_WRITE | FA_CREATE_ALWAYS)", TEST_FILENAME);
    if (failed(f_open(&f, TEST_FILENAME, FA_WRITE | FA_CREATE_ALWAYS)))
        return;

    size_t written;
    // Write test string
    printf("f_write(&f, \"%s\")", contents);
    if (failed(f_write(&f, contents, sizeof(contents) - 1, &written)))
        return;
    printf("  Bytes written: %d\n", written);

    // Close file
    printf("f_close(&f)");
    if (failed(f_close(&f)))
        return;

    printf("\nReading test file\n----------------------------\n");

    // Open test file
    printf("f_open(&f, \"%s\", FA_READ)", TEST_FILENAME);
    if (failed(f_open(&f, TEST_FILENAME, FA_READ)))
        return;

    printf("  File size: %u\n", (uint32_t)f_size(&f));

    size_t readed;
    // Read file
    printf("f_read(&f, ...)");
    if (failed(f_read(&f, readbuf, sizeof(readbuf) - 1, &readed)))
        return;
    readbuf[readed] = 0;

    printf("  Readed %u bytes, test file contents: %s\n", readed, readbuf);

    // Close file
    printf("f_close(&f)");
    if (failed(f_close(&f)))
        return;

    // Unmount
    printf("f_mount(NULL, \"%s\")", vol);
    if (failed(f_mount(NULL, vol, 0)))
        return;
}
#endif


LOCAL void wifi_event(System_Event_t *event) {
  switch (event->event_id) {
    case EVENT_SOFTAPMODE_STACONNECTED:
        printf("TCP server: %d\n", start_server());
    break;
    //default:
      //os_printf("WiFi Event: %d\n", event->event_id);
  }
}

multi_args_t multiarg;
void user_init(void)
{
    #ifdef NO_DEBUG
        os_install_putc1(user_printf);
    #endif

    sendQueue = xQueueCreate(100, sizeof(queue_struct_t));
    vSemaphoreCreateBinary(sentFlagSemaphore);

    //uart_set_baud(0, 115200);
    printf("SDK version:%s\n\n", system_get_sdk_version());
    printf("\n\n");
    

// -------------------- START SERVER ----------------------------------------//
    espconn_init();
    start_ap("AQUARIOUS", "patlas", 9, 6);
    vTaskDelay (1000/portTICK_RATE_MS);
    //printf("TCP server: %d\n", start_server());
    wifi_set_event_handler_cb(wifi_event);
// --------------------------------------------------------------------------//

// -------------------- ENABLE SD CARD & FatFS ------------------------------//
    char const * const vol = "0:";

    FATFS fs;
    if(FR_OK != f_mount(&fs, vol, 1)){
        printf("[ERROR] - cannot mount volume\n");
        return;
    }

    if (FR_OK != f_chdrive(vol)){
        printf("[ERROR] - cannot select volume\n");
        return;
    }

// ----------------------------------------------------------------------------//    
    if(pdTRUE != xSemaphoreGive(sentFlagSemaphore))
        printf("MAIN: cannot release semaphore\n");

    multiarg.arg1 = &sendQueue;
    multiarg.arg2 = &sentFlagSemaphore;
    if(pdPASS == xTaskCreate(sender_thread, "sender", 512, &multiarg, 2, NULL))
        printf("MAIN: task created\n");



}


/*
os_timer_t mmc_timer;

void disk_timerproc(void);

// start timer that invoke delay period for FatFs - disk_timerproc() -> mmc_esp8266.c
static void start_os_timer_mmc(void)
{
	os_timer_disarm(&mmc_timer);
	os_timer_setfn(&mmc_timer, (os_timer_func_t *)disk_timerproc, NULL);
	os_timer_arm(&mmc_timer, 5, 0x01); // 5ms is minimal timeout for os_timer
}

void LEDBlinkTask (void *pvParameters)
{
    while(1)
    {
    // Delay and turn on
    vTaskDelay (300/portTICK_RATE_MS);
        GPIO_OUTPUT_SET (2, 1);

    // Delay and LED off
        vTaskDelay (300/portTICK_RATE_MS);
        GPIO_OUTPUT_SET (2, 0);
    }
}

void DiskTask (void *pvParameters)
{

	spi_init(HSPI);
	spi_mode(HSPI, 0, 0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	
	
	vTaskDelay (100/portTICK_RATE_MS);  //delay for VCC stabilization

	

	vTaskSuspend(NULL);
}

void user_init(void)
{
    printf("SDK version:%s\n", system_get_sdk_version());
 	printf("PATLAS\n");


 	start_os_timer_mmc();

	printf("PATLAS2\n");




	//start_ap("AQUARIOUS", "patlas", 9, 6);
	//printf("TCP server: %d\n", start_server());

	xTaskCreate(DiskTask, (signed char *)"DiskTest", 256, NULL, 2, NULL);


}
*/



