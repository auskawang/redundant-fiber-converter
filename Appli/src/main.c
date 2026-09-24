/**
 * @file main.c
 * @brief Application entry point: system boot, peripheral init, and task scheduling.
 */

#include <stdint.h>
#include <stdbool.h>

#include "stm32n6xx_hal.h"
#include "app_config.h"
#include "board_pins.h"

/* Platform Adaptation */
#include "bxp_clock.h"
#include "bxp_gpio.h"
#include "bxp_spi.h"
#include "bxp_i2c.h"

/* Application Modules */
#include "app_init.h"
#include "path_manager.h"
#include "peer_comm.h"
#include "optical_health.h"
#include "diagnostics.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* 1. Reset of all peripherals, Initializes the Flash interface and Systick */
    HAL_Init();

    /* 2. Configure system clock tree (STM32N6) */
    bxp_clock_init();

    /* 3. Initialize Board Support Package (GPIOs, SPI, I2C) */
    bxp_gpio_init();
    bxp_spi_init();
    bxp_i2c_init();

    /* 4. Power Sequencing & Hardware Reset of Switch/PHYs */
    app_init_power_sequence();

    /* 5. Initialize Diagnostics & Logging */
    diagnostics_init();

    /* 6. Initialize State Machines & Communication */
    peer_comm_init();
    optical_health_init();
    path_manager_init();

    /* Main Application Loop / Cooperative Scheduler */
    while (1)
    {
        /* Process peer heartbeats and communication state */
        peer_comm_process();

        /* Evaluate SFP optical telemetry (RSSI, Temp, Bias) */
        optical_health_process();

        /* Execute primary/backup failover state machine */
        path_manager_process();

        /* Collect and report diagnostics */
        diagnostics_process();
    }

    return 0;
}

/**
 * @brief  This function is executed in case of error occurrence.
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
