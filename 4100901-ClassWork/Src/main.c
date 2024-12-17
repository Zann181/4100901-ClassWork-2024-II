#include <stdint.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"

// Función para procesar comandos UART
void process_uart_command(const char *buffer) {
    if (buffer[0] == 'L' && buffer[1] == '1') {
        gpio_toggle_led(LED_LEFT_PIN);  // Encender/Alternar LED izquierdo
        UART_send_string(USART2, "LED Izquierdo Activado por UART\r\n");
    } else if (buffer[0] == 'L' && buffer[1] == '2') {
        gpio_toggle_led(LED_RIGHT_PIN); // Encender/Alternar LED derecho
        UART_send_string(USART2, "LED Derecho Activado por UART\r\n");
    } else {
        UART_send_string(USART2, "Comando Invalido\r\n");
    }
}

int main(void) {
    configure_systick_and_start(); // Configura SysTick
    configure_gpio();              // Configura GPIOs y EXTI
    UART_Init(USART2);             // Inicializa UART2

    //UART_send_string(USART2, "Sistema Iniciado. Use L1 o L2 para activar LEDs\r\n");

    uint8_t buffer[10]; // Buffer para recepción UART

    // Recepción asíncrona de UART
    UART_receive_it(USART2, buffer, sizeof(buffer));

    while (1) {
        // Interrupciones por botones físicos
        if (button_pressed_left) {
            gpio_toggle_led(LED_LEFT_PIN); // Alterna LED izquierdo
            UART_send_string(USART2, "Botón Izquierdo Presionado, LED Izquierdo Encendido.\r\n");
            button_pressed_left = 0;       // Limpia la bandera
        }
        if (button_pressed_right) {
            gpio_toggle_led(LED_RIGHT_PIN); // Alterna LED derecho
            UART_send_string(USART2, "Botón Derecho Presionado, LED Derecho Encendido.\r\n");
            button_pressed_right = 0;       // Limpia la bandera
        }


        // Procesar comandos UART
        if (rx_ready != 0) {
            UART_send_string(USART2, "Comando Recibido: ");
            UART_send_string(USART2, (char *)buffer);
            UART_send_string(USART2, "\r\n");

            process_uart_command((char *)buffer); // Procesar comando UART
            UART_receive_it(USART2, buffer, sizeof(buffer)); // Reiniciar recepción UART
            rx_ready = 0; // Limpiar bandera de recepción
        }

   
   }
}
