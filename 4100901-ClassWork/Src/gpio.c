#include "gpio.h"

// Variables globales
volatile uint8_t button_pressed_left = 0;
volatile uint8_t button_pressed_right = 0;

// Delay para el debounce (espera unos milisegundos)
void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < (ms * 400); i++); // Ajuste para 4 MHz
}

// Verificar si el botón está realmente presionado (con debounce)
uint8_t is_button_pressed(uint8_t pin) {
    if (!(GPIOC->IDR & (1 << pin))) { // Verifica si está presionado
        delay_ms(50);                 // Espera 50 ms para filtrar rebote
        if (!(GPIOC->IDR & (1 << pin))) {
            return 1; // Confirmar si aún está presionado
        }
    }
    return 0;
}

// Inicializar un pin GPIO
void init_gpio_pin(GPIO_t *GPIOx, uint8_t pin, uint8_t mode) {
    GPIOx->MODER &= ~(0x3 << (pin * 2));
    GPIOx->MODER |= (mode << (pin * 2));
}

// Configurar pines GPIOA para USART2 (PA2: TX, PA3: RX)
void configure_gpio_for_usart(void) {
    *RCC_AHB2ENR |= (1 << 0); // Habilitar GPIOA

    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOA->MODER |= ((2U << (2 * 2)) | (2U << (3 * 2)));

    GPIOA->AFR[0] &= ~((0xF << (4 * 2)) | (0xF << (4 * 3)));
    GPIOA->AFR[0] |= ((7U << (4 * 2)) | (7U << (4 * 3)));

    GPIOA->OSPEEDR |= ((3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOA->PUPDR &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
}

// Configuración general de GPIO y EXTI
void configure_gpio(void) {
    *RCC_AHB2ENR |= (1 << 0) | (1 << 2);
    *RCC_APB2ENR |= (1 << 0);

    init_gpio_pin(GPIOA, LED_LEFT_PIN, 0x1);
    init_gpio_pin(GPIOA, LED_RIGHT_PIN, 0x1);
    init_gpio_pin(GPIOC, BUTTON_LEFT_PIN, 0x0);
    init_gpio_pin(GPIOC, BUTTON_RIGHT_PIN, 0x0);

    *(SYSCFG + 3) &= ~((0xF << 4) | (0xF << 8));
    *(SYSCFG + 3) |= (0x2 << 4) | (0x2 << 8);

    EXTI->FTSR1 |= (1 << BUTTON_LEFT_PIN) | (1 << BUTTON_RIGHT_PIN);
    EXTI->IMR1 |= (1 << BUTTON_LEFT_PIN) | (1 << BUTTON_RIGHT_PIN);

    *NVIC_ISER1 |= (1 << (40 - 32));
    configure_gpio_for_usart();
}

// Alternar el estado de un LED
void gpio_toggle_led(uint8_t pin) {
    TOGGLE_LED(pin);
}

// Manejador de interrupciones EXTI15_10
void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR1 & (1 << BUTTON_LEFT_PIN)) {
        EXTI->PR1 = (1 << BUTTON_LEFT_PIN); // Limpiar bandera
        if (is_button_pressed(BUTTON_LEFT_PIN)) { // Debounce
            button_pressed_left = 1;
        }
    }
    if (EXTI->PR1 & (1 << BUTTON_RIGHT_PIN)) {
        EXTI->PR1 = (1 << BUTTON_RIGHT_PIN); // Limpiar bandera
        if (is_button_pressed(BUTTON_RIGHT_PIN)) { // Debounce
            button_pressed_right = 1;
        }
    }
}
