#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"

// Pinos
#define RED_LED_PIN 13
#define A_BTN_PIN 5
#define B_BTN_PIN 6
#define IS_RGBW false
#define WS2812_PIN 7

// Variáveis globais
static volatile uint current_number = 0;
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

// Buffer do padrão de desenhos para a matriz 5x5
bool led_buffer[10][25] = {
    0, 1, 1, 1, 0, 
    1, 0, 0, 0, 1, 
    1, 0, 0, 0, 1, 
    1, 0, 0, 0, 1, 
    0, 1, 1, 1, 0,

    0, 0, 1, 0, 0, 
    0, 1, 1, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 1, 0, 0,

    0, 1, 1, 1, 0, 
    1, 0, 0, 1, 0, 
    0, 0, 1, 0, 0, 
    0, 1, 0, 0, 0, 
    1, 1, 1, 1, 1,

    1, 1, 1, 1, 0, 
    0, 0, 0, 1, 0, 
    0, 1, 1, 0, 0, 
    0, 0, 0, 1, 0, 
    1, 1, 1, 1, 0
};

// Prototipo de funções
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void set_led(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Função de interrupção com debouncing.
 * 
 * @param gpio a GPIO que gerou interrupção.
 * @param events a evento que gerou interrupção.
 */
static void gpio_irq_handler(uint gpio, uint32_t events);


int main() {
    // Inicializações
    stdio_init_all();

    // LED vermelho
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);

    // Botões
    gpio_init(A_BTN_PIN);
    gpio_set_dir(A_BTN_PIN, GPIO_IN);
    gpio_pull_up(A_BTN_PIN);

    gpio_init(B_BTN_PIN);
    gpio_set_dir(B_BTN_PIN, GPIO_IN);
    gpio_pull_up(B_BTN_PIN);

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Configuração das interrupções dos botões com callback
    gpio_set_irq_enabled_with_callback(A_BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(B_BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (1) {
        gpio_put(RED_LED_PIN, 1);
        sleep_ms(1000);
        gpio_put(RED_LED_PIN, 0);
        sleep_ms(1000);
    }

    return 0;
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_led(uint8_t r, uint8_t g, uint8_t b) {
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < 25; i++) {
        if (led_buffer[current_number][i]) {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}

static void gpio_irq_handler(uint gpio, uint32_t events) {
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    printf("A = %d\n", current_number);
    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 2000000) { // 200 ms? de debouncing
        last_time = current_time; // Atualiza o tempo do último evento

        if (gpio == 5) {
            set_led(255, 102, 178); // setando os leds na cor rosa
            if(current_number < 10) { current_number++; }
        } else if (gpio == 6) {
            set_led(255, 102, 178); // setando os leds na cor rosa
            if(current_number > 0) { current_number--; }
        }
    }
}
