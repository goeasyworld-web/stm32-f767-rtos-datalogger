

#ifndef INC_MQTT_CLIENT_APP_H_
#define INC_MQTT_CLIENT_APP_H_

#include <stdbool.h>
#include "err.h"
#include "lwip.h"

void mqtt_app_init(void);
bool mqtt_app_is_connected(void);
static void mqtt_app_publish_cb(void *args, err_t result);
void mqtt_app_publish_temp(int32_t temp);
static void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback);






#endif

