#pragma once

/** @defgroup RTC RTC
 * @{
 *
 * @brief Module for interaction with the RTC
 */

#define BIT(n) (0x01<<(n))

#define IRQ_RTC             8

#define RTC_REG_A           0x0A	
#define RTC_REG_B           0x0B
#define RTC_REG_C           0x0C	

#define RTC_ADDR_REG        0x70
#define RTC_DATA_REG        0x71

#define REG_A_UIP           BIT(7)

#define REG_B_BCD           BIT(2)
#define REG_B_HOUR          BIT(1)
#define REG_B_PERIODIC      BIT(6)
#define REG_B_ALARM         BIT(5)
#define REG_B_UPDATE        BIT(4)

#define REG_C_PF            BIT(6)
#define REG_C_AF            BIT(5)
#define REG_C_UF            BIT(4)

#define SEC 0x00	
#define MINT 0x02	
#define HOUR 0x04	

#define DAY 0x07
#define MONTH 0x08	
#define YEAR 0x09	

/**
 * @brief Converts BCD to decimal
 * 
 * @param bcdByte Byte to convert from BCD to decimal
 * 
 * @return bcdByte conversion to decimal
 */
int decimal (unsigned long bcdByte);

/**
 * @brief Subscribes to RTC interrupts
 * 
 * @return 0 if no errors occur and 1 otherwise
 */
int rtc_subscribe();

/**
 * @brief Unsubscribes to RTC interrupts
 * 
 * @return 0 if no errors occur and 1 otherwise
 */
int rtc_unsubscribe();

/**
 * @brief Checks if RTC is updating
 * 
 * @return 0 if no errors occur and 1 otherwise
 */
int RTC_Update_Check();

/**
 * @brief Checks if RTC is in BCD mode
 * 
 * @return 0 if no errors occur and 1 otherwise
 */
int RTC_Check_BCD();

/**
 * @brief Sets periodic interrupts on RTC
 */
void set_rtc_periodic();

/**
 * @brief Sets alarm interrupts on RTC
 */
void set_rtc_alarm();

/**
 * @brief Sets update interrupts on RTC
 */
void set_rtc_update();

/**
 * @brief Gets year from RTC
 * 
 * @return Current year in decimal
 */
int get_year();

/**
 * @brief Gets month from RTC
 * 
 * @return Current month in decimal
 */
int get_month();

/**
 * @brief Gets day from RTC
 * 
 * @return Current day in decimal
 */
int get_day();

/**
 * @brief Gets hour from RTC
 * 
 * @return Current hour in decimal
 */
int get_hour();

/**
 * @brief Gets minute from RTC
 * 
 * @return Current minute in decimal
 */
int get_min();

/**
 * @brief Gets second from RTC
 * 
 * @return Current second in decimal
 */
int get_sec();

