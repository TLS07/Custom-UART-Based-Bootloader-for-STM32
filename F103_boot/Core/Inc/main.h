/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>

typedef enum
{
	BL_OK=0,
	BL_ERROR
}BL_Status_t;

void JumpToApplication(void);
int bootloader_is_app_valid(void);
void Bootloader_UpdateMode(void);
#include "flash_layout.h"
#include "app_header.h"
/* USER CODE BEGIN EFP */
BL_Status_t Bootloader_validation_header(const app_header_t *header);
BL_Status_t Bootloader_EraseApplication(void);
BL_Status_t Bootloader_ReceiveFirmware(const app_header_t *header);
BL_Status_t Bootloader_WriteChunk(uint32_t flash_address,uint8_t *buffer, uint32_t length);
BL_Status_t Bootloader_VerifyFirmwareCRC(const app_header_t *header);
int bootloader_is_app_valid(void);
uint32_t Bootloader_CalculateCRC(uint32_t start_address, uint32_t length);
BL_Status_t Bootloader_EraseHeader(void);
BL_Status_t Bootloader_WriteHeader(const app_header_t *header);
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
BL_Status_t Bootloader_validation_header(const app_header_t *header);
BL_Status_t Bootloader_EraseApplication(void);
BL_Status_t Bootloader_ReceiveFirmware(const app_header_t *header);
BL_Status_t Bootloader_WriteChunk(uint32_t flash_address,uint8_t *buffer, uint32_t length);
BL_Status_t Bootloader_VerifyFirmwareCRC(const app_header_t *header);
int bootloader_is_app_valid(void);
uint32_t Bootloader_CalculateCRC(uint32_t start_address, uint32_t length);
BL_Status_t Bootloader_EraseHeader(void);
BL_Status_t Bootloader_WriteHeader(const app_header_t *header);
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
