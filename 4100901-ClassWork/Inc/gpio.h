#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// Definición de estructuras GPIO y EXTI
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
    volatile uint32_t BRR;
} GPIO_t;

typedef struct {
    volatile uint32_t IMR1;
    volatile uint32_t EMR1;
    volatile uint32_t RTSR1;
    volatile uint32_t FTSR1;
    volatile uint32_t SWIER1;
    volatile uint32_t PR1;
} EXTI_t;

#define GPIOA ((GPIO_t *)0x48000000)
#define GPIOC ((GPIO_t *)0x48000800)
#define EXTI  ((EXTI_t *)0x40010400)
#define SYSCFG ((uint32_t *)0x40010000)
#define RCC_AHB2ENR ((uint32_t *)0x4002104C)
#define RCC_APB2ENR ((uint32_t *)0x40021060)
#define NVIC_ISER1  ((uint32_t *)0xE000E104)

// Definición de pines
#define LED_LEFT_PIN    5
#define LED_RIGHT_PIN   6
#define BUTTON_LEFT_PIN 13
#define BUTTON_RIGHT_PIN 14

#define BUTTON_IS_PRESSED(pin) (!(GPIOC->IDR & (1 << pin)))
#define TOGGLE_LED(pin) (GPIOA->ODR ^= (1 << pin))

// Variables globales
extern volatile uint8_t button_pressed_left;
extern volatile uint8_t button_pressed_right;

// Prototipos de funciones
void delay_ms(uint32_t ms);
void init_gpio_pin(GPIO_t *GPIOx, uint8_t pin, uint8_t mode);
void configure_gpio_for_usart(void);
void configure_gpio(void);
void gpio_toggle_led(uint8_t pin);
uint8_t gpio_button_is_pressed(uint8_t pin);
void EXTI15_10_IRQHandler(void);

#endif // GPIO_H
