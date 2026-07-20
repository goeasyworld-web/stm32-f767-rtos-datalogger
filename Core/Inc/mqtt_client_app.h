

#ifndef INC_MQTT_CLIENT_APP_H_
#define INC_MQTT_CLIENT_APP_H_

#include <stdbool.h>

void mqtt_app_init(void);
bool mqtt_app_is_connected(void);
static void mqtt_app_publish_cb(void *args, err_t result);
static void mqtt_app_publish_test(void);

#endif

