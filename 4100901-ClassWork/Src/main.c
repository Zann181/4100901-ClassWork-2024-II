#include <stdint.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"

// Estado de LEDs
uint8_t led_left_on = 0;   // Estado del LED izquierdo
uint8_t led_right_on = 0;  // Estado del LED derecho
uint8_t led_main_on = 0;   // Estado del LED principal

// Tiempo de debounce en milisegundos
#define DEBOUNCE_TIME_MS 50

// Función para procesar comandos UART
void process_uart_command(const char *buffer) {
    if (buffer[0] == 'L' && buffer[1] == '1') {
        gpio_toggle_led_left();  // Encender/Alternar LED izquierdo
        UART_send_string(USART2, "LED Izquierdo Activado por UART\r\n");
        led_left_on = !led_left_on;
    } else if (buffer[0] == 'L' && buffer[1] == '2') {
        gpio_toggle_led_right(); // Encender/Alternar LED derecho
        UART_send_string(USART2, "LED Derecho Activado por UART\r\n");
        led_right_on = !led_right_on;
    } else {
        UART_send_string(USART2, "Comando Invalido\r\n");
    }
}

int main(void) {
    configure_systick_and_start();  // Configurar SysTick
    configure_gpio();               // Configurar GPIOs y botones
    UART_Init(USART2);              // Inicializar UART

    UART_send_string(USART2, "Sistema iniciado. Use L1 o L2 para activar LEDs.\r\n");

    uint32_t last_button_left_time = 0;   // Tiempo último botón izquierdo
    uint32_t last_button_right_time = 0;  // Tiempo último botón derecho
    uint32_t last_button_main_time = 0;   // Tiempo último botón principal

    char buffer[10];                      // Buffer UART
    UART_receive_it(USART2, buffer, sizeof(buffer)); // Recepción UART

    while (1) {
        uint32_t now = systick_GetTick(); // Tiempo actual

        // Procesar interrupciones de botón izquierdo con debounce
        if (button_left_pressed && (now - last_button_left_time >= DEBOUNCE_TIME_MS)) {
            button_left_pressed = 0; // Limpiar bandera
            last_button_left_time = now; // Actualizar tiempo
            if (led_left_on) {
                UART_send_string(USART2, "LED Izquierdo Apagado.\r\n");
            } else {
                UART_send_string(USART2, "LED Izquierdo Encendido.\r\n");
            }
            gpio_toggle_led_left();  // Alternar LED izquierdo
            led_left_on = !led_left_on;
        }

        // Procesar interrupciones de botón derecho con debounce
        if (button_right_pressed && (now - last_button_right_time >= DEBOUNCE_TIME_MS)) {
            button_right_pressed = 0; // Limpiar bandera
            last_button_right_time = now; // Actualizar tiempo
            if (led_right_on) {
                UART_send_string(USART2, "LED Derecho Apagado.\r\n");
            } else {
                UART_send_string(USART2, "LED Derecho Encendido.\r\n");
            }
            gpio_toggle_led_right(); // Alternar LED derecho
            led_right_on = !led_right_on;
        }

        // Procesar interrupciones de botón principal con debounce
        if (button_pressed && (now - last_button_main_time >= DEBOUNCE_TIME_MS)) {
            button_pressed = 0; // Limpiar bandera
            last_button_main_time = now; // Actualizar tiempo
            if (led_main_on) {
                UART_send_string(USART2, "LED Principal Apagado.\r\n");
            } else {
                UART_send_string(USART2, "LED Principal Encendido.\r\n");
            }
            gpio_toggle_led();  // Alternar LED principal
            led_main_on = !led_main_on;
        }

        // Procesar comandos UART
        if (rx_ready) {
            rx_ready = 0; // Limpiar bandera de recepción
            process_uart_command(buffer); // Procesar comando recibido
            UART_receive_it(USART2, buffer, sizeof(buffer)); // Reiniciar recepción UART
        }
    }
}
