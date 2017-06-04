#include "access_point.h"

static struct softap_config ap_config;

void start_ap(uint8 *ssid, uint8 *pass, uint8 ssid_len, uint8 pass_len)
{
	wifi_softap_dhcps_stop();
	wifi_softap_get_config(&ap_config);

	memcpy(ap_config.ssid, ssid, ssid_len);
	memcpy(ap_config.password, pass, pass_len);

	wifi_set_opmode_current(SOFTAP_MODE);//STATIONAP_MODE
	wifi_softap_set_config_current(&ap_config);
	

	wifi_softap_dhcps_start();

	//TODO add auth mode -> enable password to established wifi connection

}
