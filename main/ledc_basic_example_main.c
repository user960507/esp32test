#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/temperature_sensor.h"


// Define GPIO pin for LED
#define LED_BUILTIN 8 // recuerda que en esp 0 es on para el led


//led functions and subs
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
        gpio_set_level(LED_BUILTIN, 0);
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

void app_main(void)
{   
    //variables
    temperature_sensor_handle_t temp_sensor;
    float temperature;

    //setups
    led_setup();

    temp_sensor = temp_setup();
    temperature_sensor_enable(temp_sensor);

    //main logic
    temperature_sensor_get_celsius(temp_sensor, &temperature);
    printf("lectura %.02f C", temperature);
    blink_code(5,500,5);

}