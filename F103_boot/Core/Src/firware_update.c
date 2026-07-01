/*
 * firware_update.c
 *
 *  Created on: 13-Jun-2026
 *      Author: Admin
 */


#include "main.h"
extern UART_HandleTypeDef huart2;
#define APP_MAX_SIZE      48128U         // Maximum application size (47 KB)
BL_Status_t Bootloader_validation_header(const app_header_t* header);
void Bootloader_UpdateMode(void)
{
    uint8_t rx_buffer[5];
    app_header_t rx_header;
    char tx_msg[100];

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)"Waiting for PC...\r\n",
                      strlen("Waiting for PC...\r\n"),
                      100);

    while(1)
    {
        if(HAL_UART_Receive(&huart2,
                            rx_buffer,
                            5,
                            HAL_MAX_DELAY) == HAL_OK)
        {
            if(memcmp(rx_buffer, "HELLO", 5U) == 0)
            {
                HAL_UART_Transmit(&huart2,
                                  (uint8_t*)"ACK\r\n",
                                  strlen("ACK\r\n"),
                                  100);

                /* Receive application header */
                if(HAL_UART_Receive(&huart2,
                                    (uint8_t *)&rx_header,
                                    sizeof(app_header_t),
                                    HAL_MAX_DELAY) == HAL_OK)
                {
                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)"HEADER RECEIVED\r\n",
                                      strlen("HEADER RECEIVED\r\n"),
                                      100);

                    sprintf(tx_msg,
                            "Magic=%08lX Size=%lu CRC=%08lX Ver=%lu\r\n",
                            (unsigned long)rx_header.magic_key,
                            (unsigned long)rx_header.size,
                            (unsigned long)rx_header.crc,
                            (unsigned long)rx_header.version);

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t *)tx_msg,
                                      strlen(tx_msg),
                                      100);

                    /* Validate Header */
                    if (Bootloader_validation_header(&rx_header) != BL_OK)
                    {
                    	/* Invalid header, wait for a new update request */
                    	continue;
                    }

                    HAL_UART_Transmit(&huart2,(uint8_t *)"READY FOR FLASH ERASE\r\n",strlen("READY FOR FLASH ERASE\r\n"),100);
                }
            }
        }
    }
}



/*
 * This function is to validate that , the recived header is right or wrong  */
BL_Status_t Bootloader_validation_header(const app_header_t* header)
{
	//check the magic key
	if(header->magic_key!=APP_MAGIC)
	{
		  HAL_UART_Transmit(&huart2,
		                          (uint8_t *)"HEADER_FAIL : INVALID MAGIC\r\n",
		                          strlen("HEADER_FAIL : INVALID MAGIC\r\n"),
		                          100);
		  return BL_ERROR;
	}

	// check application size
	if(header->size==0U)
	{
		  HAL_UART_Transmit(&huart2,
		                          (uint8_t *)"HEADER_FAIL : INVALID SIZE\r\n",
		                          strlen("HEADER_FAIL : INVALID SIZE\r\n"),
		                          100);

		        return BL_ERROR;
	}

	if(header->size > APP_MAX_SIZE)
	{
		HAL_UART_Transmit(&huart2,
	                          (uint8_t *)"HEADER_FAIL : APP TOO LARGE\r\n",
	                          strlen("HEADER_FAIL : APP TOO LARGE\r\n"),
	                          100);

	        return BL_ERROR;
	    }
	  /* Header is valid */
	    HAL_UART_Transmit(&huart2,
	                      (uint8_t *)"HEADER_OK\r\n",
	                      strlen("HEADER_OK\r\n"),
	                      100);

	    return BL_OK;

}
