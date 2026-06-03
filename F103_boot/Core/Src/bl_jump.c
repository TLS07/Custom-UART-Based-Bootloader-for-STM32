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

    /* Read application msp */
    appStack = *(volatile uint32_t*)APP_START_ADDR;


    /* Read reset handler address  */
    appResetHandler = *(volatile uint32_t*)(APP_START_ADDR + 4);
    appEntry = (pFunction)appResetHandler;

    /* Disable interrupts */
    __disable_irq();

    /* Stop SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Set main stack pointer */
    __set_MSP(appStack);

    /* Jump to application reset handler */
    appEntry();
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

