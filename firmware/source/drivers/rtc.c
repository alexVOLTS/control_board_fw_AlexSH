/**
 ******************************************************************************
 * @file           : rtc.c
 * @author         : Aleksandr Shabalin       <alexnv97@gmail.com>
 * @brief          : RTC driver
 ******************************************************************************
 * ----------------- Copyright (c) 2023 Aleksandr Shabalin------------------- *
 ******************************************************************************
 ******************************************************************************
 */

/******************************************************************************/
/* Includes ----------------------------------------------------------------- */
/******************************************************************************/
#include <stdlib.h>

#include "rtc.h"
#include "rtc_i2c.h"


/******************************************************************************/
/* Private defines ---------------------------------------------------------- */
/******************************************************************************/


/******************************************************************************/
/* Private variables -------------------------------------------------------- */
/******************************************************************************/
osThreadId_t RtcTaskHandle;

const osThreadAttr_t RtcTask_attributes = {
    .name = "RtcTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

RTC_INFO_t rtc_info;
bool rtc_ok;


/******************************************************************************/
/* Private function prototypes ---------------------------------------------- */
/******************************************************************************/
uint8_t prvGetDate(RTC_DATE_t *date);
uint8_t prvSetDate(RTC_DATE_t *date);
uint8_t prvCheckDate(RTC_DATE_t *date);
uint8_t prvCheckTime(RTC_TIME_t *time);
uint8_t prvSetTime(RTC_TIME_t *time);
uint8_t prvGetTime(RTC_TIME_t *time);


/******************************************************************************/


/**
 * @brief          RTC task
 * @param[in]      argument: Pointer to *argument for FreeRTOS
 */
void RtcTask(void *argument)
{
  uint8_t error = RTC_OK;

  error = RtcInit();

  if (error != RTC_OK)
  {
    RtcSetError(error);
    return;
  }

  for (;;)
  {

    if (RtcGetError() != RTC_OK)
    {
      RtcErrorHandler(rtc_info.error);
      RtcSetError(RTC_OK);
      continue;
    }

    switch (RtcGetStatus())
    {
      case RTC_GET_DATE:
        error = RtcGetDate();
        break;
      case RTC_GET_TIME:
        error = RtcGetTime();
        break;
      case RTC_IDLE:
        __NOP();
        break;
    }

    if (error != RTC_OK)
    {
      RtcSetError(error);
      continue;
    }
  }
}
/******************************************************************************/




/**
 * @brief          RTC init task
 */
void RtcInitTask(void)
{
  RtcTaskHandle = osThreadNew(RtcTask, NULL, &RtcTask_attributes);
}
/******************************************************************************/




/**
 * @brief          RTC initialization
 * @return         Current error instance
 */
uint8_t RtcInit(void)
{
  uint8_t dummy;

  if (RtcI2cInit() != RTC_OK)
    return RTC_INIT_ERROR;

  memset(&rtc_info, 0x00, sizeof(rtc_info));
  rtc_ok = false;

  //if (RtcI2cReadBufferInterrupt(RTC_HW_ADDRESS, ADDR_BYTE, 0, &dummy, 1) == RTC_OK)
  if (RtcI2cReadByte(RTC_HW_ADDRESS, 0, &dummy, 1) == RTC_OK)
    rtc_ok = true;

  PROJ_UNUSED(dummy);

  osDelay(100);

  if (rtc_ok)
    RtcI2cWriteByte(RTC_HW_ADDRESS, RTC_REG_CONTROL, 0, 1);
    //RtcI2cWriteBufferInterrupt(RTC_HW_ADDRESS, ADDR_BYTE, RTC_REG_CONTROL, 0, 1);
  else
    return RTC_INIT_ERROR;

  if (RtcSetStatus(RTC_IDLE) != RTC_OK)
    return RTC_INIT_ERROR;

  return RTC_OK;
}
/******************************************************************************/




/**
 * @brief          RTC set current date
 * @param[in]      buf: Pointer to @ref buffer with input date
 * @return         Current error instance
 */
uint8_t RtcSetDate(char *buf)
{
  uint8_t res = 0x00;
  RTC_DATE_t date;

  memcpy(rtc_info.date_buf, buf, 10);

  rtc_info.date_buf[2] = 0;
  rtc_info.date_buf[5] = 0;
  rtc_info.date_buf[10] = 0;

  if (rtc_info.date_buf == NULL)
    return RTC_DATE_BUFFER_ERROR;

  date.date = atoi(&buf[0]);
  date.month = atoi(&buf[3]);
  date.year = atoi(&buf[6]) % 100;

  res = prvCheckDate(&date);

  if (res != RTC_OK)
    return RTC_CHECK_DATE_ERROR;

  res = RtcI2cSetDate(&date);

  if (res == RTC_OK)
    PrintfConsoleCRLF(CLR_DEF"Date set "CLR_GR"successful"CLR_DEF);

  if (RtcI2cSetMode(RTC_I2C_IDLE) != RTC_OK)
    return RTC_TRANSMIT_ERROR;

  return res;
}
/******************************************************************************/




/**
 * @brief          RTC get current date
 * @return         Current error instance
 */
uint8_t RtcGetDate(void)
{
  uint8_t res = 0x00;

  res = RtcI2cGetDate(&rtc_info.date);

  if(res == RTC_OK)
    PrintfConsoleCRLF(CLR_DEF"Date: %02u.%02u.%04u", rtc_info.date.date, rtc_info.date.month, rtc_info.date.year);

  RtcSetStatus(RTC_IDLE);

  if (RtcI2cSetMode(RTC_I2C_IDLE) != RTC_OK)
    return RTC_RECEIVE_ERROR;

  return res;
}
/******************************************************************************/




/**
 * @brief          RTC set current time
 * @param[in]      buf: Pointer to buffer with input time
 * @return         Current error instance
 */
uint8_t RtcSetTime(char *buf)
{
  uint8_t res = 0x00;
  RTC_TIME_t time;

  memcpy(rtc_info.time_buf, buf, 8);

  rtc_info.time_buf[2] = 0;
  rtc_info.time_buf[5] = 0;
  rtc_info.time_buf[8] = 0;

  if (rtc_info.time_buf == NULL)
    return RTC_TIME_BUFFER_ERROR;

  time.hours   = atoi(&buf[0]);
  time.minutes = atoi(&buf[3]);
  time.seconds = atoi(&buf[6]);

  res = prvCheckTime(&time);

  if (res != RTC_OK)
    return RTC_CHECK_TIME_ERROR;

  res = RtcI2cSetTime(&time);

  if (res == RTC_OK)
    PrintfConsoleCRLF(CLR_DEF"Time set "CLR_GR"successful"CLR_DEF);

  if (RtcI2cSetMode(RTC_I2C_IDLE) != RTC_OK)
    return RTC_TRANSMIT_ERROR;

  return res;
}
/******************************************************************************/




/**
 * @brief          RTC get current time
 * @return         Current error instance
 */
uint8_t RtcGetTime(void)
{
  uint8_t res = 0x00;

  res = RtcI2cGetTime(&rtc_info.time);

  if (res == RTC_OK)
    PrintfConsoleCRLF("\t"CLR_GR"RTC time %02u:%02u:%02u.%03u"CLR_DEF, rtc_info.time.hours, rtc_info.time.minutes,
        rtc_info.time.seconds, rtc_info.time.ms);

  RtcSetStatus(RTC_IDLE);

  if (RtcI2cSetMode(RTC_I2C_IDLE) != RTC_OK)
    return RTC_RECEIVE_ERROR;

  return res;
}
/******************************************************************************/




/**
 * @brief          RTC set current status
 * @param[in]      status: status which need to be set
 * @return         Current error instance
 */
uint8_t RtcSetStatus(RTC_STATUS_t status)
{
  if (status > RTC_NUM_OF_STATES)
    return RTC_SET_STATUS_ERROR;

  rtc_info.status = status;

  return RTC_OK;
}
/******************************************************************************/




/**
 * @brief          RTC get current status
 * @return         rtc_info.status: current status instance
 */
RTC_STATUS_t RtcGetStatus(void)
{
  return rtc_info.status;
}
/******************************************************************************/




/**
 * @brief          RTC set current error
 * @param[in]      error: error which need to be set
 */
uint8_t RtcSetError(RTC_ERROR_t error)
{
  if (error > RTC_NUM_OF_ERRORS)
    return RTC_SET_ERROR;

  rtc_info.error = error;

  return RTC_OK;
}
/******************************************************************************/




/**
 * @brief          RTC get current error
 * @return         rtc_info.error current instance
 */
RTC_ERROR_t RtcGetError(void)
{
  return rtc_info.error;
}
/******************************************************************************/




uint8_t prvCheckDate(RTC_DATE_t *date)
{
  uint8_t status = RTC_OK;

  if ((date->date < 1) || (date->date > 31))
    status = RTC_CHECK_DATE_ERROR;
  else if ((date->month < 1) || (date->month > 12))
    status = RTC_CHECK_DATE_ERROR;
  else if ((date->year < 0) || (date->year > 99))
    status = RTC_CHECK_DATE_ERROR;

  return status;
}
/******************************************************************************/




uint8_t prvCheckTime(RTC_TIME_t *time)
{
  uint8_t status = RTC_OK;

  if ((time->hours < 0) || (time->hours > 23))
    status = RTC_CHECK_TIME_ERROR;
  if ((time->minutes < 0) || time->minutes > 59)
    status = RTC_CHECK_TIME_ERROR;
  if ((time->seconds < 0) || time->seconds > 59)
    status = RTC_CHECK_TIME_ERROR;

  return status;
}
/******************************************************************************/




/**
 * @brief          RTC error handler
 * @param[in]      RTC_ERROR_t instance error
 */
void RtcErrorHandler(RTC_ERROR_t error)
{
  switch (error)
  {
    case RTC_OK:
      break;
    case RTC_INIT_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"INIT"CLR_DEF);
      break;
    case RTC_I2C_INIT_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC I2C: "CLR_RD"LOW LEVEL I2C INIT"CLR_DEF);
    case RTC_TRANSMIT_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"TRANSMIT"CLR_DEF);
      break;
    case RTC_RECEIVE_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"RECEIVE"CLR_DEF);
      break;
    case RTC_I2C_TRANSMIT_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC I2C: "CLR_RD"TRANSMIT"CLR_DEF);
      break;
    case RTC_I2C_RECEIVE_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC I2C: "CLR_RD"RECEIVE"CLR_DEF);
      break;
    case RTC_SET_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"SET MODE STATUS"CLR_DEF);
      break;
    case RTC_SET_LL_I2C_MODE_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC I2C: "CLR_RD"SET MODE OF LL I2C"CLR_DEF);
      break;
    case RTC_SET_STATUS_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"SET MODE STATUS"CLR_DEF);
      break;
    case RTC_CHECK_DATE_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"DATE FORMAT IS NOT CORRECT"CLR_DEF);
      break;
    case RTC_CHECK_TIME_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"TIME FORMAT IS NOT CORRECT"CLR_DEF);
      break;
    case RTC_DATE_BUFFER_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"SETTING BUFFER FOR DATE"CLR_DEF);
      break;
    case RTC_TIME_BUFFER_ERROR:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"SETTING BUFFER FOR TIME"CLR_DEF);
      break;
    default:
      PrintfConsoleCRLF("\t"CLR_DEF"ERROR RTC: "CLR_RD"UNDEFINED"CLR_DEF);
      break;
  }
}
/******************************************************************************/

