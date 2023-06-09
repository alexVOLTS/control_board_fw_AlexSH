/**
 ******************************************************************************
 * @file           : dma.h
 * @author         : Konstantin Soloviev
 * @author         : Dmitry Karasev        <karasev@voltsbattery.com>
 * @author         : Aleksandr Shabalin
 * @brief          : Header for the DMA part of the I2C1 and USART5 drivers.
 *                   It contains global variables, functions and defines.
 ******************************************************************************
 * ----------------- Copyright (c) 2023 ------------------------------------- *
 ******************************************************************************
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _DRV_DMA_H_
#define _DRV_DMA_H_


/******************************************************************************/
/* Includes ----------------------------------------------------------------- */
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "cmsis_os.h"
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/
/* Public defines ----------------------------------------------------------- */
/******************************************************************************/



/******************************************************************************/
/* Public variables --------------------------------------------------------- */
/******************************************************************************/
extern volatile bool i2c1_dma_tx;

extern osSemaphoreId Dma174_TxSemaphoreHandle;
extern osMutexId dma174_MutexHandle;


/******************************************************************************/
/* Public functions --------------------------------------------------------- */
/******************************************************************************/
void DMA_Init174(void);
void DMA_ConfigTxI2C1(volatile void *buf, uint16_t len);
void DMA_ConfigTxUART5(volatile void *buf, uint16_t len);


/******************************************************************************/


#ifdef __cplusplus
}
#endif


#endif  /* _DRV_DMA_H_ */
