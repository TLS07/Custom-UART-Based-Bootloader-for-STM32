/*
 * bl_jump.c
 *
 *  Created on: 30-May-2026
 *      Author: Admin
 */

#include"main.h"
#include "flash_layout.h"

#define  APP_MAGIC 	0x1ABCDEF0
typedef void (*pFunction)(void);

void JumpToApplication(void)
{
    uint32_t appStack;
    uint32_t appResetHandler;
    pFunction appEntry;

    /* Read application MSP */
    appStack = *(volatile uint32_t *)APP_START_ADDR;

    /* Read application Reset_Handler */
    appResetHandler = *(volatile uint32_t *)(APP_START_ADDR + 4U);

    appEntry = (pFunction)appResetHandler;

    /* Disable interrupts */
    __disable_irq();

    /* Stop SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Set application's stack pointer */
    __set_MSP(appStack);

    /* Jump to application */
    appEntry();

    /* If execution comes back here, jump failed */
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
        HAL_Delay(500);
    }
}


//fucntion to validate the application code is valid
int bootloader_is_app_valid(void)
{
    uint32_t HDR_ADDR = APP_HEADER_ADDR;

    const app_header_t *app_hdr =
            (const app_header_t*)HDR_ADDR;

    /* Check magic number */
    if(app_hdr->magic_key != APP_MAGIC)
    {
        return 1;
    }

    /* Check reset handler */
    uint32_t reset_handler =*(uint32_t*)(APP_START_ADDR + 4);

    if((reset_handler & 0xFF000000) != 0x08000000)
    {
        return 2;
    }

    return 0;
}


uint32_t Bootloader_CalculateCRC(uint32_t start_address, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFU;

    uint8_t *data = (uint8_t *)start_address;

    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 1U)
            {
                crc = (crc >> 1) ^ 0xEDB88320U;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFFU;
}
