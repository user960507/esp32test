#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/temperature_sensor.h"

#include "time.h" //time funtions
#include "sys/time.h"//time funtions

#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"


// Define GPIO pin for LED
#define LED_BUILTIN 8 // recuerda que en esp 0 es on para el led

#define EXAMPLE_ESP_WIFI_SSID      "myWifi"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       1

#define EXAMPLE_GTK_REKEY_INTERVAL 0
#define EXAMPLE_GTK_REKEY_INTERVAL 0

#define CONFIG_EXAMPLE_IPV4 "IPV4"
#define IIPADDR_ANY "192.168.4.2"  //hardcoded por el momento
#define PORT                        5555
#define KEEPALIVE_IDLE              5
#define KEEPALIVE_INTERVAL          5
#define KEEPALIVE_COUNT             3


bool connected = false;

//Definicion de los commandos
typedef enum comandos{
    gettemp,
    blynk1,
    unknown
}Comms;

Comms resolveCMD(char *datos){
    if(strcmp(datos, "gettemp")==0){return gettemp;}
    if(strcmp(datos, "blynk1")==0){return blynk1;}
    return unknown;
}



void append(char *dest, char *src) {
    while (*dest) { // Move the pointer to the end of the destination string
        dest++;
    }
    while (*src) { // Copy the source string to the destination
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0'; // Null-terminate the resulting string
}

//led functions
void led_setup(void){
    // Configure GPIO
    gpio_reset_pin(LED_BUILTIN);
    gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_BUILTIN, 1);
}

void blink_code(int cantidad, int tiempoms, int ciclos)
{   

    for(int i = 0; i < ciclos; i++){
        //ciclos
        for(int j = 0; j <cantidad; j++)
        {
            //cantidad de blinks
            
                // Turn LED ON
                printf("LED ON\n");

                gpio_set_level(LED_BUILTIN, 0);
                vTaskDelay(tiempoms / portTICK_PERIOD_MS); // Delay 1 second

                // Turn LED OFF
                printf("LED OFF\n");
                gpio_set_level(LED_BUILTIN, 1);
                vTaskDelay(tiempoms / portTICK_PERIOD_MS); // Delay 1 second
        }
        gpio_set_level(LED_BUILTIN, 1);
        vTaskDelay(5*tiempoms / portTICK_PERIOD_MS); // Delay 5 second
    }


}

//temp sensor functions and subs
temperature_sensor_handle_t temp_setup(void){
    temperature_sensor_handle_t temp_sensor = NULL;
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    temperature_sensor_install(&temp_sensor_config, &temp_sensor);
    return temp_sensor;
}

float get_temperature(temperature_sensor_handle_t temp_sensor){
    float temp;

    for(int i =0; i<5; i++){
        temperature_sensor_get_celsius(temp_sensor, &temp);
        vTaskDelay(100/ portTICK_PERIOD_MS); // Delay 0.1 second
    }
    
    return temp;
}

void tcp_client(void)
{
    char rx_buffer[128];
    char host_ip[] = IIPADDR_ANY;
    int addr_family = 0;
    int ip_protocol = 0;

    //setups
    led_setup(); // habiaaa ovidado configurar el pin para el led!!!!!
    temperature_sensor_handle_t temp_sensor = temp_setup();
    temperature_sensor_enable(temp_sensor);

    //variables
    float temperature;
    char buffy_temp[32];
    char *payload = "Message from ESP32____\n";

    while (1) {
		if (!connected){
			break;
		}
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;


        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            printf("Unable to create socket: errno %d", errno);
            break;
        }
        printf("Socket created, connecting to %s:%d \n", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            printf("Socket unable to connect: errno %d", errno);
            break;
        }
        printf("Successfully connected \n");

        while (1) {
            int err = 0;
            err = send(sock, payload, strlen(payload), 0);

//----------------------------------recieve dataaa
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occurred during receiving
            if (len < 0) {
                printf("recv failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                printf("Received %d bytes from %s: \n", len, host_ip);
                printf( "%s", rx_buffer);
            }

//------------------------------------send dataaa
            /*
            if(strcmp(rx_buffer, "gettemp")== 0){
                temperature = get_temperature(temp_sensor);
                printf("lectura  %.02f C\n", temperature);
                sprintf(buffy_temp, "lectura %.02f C\n", temperature);
                err = send(sock, buffy_temp, strlen(buffy_temp), 0);
            }
            else{
                err = send(sock, payload, strlen(payload), 0);
            }
            */
           switch (resolveCMD(rx_buffer))
           {
           case gettemp:
                temperature = get_temperature(temp_sensor);
                printf("lectura  %.02f C\n", temperature);
                sprintf(buffy_temp, "lectura %.02f C\n", temperature);
                err = send(sock, buffy_temp, strlen(buffy_temp), 0);
            break;
            case blynk1:
                printf("blinkinnnnn");
                blink_code(3,250,3);
                err = send(sock, payload, strlen(payload), 0);
            break;
           
           default:
            err = send(sock, payload, strlen(payload), 0);
            break;
           }
//------------------------------------send dataaa


            //int err = send(sock, payload, strlen(payload), 0);
			printf("ENVIEEE DATOSSSSSSSS\n");
            if (err < 0) {
                printf("Error occurred during sending: errno %d \n", errno);
                break;
            }


        }

        if (sock != -1) {
            printf("Shutting down socket and restarting...\n");
            shutdown(sock, 0);
            close(sock);
        }
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        printf("station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
		connected = true;
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay 1 second
		tcp_client();
		
		

    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        printf("station "MACSTR" leave, AID=%d, reason=%d", MAC2STR(event->mac), event->aid, event->reason);
    }
}

void wifi_init_softap(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
	
    

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .pmf_cfg = {
                    .required = true,
            },
            .gtk_rekey_interval = EXAMPLE_GTK_REKEY_INTERVAL,
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    printf("Parece que no fallo la config\n");

    
}

void app_main(void)
{   


    //Initialize NVS
    nvs_flash_init();
    wifi_init_softap();
    

}