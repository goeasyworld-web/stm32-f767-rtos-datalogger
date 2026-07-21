#include "main.h"
#include "lwip/apps/mqtt.h"
#include "mqtt_client_app.h"
#include <stdio.h>
#include <string.h>
#include "lwip/dns.h"
#include "lwip/altcp_tls.h"
#include "err.h"



static mqtt_client_t *client;
static volatile bool mqtt_connected = false;


static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
	if(status == MQTT_CONNECT_ACCEPTED)
	{
		printf("MQTT Connected \r\n");
		mqtt_connected = true;
	}
	else
	{
		printf("MQTT Connect failed, status = %d\r\n", status);
		mqtt_connected = false;
	}
}
static void mqtt_connect_tls(const ip_addr_t *broker_ip)
{
	struct mqtt_connect_client_info_t client_info;

	client_info.client_id = "stm32_board";
	client_info.client_user = "goeasy17";
	client_info.client_pass = "Iotlearning@123";
	client_info.keep_alive = 60;
	client_info.will_topic = NULL;
	/* NOTE: CubeMX/manual edits — verify ALL THREE remain 1 after any regeneration:
	   LWIP_DNS, LWIP_ALTCP, LWIP_ALTCP_TLS, LWIP_ALTCP_TLS_MBEDTLS
	   (altcp_tls_mbedtls.c compiles to an EMPTY file if LWIP_ALTCP_TLS_MBEDTLS is missing,
	    even though the .c/.h files are physically present and included in the build —
	    this produced 'undefined reference' linker errors that looked like a missing-file
	    problem but was actually a missing-macro problem) */
	client_info.tls_config = altcp_tls_create_config_client(NULL, 0);

    if (client_info.tls_config == NULL)
    {
        printf("TLS config creation FAILED (returned NULL)\r\n");
        return;
    }

	err_t err = mqtt_client_connect(client, broker_ip, 8883, mqtt_connection_cb, NULL, &client_info);
	printf("mqtt_client_connect returned err=%d\r\n", err);
}

static void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback)
{
	if(ipaddr!=NULL)
	{
		printf("DNS Resolved: %s\r\n", ip4addr_ntoa(ipaddr));
		mqtt_connect_tls(ipaddr);
	}
	else
	{
		printf("DNS Resolution Failed \r\n");
	}
}


void mqtt_app_start_hivemq(void)
{
	ip_addr_t resolved_ip;

	err_t err = dns_gethostbyname("43c4a0c06d934ed896731c85ec2a6002.s1.eu.hivemq.cloud", &resolved_ip, dns_found_cb, NULL);

	if(err == ERR_OK)
	{
		printf("DNS Resolved immediately: %s\r\n", ip4addr_ntoa(&resolved_ip));
	}
	else if(err == ERR_INPROGRESS)
	{
		printf(" DNS Lookup in progress..\r\n");
	}
	else
	{
		printf("DNS Lookup failed immediately, err=%d\r\n",err);
	}
}

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

void mqtt_app_publish_temp(int32_t temp)
{
	if(!mqtt_connected)
	{
		return;
	}

	const char* topic= "sensor/bme280/temperature";
	char payload[16];

	snprintf(payload, sizeof(payload), "%ld.%02ld", temp/100, temp %100);

	err_t err = mqtt_publish(client, topic, payload, strlen(payload), 0, 0, mqtt_app_publish_cb, NULL);
	if(err != ERR_OK)
	{
		printf("MQTT Publish call failed immediately, err=%d \r\n", err);
	}

}



/*void mqtt_app_init(void)
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
	mqtt_client_connect(client, &broker_ip, 8883, mqtt_connection_cb, NULL, &client_info);

}*/

bool mqtt_app_is_connected(void)
{
	return mqtt_connected;
}


