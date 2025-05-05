#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>


#define led_red 13
#define led_green 11
#define led_blue 12
#define BUTTON_A 5
#define BUZZER_A 21

// Handles das tarefas
TaskHandle_t handle_normal = NULL;
TaskHandle_t handle_noturno = NULL;


// Flag das tarefas
bool modo = false;
bool verde = false, vermelho = false, amarelo = false, amarelo_noturno = false;

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if(gpio == botaoB){
        reset_usb_boot(0, 0);
    } else if (gpio == BUTTON_A) {
        modo = !modo;
        if (modo) {
            vTaskSuspend(handle_normal);
            vTaskResume(handle_noturno);
            amarelo_noturno = true;
            verde = amarelo = vermelho = false;
        } else {
            vTaskSuspend(handle_noturno);
            vTaskResume(handle_normal);
            amarelo_noturno = false;
        }
    }

}

void led_on(int pino) {
    gpio_init(pino);
    gpio_set_dir(pino,GPIO_OUT);
}

void button_on(int pino) {
    gpio_init(pino);
    gpio_set_dir(pino,GPIO_IN);
    gpio_pull_up(pino);
}

// Task 1
void vModo_normal() {

    led_on(BUZZER_A);
    led_on(led_red);
    led_on(led_green);

        while(true) {
            // Vermelho
            amarelo = false;
            vermelho = true;
            gpio_put(led_green,false);
            gpio_put(led_red,true);
            sleep_ms(2000);
            // Amarelo
            vermelho = false;
            amarelo = true;
            gpio_put(led_green,true);
            sleep_ms(2000);
            // Verde
            amarelo = false;
            verde = true;
            gpio_put(led_red,false);
            sleep_ms(2000);
            // Amarelo
            verde = false;
            amarelo = true;
            gpio_put(led_red,true);
            sleep_ms(2000);
        }
}

void vModo_noturno () {

    led_on(BUZZER_A);
    led_on(led_red);
    led_on(led_green);

        while(true) {
            gpio_put(led_red,true);
            gpio_put(led_green,true);
            sleep_ms(2000);
            gpio_put(led_red,false);
            gpio_put(led_green,false);
            sleep_ms(2000);
        }
}

// Ativação do buzzer
void buzz(uint8_t BUZZER_PIN, uint16_t freq, uint16_t duration) {
    int period = 1000000 / freq;
    int pulse = period / 2;
    int cycles = freq * duration / 1000;
    for (int j = 0; j < cycles; j++) {
        gpio_put(BUZZER_PIN, 1);
        sleep_us(pulse);
        gpio_put(BUZZER_PIN, 0);
        sleep_us(pulse);
    }
}

// Ativação do buzzer por um dado tempo 
void buzz_for_duration(uint8_t BUZZER_PIN, uint16_t freq, uint16_t duration, uint16_t total_time_ms) {
    uint16_t elapsed_time = 0;

    while (elapsed_time < total_time_ms) {
        buzz(BUZZER_PIN, freq, duration);
        elapsed_time += duration;
        sleep_ms(1); // Espera 1ms entre cada chamada de buzz
    }
}

void vBuzzers () {

    led_on(led_red);
    led_on(led_green);

        while(true) {
            if(vermelho) {
                buzz(BUZZER_A, 600, 500); // 500 ms ligado e
                sleep_ms(1500);//            1,5 seeg desligado
            } else if (amarelo) {
                for (int i = 0; i < 5; i++) {  
                    buzz(BUZZER_A, 800, 100);
                    sleep_ms(100);
                }
            } else if(verde) {
                buzz(BUZZER_A, 1000, 1000); // beep curto por 1 seg
                sleep_ms(900); 
            } else if (amarelo_noturno) {
                buzz(BUZZER_A, 500, 400);
                sleep_ms(2000);
            }
        }
}


int main()
{
    stdio_init_all();

    button_on(BUTTON_A);

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    xTaskCreate(vModo_normal, "Modo Normal", 1024, NULL, 1, &handle_normal);
    xTaskCreate(vModo_noturno, "Modo Noturno", 1024, NULL, 1, &handle_noturno);
    xTaskCreate(vBuzzers, "Buzzers Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    vTaskSuspend(handle_noturno);

    vTaskStartScheduler();

    panic_unsupported();

}
