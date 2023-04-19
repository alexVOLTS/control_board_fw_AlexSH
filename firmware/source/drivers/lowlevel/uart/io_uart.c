/**
 ******************************************************************************
 * @file           : io_uart.c
 * @author         : Aleksandr Shabalin       <alexnv97@gmail.com>
 * @brief          : IO_Uart usage file
 ******************************************************************************
 * ----------------- Copyright (c) 2022 Aleksandr Shabalin------------------- *
 ******************************************************************************
 ******************************************************************************
 */


/******************************************************************************/
/* Includes ----------------------------------------------------------------- */
/******************************************************************************/
#include "io_uart.h"

/******************************************************************************/
/* Private variables -------------------------------------------------------- */
/******************************************************************************/
struct uart io_uart;


/******************************************************************************/


/**
 * @brief          I/O uart init
 */
void IoUartInit(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_USART_InitTypeDef USART_InitStruct = {0};

  IOUART_ENABLE_CLOCK();
  __DSB();

  GPIO_InitStruct.Pin        = IOUART_TX_Pin | IOUART_RX_Pin;
  GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate  = IOUART_GPIO_AF;
  LL_GPIO_Init(IOUART_Port, &GPIO_InitStruct);

  /* UART interrupt Init */
  NVIC_SetPriority(IOUART_IRQn, 5);
  NVIC_EnableIRQ(IOUART_IRQn);

  LL_USART_EnableIT_RXNE(IOUART_Periph);
  LL_USART_EnableIT_ERROR(IOUART_Periph);
  LL_USART_EnableIT_IDLE(IOUART_Periph);

  USART_InitStruct.BaudRate            = 115200;
  USART_InitStruct.DataWidth           = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits            = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity              = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(IOUART_Periph, &USART_InitStruct);

  LL_USART_DisableIT_CTS(IOUART_Periph);
  LL_USART_ConfigAsyncMode(IOUART_Periph);

  LL_USART_Enable(IOUART_Periph);
}
/******************************************************************************/



















