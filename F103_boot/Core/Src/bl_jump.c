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
extern UART_HandleTypeDef huart2;

void JumpToApplication(void)
{
    uint32_t appStack;
    uint32_t appResetHandler;
    pFunction appEntry;
    char buff[100];

//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)"JUMP: Entered JumpToApplication\r\n",
//                      strlen("JUMP: Entered JumpToApplication\r\n"),
//                      100);

    /* Read application MSP */
    appStack = *(volatile uint32_t *)APP_START_ADDR;

    sprintf(buff,
            "JUMP: MSP = 0x%08lX\r\n",
            (unsigned long)appStack);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)buff,
                      strlen(buff),
                      100);

    /* Read application Reset_Handler */
    appResetHandler = *(volatile uint32_t *)(APP_START_ADDR + 4U);

    sprintf(buff,
            "JUMP: RESET_HANDLER = 0x%08lX\r\n",
            (unsigned long)appResetHandler);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)buff,
                      strlen(buff),
                      100);

    /* Store application entry address */
    appEntry = (pFunction)appResetHandler;

//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)"JUMP: Entry address loaded\r\n",
//                      strlen("JUMP: Entry address loaded\r\n"),
//                      100);

    /* Disable interrupts */
//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)"JUMP: Disabling IRQ\r\n",
//                      strlen("JUMP: Disabling IRQ\r\n"),
//                      100);

    __disable_irq();

    /* Stop SysTick */
//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)"JUMP: Stopping SysTick\r\n",
//                      strlen("JUMP: Stopping SysTick\r\n"),
//                      100);

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Set application's vector table */
//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)"JUMP: Setting VTOR\r\n",
//                      strlen("JUMP: Setting VTOR\r\n"),
//                      100);

    SCB->VTOR = APP_START_ADDR;

    /* Set application's stack pointer */
//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)"JUMP: Setting MSP\r\n",
//                      strlen("JUMP: Setting MSP\r\n"),
//                      100);

    __set_MSP(appStack);

    /* Jump to application */
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"JUMP: Calling application Reset_Handler\r\n",
                      strlen("JUMP: Calling application Reset_Handler\r\n"),
                      100);

    appEntry();

    /* If execution comes back here, jump failed */
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"JUMP: Returned from application!\r\n",
                      strlen("JUMP: Returned from application!\r\n"),
                      100);

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
        HAL_Delay(500);
    }
}





