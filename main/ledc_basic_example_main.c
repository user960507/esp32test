#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define GPIO pin for LED
#define LED_BUILTIN 8 // recuerda que en esp 0 es on para el led

void led_setup(void){
    // Configure GPIO
    gpio_reset_pin(LED_BUILTIN);
    gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
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

void app_main(void)
{

    led_setup();

    blink_code(3,1000,5);

}