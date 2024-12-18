#include "gpio.h"
#include "rcc.h"

#define EXTI_BASE 0x40010400
#define EXTI ((EXTI_t *)EXTI_BASE)

#define EXTI15_10_IRQn 40
#define NVIC_ISER1 ((uint32_t *)(0xE000E104)) // NVIC Interrupt Set-Enable Register



#define SYSCFG_BASE 0x40010000
#define SYSCFG ((SYSCFG_t *)SYSCFG_BASE)


#define GPIOA ((GPIO_t *)0x48000000) // Base address of GPIOA
#define GPIOC ((GPIO_t *)0x48000800) // Base address of GPIOC

#define LED_PIN 6 // Pin 5 of GPIOA
#define BUTTON_PIN 8 // Pin 13 of GPIOC

#define LED_LEFT_PIN    5
#define LED_RIGHT_PIN   1
#define BUTTON_LEFT_PIN 13
#define BUTTON_RIGHT_PIN 10



#define BUTTON_IS_PRESSED()    (!(GPIOC->IDR & (1 << BUTTON_PIN)))
#define BUTTON_IS_RELEASED()   (GPIOC->IDR & (1 << BUTTON_PIN))

#define BUTTON_LEFT_IS_PRESSED()     (!(GPIOC->IDR & (1 << BUTTON_LEFT_PIN)))
#define BUTTON_RIGHT_IS_PRESSED()    (!(GPIOC->IDR & (1 << BUTTON_RIGHT_PIN)))
#define BUTTON_IS_LEFTRELEASED()   (GPIOC->IDR & (1 << BUTTON_PIN))
#define BUTTON_IS_RELEASED()   (GPIOC->IDR & (1 << BUTTON_PIN))

#define TOGGLE_LED_LEFT()      (GPIOA->ODR ^= (1 << LED_LEFT_PIN))
#define TOGGLE_LED_RIGHT()     (GPIOA->ODR ^= (1 << LED_RIGHT_PIN))


#define TOGGLE_LED()           (GPIOA->ODR ^= (1 << LED_PIN))

volatile uint8_t button_pressed = 0; // Flag to indicate button press

volatile uint8_t button_left_pressed = 0; 
volatile uint8_t button_right_pressed = 0; 

void configure_gpio_for_usart() {
    // Enable GPIOA clock
    *RCC_AHB2ENR |= (1 << 0);

    // Configure PA2 (TX) as alternate function
    GPIOA->MODER &= ~(3U << (2 * 2)); // Clear mode bits for PA2
    GPIOA->MODER |= (2U << (2 * 2));  // Set alternate function mode for PA2

    // Configure PA3 (RX) as alternate function
    GPIOA->MODER &= ~(3U << (3 * 2)); // Clear mode bits for PA3
    GPIOA->MODER |= (2U << (3 * 2));  // Set alternate function mode for PA3

    // Set alternate function to AF7 for PA2 and PA3
    GPIOA->AFR[0] &= ~(0xF << (4 * 2)); // Clear AFR bits for PA2
    GPIOA->AFR[0] |= (7U << (4 * 2));   // Set AFR to AF7 for PA2
    GPIOA->AFR[0] &= ~(0xF << (4 * 3)); // Clear AFR bits for PA3
    GPIOA->AFR[0] |= (7U << (4 * 3));   // Set AFR to AF7 for PA3

    // Configure PA2 and PA3 as very high speed
    GPIOA->OSPEEDR |= (3U << (2 * 2)); // Very high speed for PA2
    GPIOA->OSPEEDR |= (3U << (3 * 2)); // Very high speed for PA3

    // Configure PA2 and PA3 as no pull-up, no pull-down
    GPIOA->PUPDR &= ~(3U << (2 * 2)); // No pull-up, no pull-down for PA2
    GPIOA->PUPDR &= ~(3U << (3 * 2)); // No pull-up, no pull-down for PA3
}

void init_gpio_pin(GPIO_t *GPIOx, uint8_t pin, uint8_t mode)
{
    GPIOx->MODER &= ~(0x3 << (pin * 2)); // Clear MODER bits for this pin
    GPIOx->MODER |= (mode << (pin * 2)); // Set MODER bits for this pin
}

void configure_gpio(void)
{
    // Habilitar el reloj para GPIOA y GPIOC
    *RCC_AHB2ENR |= (1 << 0) | (1 << 2); // bit0 = GPIOA, bit2 = GPIOC

    // Habilitar SYSCFG
    *RCC_APB2ENR |= (1 << 0); // RCC_APB2ENR_SYSCFGEN

    // ----- Configuración del botón izquierdo (PC13) -----
    // Asignar EXTI13 a PC13
    SYSCFG->EXTICR[3] &= ~(0xF << 4); // Limpia bits para EXTI13
    SYSCFG->EXTICR[3] |=  (0x2 << 4); // 0x2 = Puerto C

    // EXTI13 flanco de bajada
    EXTI->FTSR1 |=  (1 << BUTTON_LEFT_PIN);
    EXTI->RTSR1 &= ~(1 << BUTTON_LEFT_PIN);

    // Habilitar interrupción para EXTI13
    EXTI->IMR1 |= (1 << BUTTON_LEFT_PIN);

    // Inicializar LED izquierdo (PA5) como salida
    init_gpio_pin(GPIOA, LED_LEFT_PIN, 0x1);
    // Inicializar botón izquierdo (PC13) como entrada
    init_gpio_pin(GPIOC, BUTTON_LEFT_PIN, 0x0);

    // ----- Configuración del botón derecho (PC10) -----
    // Asignar EXTI10 a PC10
    // EXTI10 se configura en EXTICR[2] (cada registro EXTICR controla 4 líneas de EXTI).
    // EXTI8-11 se mapean en EXTICR[2], EXTI10 se ubica en los bits [11:8].
    SYSCFG->EXTICR[2] &= ~(0xF << 8);  
    SYSCFG->EXTICR[2] |=  (0x2 << 8);  // 0x2 = Puerto C

    // EXTI10 flanco de bajada
    EXTI->FTSR1 |=  (1 << BUTTON_RIGHT_PIN);
    EXTI->RTSR1 &= ~(1 << BUTTON_RIGHT_PIN);

    // Habilitar interrupción para EXTI10
    EXTI->IMR1 |= (1 << BUTTON_RIGHT_PIN);

    // Inicializar LED derecho (PA1) como salida
    init_gpio_pin(GPIOA, LED_RIGHT_PIN, 0x1);
    // Inicializar botón derecho (PC10) como entrada
    init_gpio_pin(GPIOC, BUTTON_RIGHT_PIN, 0x0);

    // Habilitar interrupciones EXTI15_10 en NVIC
    // El vector EXTI15_10 cubre EXTI líneas 10 a 15, por lo tanto ambas interrupciones (botón izquierdo y derecho) entran por aquí.
    *NVIC_ISER1 |= (1 << (EXTI15_10_IRQn - 32));

    // Configurar GPIO para USART (si es necesario)
    configure_gpio_for_usart();
}




uint8_t gpio_button_is_pressed(void)
{
    return BUTTON_IS_PRESSED();
}

void gpio_toggle_led(void)
{
    TOGGLE_LED();
}


uint8_t gpio_button_left_is_pressed(void)
{
    return BUTTON_LEFT_IS_PRESSED();
}

uint8_t gpio_button_right_is_pressed(void)
{
    return BUTTON_RIGHT_IS_PRESSED();
}

void gpio_toggle_led_left(void)
{
    TOGGLE_LED_LEFT();
}

void gpio_toggle_led_right(void)
{
    TOGGLE_LED_RIGHT();
}

void EXTI15_10_IRQHandler(void)
{
    // Verificar si la interrupción proviene del botón izquierdo
    if (EXTI->PR1 & (1 << BUTTON_LEFT_PIN)) {
        EXTI->PR1 = (1 << BUTTON_LEFT_PIN); // Limpiar el bit pendiente
        button_left_pressed = 1;            // Establecer bandera para botón izquierdo
    }

    // Verificar si la interrupción proviene del botón derecho
    if (EXTI->PR1 & (1 << BUTTON_RIGHT_PIN)) {
        EXTI->PR1 = (1 << BUTTON_RIGHT_PIN); // Limpiar el bit pendiente
        button_right_pressed = 1;            // Establecer bandera para botón derecho
    }

    if (EXTI->PR1 & (1 << BUTTON_PIN)) {
        EXTI->PR1 = (1 << BUTTON_PIN); // Clear pending bit
        button_pressed = 1; // Set button pressed flag
    }

}

