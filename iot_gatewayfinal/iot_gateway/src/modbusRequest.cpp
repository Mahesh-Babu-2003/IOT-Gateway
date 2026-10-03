#include "application.h"

#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))
#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
#define TWO_BYTE_SWAP_UINT32(x) ((((x) & 0xFFFF0000) >> 16) | (((x) & 0x0000FFFF) << 16))

#define BYTE_TO_BYTE_MAX_INTERVAL 100
#define MB_REG_VALUE_BYTE_COUNT 2
typedef enum
{
    MB_READ_COIL = 0x01,
    MB_READ_DISCRETE_IP = 0x02,
    MB_READ_HOLDING_REGISTER = 0x03,
    MB_READ_INPUT_REGISTER = 0x04,
    MB_WRITE_SINGLE_COIL = 0x05,
    MB_WRITE_SINGLE_REGISTER = 0x06,
    MB_WRITE_MULTIPLE_COIL = 0x0F,
    MB_WRITE_MULTIPLE_REGISTER = 0x10,
} MB_FUNCION_CODE;

typedef enum
{
    MB_ILLEGAL_Function = 0x01,
    MB_ILLEGAL_DATA_ADDRESS,
    MB_ILLEGAL_DATA_VALUE,
    MB_SLAVE_DEVICE_FAILURE,
    MB_ACKNOWLEDGE
} MB_ERROR_CODES;

#define SLAVE_ADDRESS_IX 0
#define FUNCTION_CODE_IX 1
#define START_ADDRESS_IX 2
#define REG_QTY_IX 4
#define WRITE_SINGLE_DATA_IX 4
#define WRITE_MULTIPLE_DATA_COUNT_IX 6
#define WRITE_MULTIPLE_DATA_IX 7

#define MB_MINIMUM_PKT_SIZE 4 // SLAVE_ADDRESS + FC + START _ADDRESS(2)
typedef enum
{
    MB_SLAVE_ADDRESS = 0,
    MB_FUNCTION_CODE,
    MB_START_ADDRESS,
    MB_REG_QUANTITY_COUNT, // for read and multiple write
    MB_BYTE_COUNT,         // for multiple write
    MB_WRITE_DATA,

    MB_CRC_CHECK
} ModbusPktState;
#define MB_PKT_MAX_SIZE 20
 

void mbSendBytes(uint8_t *buffer, uint8_t count)
{
}

void mbSendReadResponse(uint8_t address, uint8_t fcode, uint8_t *buffer, uint8_t byteCount)
{
    uint8_t header_bytes[3] = {address, fcode, byteCount};
    uint16_t crc = ModBus_CRC16(header_bytes, sizeof(header_bytes));
    crc = UpdateModbus_CRC16(crc, buffer, byteCount);

    uint8_t *totalPacket = (uint8_t *)malloc(sizeof(header_bytes) + byteCount + SIZE_OF_CRC);

    uint8_t ix = 0;
    memcpy(&totalPacket[ix], header_bytes, sizeof(header_bytes));
    ix += sizeof(header_bytes);

    memcpy(&totalPacket[ix], buffer, byteCount);
    ix += byteCount;

    memcpy(&totalPacket[ix], (uint8_t *)&crc, SIZE_OF_CRC);
    ix += SIZE_OF_CRC;

    mbSendBytes(totalPacket, ix);

    // Free the allocated memory
    free(totalPacket);
}

void mbSendWriteResponse(uint8_t address, uint8_t fcode, uint16_t *start_address_ptr, uint16_t *reg_qty_or_data_ptr)
{
    uint16_t start_address = SWAP_UINT16(*start_address_ptr);
    uint16_t reg_qty_or_data = SWAP_UINT16(*reg_qty_or_data_ptr);
    uint16_t crc = ModBus_CRC16(&address, 1);
    crc = UpdateModbus_CRC16(crc, &fcode, 1);
    crc = UpdateModbus_CRC16(crc, (uint8_t *)&start_address, sizeof(uint16_t));
    crc = UpdateModbus_CRC16(crc, (uint8_t *)&reg_qty_or_data, sizeof(uint16_t));

    mbSendBytes(&address, 1);
    mbSendBytes(&fcode, 1);
    mbSendBytes((uint8_t *)&start_address, sizeof(uint16_t));
    mbSendBytes((uint8_t *)&reg_qty_or_data, sizeof(uint16_t));
    mbSendBytes((uint8_t *)&crc, SIZE_OF_CRC);

    uint8_t ix = 0;
    uint8_t *totalPacket = (uint8_t *)malloc(1 + 1 + 2 + 2 + 2);

    memcpy(&totalPacket[ix], &address, sizeof(address));
    ix += sizeof(address);
    memcpy(&totalPacket[ix], &fcode, sizeof(fcode));
    ix += sizeof(fcode);
    memcpy(&totalPacket[ix], (uint8_t *)&start_address, sizeof(uint16_t));
    ix += sizeof(uint16_t);
    memcpy(&totalPacket[ix], (uint8_t *)&reg_qty_or_data, sizeof(uint16_t));
    ix += sizeof(uint16_t);
    memcpy(&totalPacket[ix], (uint8_t *)&crc, SIZE_OF_CRC);
    ix += SIZE_OF_CRC;

    mbSendBytes(totalPacket, ix);

    // Free the allocated memory
    free(totalPacket);
}

void mbSendErrorCode(uint8_t address, uint8_t fcode, MB_ERROR_CODES error_code)
{

    uint8_t error_response_pkt[5], ix = 0;
    error_response_pkt[ix++] = address;
    error_response_pkt[ix++] = fcode | 0x80;
    error_response_pkt[ix++] = error_code;
    calculate_and_fill_modbus_crc(error_response_pkt, ix + SIZE_OF_CRC);
    ix += SIZE_OF_CRC;
    mbSendBytes(error_response_pkt, ix);
}

#define MB_READ_INPUT_START_ADDRESS 00
#define MB_READ_INPUT_END_ADDRESS (MB_READ_INPUT_START_ADDRESS + 10)

#define MB_HOLDING_START_ADDRESS 00
#define MB_HOLDING_END_ADDRESS (MB_HOLDING_START_ADDRESS + 2)

uint8_t isValidHoldingRegisterRange(uint16_t address, uint16_t reg_qty)
{
    return (address >= MB_HOLDING_START_ADDRESS) && ((address + reg_qty) <= (MB_HOLDING_END_ADDRESS));
}

uint8_t isReadInputRegisterRange(uint16_t address, uint16_t reg_qty)
{
    return (address >= MB_READ_INPUT_START_ADDRESS) && ((address + reg_qty) <= (MB_READ_INPUT_END_ADDRESS));
}

void process_read_holding_register(uint8_t address, uint16_t *start_address, uint16_t *reg_qty)
{
    (void)*start_address;
    (void)*reg_qty;
    if (isValidHoldingRegisterRange(*start_address, *reg_qty))
    {
        uint8_t total_bytes = *reg_qty * 2;

        uint16_t readings[2] = {0}, ix = 0;
        readings[ix++] = liveObj.do_1_mode;
        readings[ix++] = liveObj.do_1_manual;
        readings[ix++] = liveObj.do_1_persistance_off;
        readings[ix++] = liveObj.do_1_persistance_on;

        mbSendReadResponse(address, MB_READ_HOLDING_REGISTER,
                           (uint8_t *)&readings[*start_address - MB_HOLDING_START_ADDRESS], total_bytes);
    }
}

void process_read_input_register(uint8_t address, uint16_t *start_address, uint16_t *reg_qty)
{
    (void)*start_address;
    (void)*reg_qty;
    if (isReadInputRegisterRange(*start_address, *reg_qty))
    {
        uint8_t total_bytes = *reg_qty * 2;
        float readings[5] = {0};
        uint8_t ix = 0;

        readings[ix++] = liveObj.ai_1_adc;
        readings[ix++] = liveObj.ai_2_adc;
        readings[ix++] = liveObj.ai_1_value;
        readings[ix++] = liveObj.ai_2_value;
        readings[ix++] = liveObj.do_1_value;

        mbSendReadResponse(address, MB_READ_INPUT_REGISTER,
                           (uint8_t *)&readings[*start_address - MB_READ_INPUT_START_ADDRESS], total_bytes);
    }
}

void process_write_multiple_register(uint8_t address, uint16_t *start_address, uint16_t *reg_qty,
                                     uint8_t byte_count, uint8_t *buffer)
{
    (void)*start_address;
    (void)byte_count;
    (void)*reg_qty;
    (void)*buffer;

    if (isValidHoldingRegisterRange(*start_address, *reg_qty))
    {
        if(byte_count == *reg_qty * MB_REG_VALUE_BYTE_COUNT){
            
            uint8_t total_bytes = *reg_qty * 2;
            uint16_t readings[2] = {0}, ix = 0;
            readings[ix++] = liveObj.do_1_mode;
            readings[ix++] = liveObj.do_1_manual;
            readings[ix++] = liveObj.do_1_persistance_off;
            readings[ix++] = liveObj.do_1_persistance_on;


            memcpy((uint8_t *)&readings[*start_address - MB_READ_INPUT_START_ADDRESS], buffer, total_bytes);
            ix = 0;
            liveObj.do_1_mode = readings[ix++];
            liveObj.do_1_manual = readings[ix++];
            liveObj.do_1_persistance_off = readings[ix++];
            liveObj.do_1_persistance_on = readings[ix++];
            saveFormulas();
            mbSendWriteResponse(address, MB_WRITE_MULTIPLE_REGISTER, start_address, reg_qty);
        }
    }
}

void process_modbus_request(uint8_t *pktBuffer, uint16_t pktSize)
{
    if (ModBus_CRC16(pktBuffer, pktSize) == 0)
    {
        uint8_t address = pktBuffer[MB_SLAVE_ADDRESS];
        uint8_t function_code = pktBuffer[MB_FUNCTION_CODE];
        uint16_t start_address = SWAP_UINT16(*((uint16_t *)&pktBuffer[START_ADDRESS_IX]));
        uint16_t reg_qty = SWAP_UINT16(*((uint16_t *)&pktBuffer[REG_QTY_IX]));

        switch (function_code)
        {
        case MB_READ_HOLDING_REGISTER:
            process_read_holding_register(address, &start_address, &reg_qty);
            break;
        case MB_READ_INPUT_REGISTER:
            process_read_input_register(address, &start_address, &reg_qty);
            break;
        case MB_WRITE_MULTIPLE_REGISTER:
            process_write_multiple_register(address, &start_address, &reg_qty,
                                            pktBuffer[WRITE_MULTIPLE_DATA_COUNT_IX], &pktBuffer[WRITE_MULTIPLE_DATA_IX]);
            break;
        case MB_READ_COIL:
        case MB_READ_DISCRETE_IP:
        case MB_WRITE_MULTIPLE_COIL:
        case MB_WRITE_SINGLE_REGISTER:
        case MB_WRITE_SINGLE_COIL:
        default:
            mbSendErrorCode(address, function_code, MB_ILLEGAL_Function);
            break;
        }
    }
}
