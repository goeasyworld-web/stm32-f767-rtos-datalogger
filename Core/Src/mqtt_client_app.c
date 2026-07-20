#include "main.h"
#include "lwip/apps/mqtt.h"
#include "mqtt_client_app.h"
#include <stdio.h>
#include <string.h>

static mqtt_client_t *client;
static volatile bool mqtt_connected = false;

static void mqtt_app_publish_cb(void *args, err_t result)
{
	if(result == ERR_OK)
	{
		printf("MQTT Publish OK\r\n");
	}
	else
	{
		printf("MQTT Publish failed \r\n");
	}
}

static void mqtt_app_publish_test(void)
{
	const char* topic= "sensor/test";
	const char*message = "hello from board";

	err_t err = mqtt_publish(client, topic, message, strlen(message), 0, 0, mqtt_app_publish_cb, NULL);
	if(err != ERR_OK)
	{
		printf("MQTT Publish call failed immediately, err=%d \r\n", err);
	}

}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
	if(status == MQTT_CONNECT_ACCEPTED)
	{
		printf("MQTT Connected \r\n");
		mqtt_connected = true;
		mqtt_app_publish_test();
	}
	else
	{
		printf("MQTT Connect failed, status = %d\r\n", status);
		mqtt_connected = false;
	}
}

void mqtt_app_init(void)
{
	struct mqtt_connect_client_info_t client_info;
	ip_addr_t broker_ip;
	client = mqtt_client_new();
	client_info.client_id = "stm32 board";
	client_info.client_user = NULL;
	client_info.client_pass = NULL;
	client_info.keep_alive = 60;
	client_info.will_topic = NULL;

	IP4_ADDR(&broker_ip, 192, 168, 0, 164);
	mqtt_client_connect(client, &broker_ip, 1883, mqtt_connection_cb, NULL, &client_info);
}

bool mqtt_app_is_connected(void)
{
	return mqtt_connected;
}


