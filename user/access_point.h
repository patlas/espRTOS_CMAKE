#ifndef ACCESS_POINT_H_
#define ACCESS_POINT_H_

#include "esp_common.h"
#include "espressif/c_types.h"
#include "lwip/stats.h"
#include "espconn.h"

void start_ap(uint8 *ssid, uint8 *pass, uint8 ssid_len, uint8 pass_len);

#endif /* ACCESS_POINT_H_ */
