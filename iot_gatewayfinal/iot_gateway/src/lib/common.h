/*
 * common.h
 *
 *  Created on: Jul 24, 2020
 *      Author: VGS TECHNOLOGIES
 */

#ifndef CUSTOM_INC_COMMON_H_
#define CUSTOM_INC_COMMON_H_
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "my_crc.h"
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>


#define	STM32_CONTROLLER_TARGET			1
#define	RENESAS_CONTROLLER_TARGET		2
#define	TI_CONTROLLER_TARGET			3
#define	PIC_CONTROLLER_TARGET			4

#define TARGET_CONTROLLER				STM32_CONTROLLER_TARGET

#if(TARGET_CONTROLLER != STM32_CONTROLLER_TARGET)
typedef enum
{
	GPIO_PIN_RESET = 0U,
	GPIO_PIN_SET
} GPIO_PinState;

typedef enum
{
  HAL_OK       = 0x00,
  HAL_ERROR    = 0x01,
  HAL_BUSY     = 0x02,
  HAL_TIMEOUT  = 0x03,
  HAL_NOT_FOUND  = 0xFF,
} HAL_StatusTypeDef;
#endif

#define PIN_ACTIVE_LOW	0
#define PIN_ACTIVE_HIGH	1

#define ENABLE_OP			1
#define DISABLE_OP			0

#define PIN_RESET 	GPIO_PIN_RESET
#define PIN_SET 	GPIO_PIN_SET

#ifndef PI
#define PI	3.1415926
#endif
#define SIZE_OF_CRC						sizeof(uint16_t)
#define MAX_VALUE_PRECISION 3

#define DECIMAL_TO_BCD(X)      ( ( (X/10) << 4 ) + (X%10) )
#define BCD_TO_DECIMAL(X)      ( ( (X>>4) * 10) + ( X & 0x0F ) )

#define VOLTAGE_TO_MILLI_VOLTS      	1000U

#define NUMBER_OF_MILLI_SECONDS_PER_SEC      	1000U
#define HUNDREAD_MSEC_TO_MSEC					100
#define HALF_SEC_TO_MSEC						500
#define SEC_TO_MSEC								1000
#define NUMBER_OF_MSECS_PER_MIN       	60000U
#define NUMBER_OF_SECONDS_PER_MIN       60U
#define NUMBER_OF_MINS_PER_HOUR         60U
#define NUMBER_OF_SECONDS_PER_HOUR      3600U
#define NUMBER_OF_HOURS_PER_DAY         24U
#define NUMBER_OF_MINUTES_PER_DAY       (NUMBER_OF_HOURS_PER_DAY * NUMBER_OF_MINS_PER_HOUR)
#define NUMBER_OF_DAYS_PER_WEEK         7U
#define NUMBER_OF_MONTHS_PER_YEAR       12U
#define MAXIMUM_MINUTES_PER_MONTH       (31U * NUMBER_OF_MINUTES_PER_DAY)

#define RTC_DATE_TIME_LENGTH            6      //YY/MM/DD HH:MM:SS
#define RTC_DATE_LENGTH                 3      //YY/MM/DD
#define RTC_TIME_LENGTH                 3      //HH:MM:SS

#define RTC_DRIFT_ALLOWED_TIME		600UL //drift allowed time
#define APP_START_YEAR				2000
#define RTC_BASE_YEAR				1900

#define NUMBER_OF_SEC_PER_DAY		86400
#define DATE_TIME_OPTIONAL_VALUE        0xFFU
typedef enum{


  DATE_TIME_YEAR_INDEX = 0, DATE_TIME_MONTH_INDEX = 1, DATE_TIME_DATE_INDEX = 2,
  DATE_TIME_HOUR_INDEX = 3, DATE_TIME_MINUTE_INDEX = 4, DATE_TIME_SECOND_INDEX = 5,

  DATE_YEAR_INDEX = 0, DATE_MONTH_INDEX = 1, DATE_DATE_INDEX = 2, DATE_DAY_INDEX = 3,

  TIME_HOUR_INDEX = 0, TIME_MINUTE_INDEX = 1, TIME_SECOND_INDEX = 2, TIME_MILLI_SECOND_INDEX = 3,
}DATE_TIME_INDEX;

typedef enum{
  SUN_DAY = 0, MON_DAY, TUES_DAY, WEDNES_DAY, THURS_DAY, FRI_DAY, SATUR_DAY, WEEK_DAY_COUNT
}WEEK_DAY;

typedef struct __attribute__((__packed__)) {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t date;
	uint8_t month;
	uint8_t year;
} Custom_Date_Time;

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t dayOfWeek;
    uint8_t date;
    uint8_t month;
    uint8_t year;
}RTC_DATE_TIME_DOW;

typedef uint8_t  RTC_DATE_TIME[RTC_DATE_TIME_LENGTH];
typedef uint8_t  RTC_DATE[RTC_DATE_LENGTH];
typedef uint8_t  RTC_TIME[RTC_TIME_LENGTH];

uint32_t day_timestamp(uint32_t timestamp);
bool validate_date(uint8_t* buffer, bool is_reverse);
bool validate_date_time(uint8_t* buffer, bool is_reverse);
bool validate_time(uint8_t* buffer, bool is_reverse);
uint32_t getTimestamp(int year, int month, int date, int hour, int min, int sec);

bool isValidDateTime(RTC_DATE_TIME_DOW rtcObj);
int convertRTCDateTimeToString(char* buffer, RTC_DATE_TIME_DOW* currentDateTime);

uint32_t getDiffrence(uint32_t min, uint32_t max);
uint64_t get_abs_diffrence(uint64_t val1, uint64_t val2);
uint16_t getHighestOfSamples(uint16_t* samples, uint16_t sampleCount);
uint16_t getPeekToPeekOfSamples(uint16_t* samples, uint16_t sampleCount);
uint16_t getAverageOfSamples(uint16_t* samples, uint16_t sampleCount);

double compute_rms_with_cluster(uint16_t *samples, uint16_t sampleCount, uint16_t mean_adc,
		uint16_t cluster_sample_count, uint16_t cluster_offset);
double compute_rms_with_samples(uint16_t *samples, uint16_t sampleCount, uint16_t mean_adc);
bool find_mean_from_peak_to_peak(uint16_t *samples, uint16_t sampleCount, uint16_t* meanValue);


void replace_character(char* data_ptr, uint16_t length, uint8_t old_character, uint8_t new_character);
uint16_t calc_character_count(char* s, uint16_t length, uint8_t data) ;


bool isTimeInWithInBoundaryTimings(uint32_t current_time, uint32_t start_time, uint32_t end_time);

void fill_sin_values(uint16_t* buffer, uint16_t sample_count, uint16_t max_value);
void fill_triangle_values(uint16_t* buffer, uint16_t sample_count, uint16_t max_value);


void printTobaseWithPadding (long n, uint8_t base, uint8_t* destination, uint8_t padding_bytes, uint8_t padding_byte, uint8_t decimal_point);
void printTobase (long n, uint8_t base, uint8_t* destination);
unsigned short crc16(const uint8_t* data_p, uint16_t length) ;

void memcpy_reverse(void* dest, const void* src, uint16_t length);

void bytes_to_value(void* dest, const void* src, uint8_t length);
int64_t bytes_to_singed_value(const void* src, uint8_t length);
uint64_t bytes_to_unsinged_value(const void* src, uint8_t length);

void value_to_bytes(void* dest, const void* src, uint8_t length);
void unsigned_value_to_hex(uint64_t value, uint8_t* hexbuf, uint8_t length);
void signed_value_to_hex(int64_t value, uint8_t* hexbuf, uint8_t length);
void long8byteshex(uint64_t value, uint8_t* hexbuf);
void long4byteshex(uint32_t value, uint8_t* hexbuf);
void long2byteshex(uint16_t value, uint8_t* hexbuf);

uint16_t calculate_crc_ccitt(uint16_t seed, uint16_t polynomial,
		const uint8_t *data, size_t length);
void iso14443a_crc(uint8_t *pbtData, size_t szLen, uint8_t *pbtCrc);
void calculate_and_fill_iso14443a_crc(unsigned char * Buffer, unsigned short Length);
uint16_t calculate_iso14443a_crc(uint8_t *pbtData, size_t szLen);
unsigned short ModBus_CRC16( unsigned char * Buffer, unsigned short Length );
unsigned short CalculatCustom_CRC16(unsigned short cur_seed, unsigned short cur_polynomial,
		unsigned char * Buffer, unsigned short Length );
uint8_t validate_custom_crc(unsigned short cur_seed, unsigned short cur_polynomial,
		uint8_t *date_ptr, uint16_t length);
void calculate_and_fill_custom_crc(unsigned short cur_seed, unsigned short cur_polynomial,
		unsigned char * Buffer, unsigned short Length);
uint16_t UpdateModbus_CRC16(uint16_t  usCRC16,  uint8_t *aucData, uint16_t usInputLength);
void calculate_and_fill_modbus_crc(uint8_t *data_ptr, uint16_t length) ;

void getValueToString(int64_t value, char* buffer, uint8_t length);
void convertUpperCaseToLower(unsigned char* dest, unsigned char* src, uint16_t length);
void convertLowerCaseToUpper(unsigned char* dest, unsigned char* src, uint16_t length);
unsigned char char2hex(unsigned char convChar);
uint8_t isAlphaNumericCharacter(uint8_t byte);
uint8_t convertCharToDecimal(uint8_t byte);
uint8_t convertHexStringByteToDecimal(const uint8_t* buffer);
uint64_t convertHexStringBytesToDecimal(const uint8_t* buffer, uint8_t byteCnt);
uint64_t convertASCIIStringBytesToDecimal(const uint8_t* buffer, uint8_t byteCnt);
void convertHexStringBytesToHexBytes(uint8_t* hex_buffer, const uint8_t* hex_string_buffer, uint8_t byteCnt);
int32_t convertStringToValue(char* str);
unsigned short convertToFloat(char* data, float *result);
uint16_t getNthFeild(uint8_t* src_buffer, uint8_t* dest_buffer, uint8_t separator,
                                            uint8_t feild_id, uint16_t src_length, uint16_t dest_length);

char get_segment_by_index(char *data_in, uint8_t index, char *segment, uint8_t max_len);

float get_value_based_on_percentage(float full_value, float percentage);

void * swapbytes(void *inp, size_t len);
void * swapbytesout(void * inp, void *out, size_t len);
void print_in_hex(uint8_t *buffer, uint16_t len) ;
uint32_t getNextLocationID(uint32_t location, uint32_t noOfRecords);
uint32_t getLocationAddress(uint32_t start_address, uint32_t record_id, uint32_t record_size);
uint16_t get_avg_Of_uint16(uint16_t* buffer, uint16_t length, uint16_t cluster_count, uint16_t cluster_offset);

typedef union __attribute__((__packed__)){
	uint8_t u8_value;
	int8_t i8_value;
	uint16_t u16_value;
	int16_t i16_value;
	uint32_t u32_value;
	int32_t i32_value;
    uint64_t u64_value;
    int64_t i64_value;
	float f_value;
	double d_value;
}REGISTER_VALUE;

typedef enum{
	LITTLE_ENDIAN_ = 0, BIG_ENDIAN_, MID_BIG_ENDIAN_, MID_LITTLE_ENDIAN_, MAX_BYTE_ORDER
}Byte_Order;

typedef enum{
	NO_TYPE = 0, BIT_TYPE,
	INT8_TYPE, UINT8_TYPE,
	INT16_TYPE, UINT16_TYPE,
	INT32_TYPE, UINT32_TYPE,
	INT64_TYPE, UINT64_TYPE,
	FLOAT_TYPE, DOUBLE_TYPE,
	STRING_TYPE,
	IP_ADDRESS_TYPE, MAC_ADDRESS_TYPE, SUBNET_MASK_TYPE,
	DATE_TYPE, TIME_TYPE,
	TIMESTAMP_TYPE,
	MAX_DATA_TYPE
}DATA_TYPE;
uint8_t dataTypeByteCount(DATA_TYPE data_type) ;
void copyBytesToValueAndUpdateByteOrder(uint8_t* dest, uint8_t* src, uint8_t src_byte_count, DATA_TYPE data_type, Byte_Order byte_order);
void convertByteOrder(uint8_t* dest_bytes, uint8_t* src_bytes, uint8_t byteCount, Byte_Order byteOrder);

typedef enum {
	NO_ACCESS, WRITE_ACCESS, READ_ACCESS, READ_WRITE_ACCESS
}AccessType;

typedef enum{
	NONE_PARITY, EVEN_PARITY, ODD_PARITY, MAX_UART_PARITY
}UART_PARITY;

typedef enum{
    COMMAND_PENDING = -1,
    COMMAND_OK = 0,
    COMMAND_COMMUNICATION_ERROR,
    COMMAND_NOT_RESPONDING,
    COMMAND_INVALID_GROUP,
    COMMAND_INVALID_PARAMETERS,
    COMMAND_INVALID_DATA,
    COMMAND_SETTING_FAIL,
}CommandStatus;
#endif /* CUSTOM_INC_COMMON_H_ */
