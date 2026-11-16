#ifndef TEST_HAL_H
#define TEST_HAL_H
#include <stdint.h>
#include <stddef.h>
typedef struct { uint16_t levels; } GPIO_TypeDef;
typedef struct { int unused; } I2C_HandleTypeDef;
#ifdef __cplusplus
extern "C" {
#endif
extern GPIO_TypeDef test_gpio_a;
extern GPIO_TypeDef test_gpio_g;
extern GPIO_TypeDef test_gpio_n;
extern GPIO_TypeDef test_gpio_p;
#ifdef __cplusplus
}
#endif
#define GPIOA (&test_gpio_a)
#define GPIOP (&test_gpio_p)
#define GPION (&test_gpio_n)
#define GPIOG (&test_gpio_g)
#define GPIO_PIN_0  (1U << 0)
#define GPIO_PIN_1  (1U << 1)
#define GPIO_PIN_2  (1U << 2)
#define GPIO_PIN_3  (1U << 3)
#define GPIO_PIN_4  (1U << 4)
#define GPIO_PIN_5  (1U << 5)
#define GPIO_PIN_7  (1U << 7)
#define GPIO_PIN_9  (1U << 9)
#define GPIO_PIN_10 (1U << 10)
#define GPIO_PIN_11 (1U << 11)
#define GPIO_PIN_12 (1U << 12)
#define GPIO_PIN_14 (1U << 14)
#define GPIO_PIN_15 (1U << 15)
typedef enum { GPIO_PIN_RESET, GPIO_PIN_SET } GPIO_PinState;
typedef enum { HAL_OK = 0, HAL_ERROR = 1 } HAL_StatusTypeDef;
#define I2C_MEMADD_SIZE_8BIT 1U
#ifdef __cplusplus
extern "C" {
#endif
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *bus, uint16_t address,
                                   uint16_t offset, uint16_t offset_size,
                                   uint8_t *data, uint16_t length, uint32_t timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *bus, uint16_t address,
                                    uint16_t offset, uint16_t offset_size,
                                    uint8_t *data, uint16_t length, uint32_t timeout);
#ifdef __cplusplus
}
#endif
#endif
