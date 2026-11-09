#ifndef TEST_HAL_H
#define TEST_HAL_H
#include <stdint.h>
#include <stddef.h>
typedef struct { uint16_t levels; } GPIO_TypeDef;
extern GPIO_TypeDef test_gpio_e;
#define GPIOE (&test_gpio_e)
#define GPIO_PIN_2 (1U << 2)
#define GPIO_PIN_3 (1U << 3)
#define GPIO_PIN_4 (1U << 4)
#define GPIO_PIN_5 (1U << 5)
typedef enum { GPIO_PIN_RESET, GPIO_PIN_SET } GPIO_PinState;
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
#endif
