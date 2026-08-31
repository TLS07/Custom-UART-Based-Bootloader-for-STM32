
#include"main.h"
extern UART_HandleTypeDef huart2;
#define APP_MAX_SIZE      48128U 



/*
 * This function validates whether the received application header is valid.
 */
BL_Status_t Bootloader_validation_header(const app_header_t *header)
{
    /* Check magic key */
    if (header->magic_key != APP_MAGIC)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER_FAIL : INVALID MAGIC\r\n", strlen("HEADER_FAIL : INVALID MAGIC\r\n"), 100);
        return BL_ERROR;
    }

    /* Check application size */
    if (header->size == 0U)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER_FAIL : INVALID SIZE\r\n", strlen("HEADER_FAIL : INVALID SIZE\r\n"), 100);
        return BL_ERROR;
    }

    if (header->size > APP_MAX_SIZE)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER_FAIL : APP TOO LARGE\r\n", strlen("HEADER_FAIL : APP TOO LARGE\r\n"), 100);
        return BL_ERROR;
    }

    /* Header is valid */
    HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER_OK\r\n", strlen("HEADER_OK\r\n"), 100);

    return BL_OK;
}






//fucntion to validate the application code is valid
int bootloader_is_app_valid(void)
{
    uint32_t HDR_ADDR = APP_HEADER_ADDR;

    const app_header_t *app_hdr =(const app_header_t*)HDR_ADDR;

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