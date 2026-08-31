/*
 * firware_update.c
 *
 *  Created on: 13-Jun-2026
 *      Author: Admin
 */


#include "main.h"
extern UART_HandleTypeDef huart2;
extern IWDG_HandleTypeDef hiwdg;
#define APP_MAX_SIZE      48128U         // Maximum application size (47 KB)
BL_Status_t Bootloader_validation_header(const app_header_t* header);


BL_Status_t Bootloader_VerifyFirmwareCRC(const app_header_t *header);

void JumpToApplication(void);

BL_Status_t Bootloader_WriteChunk(uint32_t flash_address,
                                  uint8_t *buffer,
                                  uint32_t length);

BL_Status_t  Bootloader_EraseApplication(void);
BL_Status_t Bootloader_ReceiveFirmware(const app_header_t *header);
uint32_t Bootloader_CalculateCRC(uint32_t start_address, uint32_t length);

void Bootloader_UpdateMode(void)
{
    uint8_t rx_buffer[5];
    app_header_t rx_header;
    char tx_msg[100];

    HAL_UART_Transmit(&huart2, (uint8_t *)"Waiting for PC...\r\n", strlen("Waiting for PC...\r\n"), 100);

    while (1)
    {
        
        if (HAL_UART_Receive(&huart2, rx_buffer, 5, HAL_MAX_DELAY) == HAL_OK)
        {
        	HAL_IWDG_Refresh(&hiwdg);
            if (memcmp(rx_buffer, "BOOT1", 5U) == 0)
            {
                HAL_UART_Transmit(&huart2, (uint8_t *)"ACK\r\n", strlen("ACK\r\n"), 100);

                /* Receive application header */
                if (HAL_UART_Receive(&huart2, (uint8_t *)&rx_header, sizeof(app_header_t), HAL_MAX_DELAY) == HAL_OK)
                {
                	HAL_IWDG_Refresh(&hiwdg);
                    HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER RECEIVED\r\n", strlen("HEADER RECEIVED\r\n"), 100);

                    sprintf(tx_msg, "Magic=%08lX Size=%lu CRC=%08lX Ver=%lu\r\n",
                            (unsigned long)rx_header.magic_key,
                            (unsigned long)rx_header.size,
                            (unsigned long)rx_header.crc,
                            (unsigned long)rx_header.version);

                    HAL_UART_Transmit(&huart2, (uint8_t *)tx_msg, strlen(tx_msg), 100);

                    /* Validate Header */
                    if (Bootloader_validation_header(&rx_header) != BL_OK)
                    {
                        continue;
                    }
                    HAL_IWDG_Refresh(&hiwdg);

                    HAL_UART_Transmit(&huart2, (uint8_t *)"READY FOR FLASH ERASE\r\n", strlen("READY FOR FLASH ERASE\r\n"), 100);

                    /* Erase header region */
                    if (Bootloader_EraseHeader() != BL_OK)
                    {
                        continue;
                    }
                    HAL_IWDG_Refresh(&hiwdg);

                    /* Erase application Flash */
                    if (Bootloader_EraseApplication() != BL_OK)
                    {
                        continue;
                    }
                    HAL_IWDG_Refresh(&hiwdg);

                    /* Receive firmware and write to Flash */
                    if (Bootloader_ReceiveFirmware(&rx_header) != BL_OK)
                    {
                        continue;
                    }

                    /* Verify firmware CRC */
                    if (Bootloader_VerifyFirmwareCRC(&rx_header) != BL_OK)
                    {
                        continue;
                    }
                    HAL_IWDG_Refresh(&hiwdg);


                    /* Persist header to flash only after firmware is verified good */
                    if (Bootloader_WriteHeader(&rx_header) != BL_OK)
                    {
                        continue;
                    }
                    HAL_IWDG_Refresh(&hiwdg);
                    HAL_UART_Transmit(&huart2, (uint8_t *)"CRC OK \r\n", strlen("CRC OK \r\n"), 100);
                    HAL_UART_Transmit(&huart2, (uint8_t *)"UPDATE COMPLETE\r\n", strlen("UPDATE COMPLETE\r\n"), 100);

                     HAL_UART_Transmit(&huart2, (uint8_t *)"STARTING APPLICATION\r\n", strlen("STARTING APPLICATION\r\n"), 100);


                     HAL_UART_Transmit(&huart2,
                  (uint8_t *)"BEFORE JUMP FUNCTION\r\n",
                  strlen("BEFORE JUMP FUNCTION\r\n"),
                  100);
                    JumpToApplication();

                    
                    
                }
            }
        }
    }
}









