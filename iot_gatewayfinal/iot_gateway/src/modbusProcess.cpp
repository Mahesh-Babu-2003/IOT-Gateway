#include "application.h"

#define NUMBER_OF_BITS_IN_BYTE 8
#define MODBUS_TRANSACTION_LOG 0

CommunicationResponse SendRequestWaitResponse(uint8_t address, uint8_t *bytes, uint16_t length, uint16_t timeOut)
{
#define MAX_NUMBER_FAILURE_ATTEMPTS 2
    CommunicationResponse response;
    int noOfAttempts = 0;
    do
    {
#if (MODBUS_TRANSACTION_LOG == 1)
        print_bytes_in_hex(((String) "Address" + address + " Tx_" + noOfAttempts + ": ").c_str(), bytes, length);
#endif
        if (response.byteCount > 0)
        {
            response.byteCount = 0;
        }
        digitalWrite(RS485_EN_OP, HIGH);
        delay(1);
        Serial1.write(bytes, length);
        Serial1.flush();
        Serial.flush();
        delay(1);
        digitalWrite(RS485_EN_OP, LOW);

        uint32_t timestamp = millis(), byteTimestamp = millis();
        // Serial.println((String)"Timestamp start " + timestamp + " " + timeOut);
        while (get_abs_diffrence(timestamp, millis()) < timeOut)
        {
            if (response.byteCount > 0 && get_abs_diffrence(byteTimestamp, millis()) > BYTE_TO_BYTE_TIME_OUT)
            {
                break;
            }
            processLoops();
            while (Serial1.available() > 0)
            {
                if (response.byteCount == 0)
                {
                    timestamp = millis();
                }
                int data = Serial1.read();
                // Serial.print((String) + data + ' ');
                if ((response.byteCount < MB_MAX_PACKET_SIZE) && (response.byteCount > 0 || data == address))
                {
                    response.bytes[response.byteCount++] = data;
                    response.result = true;
                }
                byteTimestamp = millis();
            }
            delay(1);
        }
        ++noOfAttempts;
    } while ((response.result == false) && (noOfAttempts < MAX_NUMBER_FAILURE_ATTEMPTS));
    // Serial.println((String)"Timestamp end " + millis());
    processLoops();

#if (MODBUS_TRANSACTION_LOG == 1)
    if (response.byteCount > 0)
    {
        print_bytes_in_hex("Rx", response.bytes, response.byteCount);
    }
#endif

    return response;
}


bool sendMeterWriteRequest(uint8_t modbus_address, 
		uint8_t fcode,
		uint8_t data_type, uint8_t byte_order, uint16_t register_address,
		REGISTER_VALUE value) {
	uint8_t attempts = 0;
	uint32_t execution_time = 0; 
	uint8_t dataTypeLength = dataTypeByteCount((DATA_TYPE)data_type);
	REGISTER_VALUE mapped_value; 
    uint8_t response_code;
 

	copyBytesToValueAndUpdateByteOrder((uint8_t*) &mapped_value,
			(uint8_t*) &value, dataTypeLength, (DATA_TYPE)data_type, (Byte_Order)byte_order);

    uint8_t packet[64] = { 0 }, pktIx = 0;
    packet[pktIx++] = modbus_address;
    packet[pktIx++] = fcode;
    packet[pktIx++] = (register_address>>8) & 0xFFU;
    packet[pktIx++] = (register_address) & 0xFFU;
    memcpy(&packet[pktIx], &mapped_value, dataTypeLength); 
    pktIx += dataTypeLength;
    ushort crc16 = ModBus_CRC16(packet, pktIx);

    print_bytes_in_hex("pkt", packet, pktIx);
    Serial.println((String) "pktIx: " + pktIx);
    Serial.println((String) "crc16: " + crc16);

    packet[pktIx++] = (uint8_t)crc16;
    packet[pktIx++] = (uint8_t)(crc16 >> 8); 
	
    CommunicationResponse response = SendRequestWaitResponse(modbus_address, packet, pktIx, 1000);
	return response.result;
}

uint16_t ReadModbusPkt(uint8_t *modbusPkt, uint8_t id, byte functionCode, ushort registerAddr, int count,
                       bool crcRequire, bool tcpHeader, ushort transactionId)
{
    uint16_t pktLength = 0;

    // Serial.println( "ReadModbusPkt: " + (String)id + " " +
    //     (String)functionCode + " " + (String)registerAddr + " " + (String)count  +  " " + (String)tcpHeader);
    //  Serial.flush();

    if (tcpHeader)
    {
        // MBAP Header
        modbusPkt[pktLength++] = (uint8_t)(transactionId >> 8); // Master transaction identifier
        modbusPkt[pktLength++] = (uint8_t)transactionId;
        modbusPkt[pktLength++] = 0; // protocol
        modbusPkt[pktLength++] = 0;
        modbusPkt[pktLength++] = 0; // length
        if (crcRequire)
        {
            modbusPkt[pktLength++] = 8;
        }
        else
        {
            modbusPkt[pktLength++] = 6;
        }
    }

    modbusPkt[pktLength++] = id;
    modbusPkt[pktLength++] = functionCode;
    modbusPkt[pktLength++] = (uint8_t)(registerAddr >> 8);
    modbusPkt[pktLength++] = (uint8_t)registerAddr;
    modbusPkt[pktLength++] = (uint8_t)(count >> 8);
    modbusPkt[pktLength++] = (uint8_t)count;
    if (crcRequire == true)
    {
        ushort crc16 = ModBus_CRC16(modbusPkt, pktLength);
        modbusPkt[pktLength++] = (uint8_t)crc16;
        modbusPkt[pktLength++] = (uint8_t)(crc16 >> 8);
    }

    // print_bytes_in_hex("Request Pkt", modbusPkt.data(), modbusPkt.size());

    return pktLength;
}


uint16_t WriteSingleModbusPkt(uint8_t *modbusPkt, uint8_t id, uint8_t regByteCount, uint8_t functionCode, ushort registerAddr, ushort value,
                              bool crcRequire, bool tcpHeader, ushort transactionId)
{
    uint16_t pktLength = 0;
    if (tcpHeader)
    {
        // MBAP Header
        modbusPkt[pktLength++] = (uint8_t)(transactionId >> 8); // Master transaction identifier
        modbusPkt[pktLength++] = (uint8_t)transactionId;
        modbusPkt[pktLength++] = 0; // protocol
        modbusPkt[pktLength++] = 0;
        modbusPkt[pktLength++] = 0; // length
        uint8_t length = 4 + regByteCount;
        if (crcRequire)
        {
            length += MODBUS_CRC_LENGTH;
        }
        modbusPkt[pktLength++] = length;
    }
    modbusPkt[pktLength++] = id;
    modbusPkt[pktLength++] = functionCode; // 0x03 READ Holding,
    modbusPkt[pktLength++] = (uint8_t)(registerAddr >> 8);
    modbusPkt[pktLength++] = (uint8_t)registerAddr;
    if (regByteCount == 4)
    {
        modbusPkt[pktLength++] = (uint8_t)(value >> 24);
        modbusPkt[pktLength++] = (uint8_t)(value >> 16);
    }
    modbusPkt[pktLength++] = (uint8_t)(value >> 8);
    modbusPkt[pktLength++] = (uint8_t)value;

    if (crcRequire == true)
    {
        ushort crc16 = ModBus_CRC16(modbusPkt, pktLength);
        modbusPkt[pktLength++] = (uint8_t)crc16;
        modbusPkt[pktLength++] = (uint8_t)(crc16 >> 8);
    }

    return pktLength;
}

uint16_t WriteMultipleModbusPkt(uint8_t *modbusPkt, uint8_t id, uint8_t regByteCount, uint8_t functionCode, ushort registerAddr, 
                                uint8_t* body, uint8_t bodyLength,
                                bool crcRequire, bool tcpHeader, ushort transactionId,
                                bool isTcpHeaderByteCountRequired)
{
    uint16_t pktLength = 0;

    if (tcpHeader)
    {
        // MBAP Header
        modbusPkt[pktLength++] = (uint8_t)(transactionId >> 8); // Master transaction identifier
        modbusPkt[pktLength++] = (uint8_t)transactionId;
        modbusPkt[pktLength++] = 0; // protocol
        modbusPkt[pktLength++] = 0;
        modbusPkt[pktLength++] = 0; // length
        if (crcRequire)
        {
            modbusPkt[pktLength++] = 8;
        }
        else
        {
            modbusPkt[pktLength++] = 6;
        }
    }
    int registerCount = bodyLength / regByteCount;
    int byteCount = bodyLength;

    if (functionCode == WRITE_MULTIPLE_COIL)
    {
        registerCount = bodyLength;
        byteCount = bodyLength / NUMBER_OF_BITS_IN_BYTE;
        if (bodyLength % NUMBER_OF_BITS_IN_BYTE != 0)
        {
            ++byteCount;
        }
    }

    modbusPkt[pktLength++] = id;
    modbusPkt[pktLength++] = functionCode; // 0x03 READ Holding,
    modbusPkt[pktLength++] = (uint8_t)(registerAddr >> 8);
    modbusPkt[pktLength++] = (uint8_t)registerAddr;
    modbusPkt[pktLength++] = (uint8_t)(registerCount >> 8);
    modbusPkt[pktLength++] = (uint8_t)(registerCount & 0xFFU);

    if (tcpHeader == false || isTcpHeaderByteCountRequired == true)
    {
        modbusPkt[pktLength++] = (uint8_t)byteCount;
    }

    if (bodyLength < 240)
    {
        if (functionCode == WRITE_MULTIPLE_COIL)
        {
            for (uint8_t ix = 0; ix < byteCount; ++ix)
            {
                uint8_t statusByte = 0;
                ushort baseIx = (ushort)(NUMBER_OF_BITS_IN_BYTE * ix);

                for (uint8_t bx = 0; bx < NUMBER_OF_BITS_IN_BYTE; ++bx)
                {
                    if ((baseIx + bx) < bodyLength)
                    {
                        uint8_t bitData = (uint8_t)(body[baseIx + bx] > 0 ? 1 : 0);
                        statusByte = (uint8_t)(statusByte | (bitData << bx));
                    }
                }
                modbusPkt[pktLength++] = statusByte;
            }
        }
        else
        {
            for (uint8_t ix = 0; ix < byteCount; ++ix)
            {
                modbusPkt[pktLength++] = body[ix];
            }
        }
    }

    if (tcpHeader == true)
    {
        ushort tcpBodyLength = (uint8_t)(pktLength - 6);
        if (crcRequire == true)
        {
            tcpBodyLength += 2;
        }

        modbusPkt[4] = (uint8_t)(tcpBodyLength >> 8);
        modbusPkt[5] = (uint8_t)tcpBodyLength;
    }

    if (crcRequire == true)
    {
        ushort crc16 = ModBus_CRC16(modbusPkt, pktLength);
        modbusPkt[pktLength++] = (uint8_t)crc16;
        modbusPkt[pktLength++] = (uint8_t)(crc16 >> 8);
    }

    return pktLength;
}

CommunicationResponse ProcessRegisterOrCoilReading(ModbusDevice *device, uint8_t address, uint8_t regByteCount, uint8_t functionCode, int startAddress, int noOfRegisters, bool crcRequire, bool tcpHeader, uint16_t transactionId)
{
    CommunicationResponse response;

    uint8_t commandCode[MB_MAX_PACKET_SIZE];
    // Serial.println((String)"Start PSRAM: " + ESP.getFreeHeap());
    uint16_t commandLength = ReadModbusPkt(commandCode, address, functionCode, (ushort)startAddress, noOfRegisters, crcRequire, tcpHeader, (ushort)transactionId);

    int requiredBodyLength = 0;
    if (functionCode == READ_COILS_ADDRESS || functionCode == READ_DISCRETE_IP_ADDRESS)
    {
        requiredBodyLength += noOfRegisters / NUMBER_OF_BITS_IN_BYTE;
        if (noOfRegisters % NUMBER_OF_BITS_IN_BYTE != 0)
        {
            ++requiredBodyLength;
        }
    }
    else
    {
        requiredBodyLength += noOfRegisters * regByteCount;
    }

    response = SendRequestWaitResponse(address, commandCode, commandLength, device->timeOut);
    if (response.result && tcpHeader)
    {
        response.result = response.byteCount > MODBUS_TCP_HEADER_LENGTH;
        if (response.result)
        {
            response.byteCount = CommunicationResponse::GetSubarray(
                response.bytes, 0, response.bytes, MODBUS_TCP_HEADER_LENGTH,
                response.byteCount - MODBUS_TCP_HEADER_LENGTH);
        }
    }
    if (response.result == false)
    {
    }

    if (response.result)
    {
        int minimumPktLength = MODBUS_HEADER_LENGTH;
        if (crcRequire)
        {
            minimumPktLength += MODBUS_CRC_LENGTH;
        }
        response.result = requiredBodyLength > 0 &&
                          response.byteCount == (minimumPktLength + MODBUS_BYTE_COUNT_LENGTH + requiredBodyLength) &&
                          response.bytes[MODBUS_FUNCTION_CODE_INDEX] == functionCode &&
                          (crcRequire == false || ModBus_CRC16(response.bytes, response.byteCount) == 0U);
    }
    if (response.result)
    {

        response.byteCount = CommunicationResponse::GetSubarray(
            response.bytes, 0, response.bytes, MODBUS_HEADER_LENGTH + MODBUS_BYTE_COUNT_LENGTH,
            requiredBodyLength);

        if (functionCode == READ_COILS_ADDRESS || functionCode == READ_DISCRETE_IP_ADDRESS)
        {
            uint8_t bytes[MB_MAX_PACKET_SIZE], byteCount = response.byteCount;
            memcpy(bytes, response.bytes, byteCount);
 
            response.clear();
             
            for (int ix = 0; ix < byteCount; ++ix)
            {
                uint8_t statusByte = bytes[ix];

                for (int bx = 0; bx < NUMBER_OF_BITS_IN_BYTE; ++bx)
                {
                    if (response.byteCount < noOfRegisters)
                    {
                       response.bytes[response.byteCount++] = (uint8_t)(statusByte & 0x1);
                    }
                    else
                    {
                        break;
                    }
                    statusByte = (uint8_t)(statusByte >> 1);
                }
            }
        }
        else
        { 
            // print_bytes_in_hex("Data", data.data(), data.size());
        }
    }

    // if (response.result)
    // {
    // }

    // Serial.println((String)"End  PSRAM: " + ESP.getFreeHeap());
    return response;
}

CommunicationResponse ProcessMultipleCoilOrRegisterWriting(uint8_t address, uint8_t regByteCount, uint8_t functionCode, uint16_t startAddress,
                                                           uint8_t noOfRegisters, uint8_t *data, bool crcRequire, bool tcpHeader, int transactionId, bool isTcpHeaderByteCountRequired)
{
    CommunicationResponse response;
 

    uint8_t commandCode[MB_MAX_PACKET_SIZE];
    uint16_t commandLength = WriteMultipleModbusPkt(commandCode, address, regByteCount, functionCode, (ushort)startAddress,
                                                    data, (regByteCount * noOfRegisters), crcRequire, tcpHeader, (ushort)transactionId, isTcpHeaderByteCountRequired);
    int headerBodyLength = MODBUS_HEADER_LENGTH + MODBUS_ADDRESS_LENGTH + MODBUS_REGISTER_COUNT_LENGTH;
    int totalLength = headerBodyLength;
    if (crcRequire)
    {
        totalLength += MODBUS_CRC_LENGTH;
    }

    response = SendRequestWaitResponse(address, commandCode, commandLength, MODBUS_RESPONSE_TIME_OUT);

    if (response.result && tcpHeader)
    {
        response.result = response.byteCount > MODBUS_TCP_HEADER_LENGTH && commandLength > MODBUS_TCP_HEADER_LENGTH;
        if (response.result)
        {
            response.byteCount = CommunicationResponse::GetSubarray(response.bytes, 0, response.bytes, MODBUS_TCP_HEADER_LENGTH,
            response.byteCount - MODBUS_TCP_HEADER_LENGTH);
            commandLength = CommunicationResponse::GetSubarray(commandCode, 0, commandCode, MODBUS_TCP_HEADER_LENGTH, 
            commandLength - MODBUS_TCP_HEADER_LENGTH);
        }
    }

    if (response.result)
    {
        response.result = response.byteCount == totalLength &&
                          response.bytes[MODBUS_FUNCTION_CODE_INDEX] == functionCode &&
                          memcmp(response.bytes, commandCode, headerBodyLength) == 0 &&
                          (crcRequire == false || ModBus_CRC16(response.bytes, response.byteCount) == 0U);
    }
    return response;
}

CommunicationResponse ProcessSingleCoilOrRegisterWriting(uint8_t address, uint8_t regByteCount, uint8_t functionCode, int startAddress, uint32_t value, bool crcRequire, bool tcpHeader, int transactionId)
{
    CommunicationResponse response;
    uint8_t commandCode[MB_MAX_PACKET_SIZE];
    uint16_t commandLength = WriteSingleModbusPkt(commandCode, address, regByteCount, functionCode, (ushort)startAddress, value, crcRequire, tcpHeader, (ushort)transactionId);

    // executionTimer.start();
    response = SendRequestWaitResponse(address, commandCode, commandLength, MODBUS_RESPONSE_TIME_OUT);
    if (response.result && tcpHeader)
    {
        response.result = response.byteCount > MODBUS_TCP_HEADER_LENGTH && commandLength > MODBUS_TCP_HEADER_LENGTH;
        if (response.result)
        {
            response.byteCount = CommunicationResponse::GetSubarray(response.bytes, 0, response.bytes, MODBUS_TCP_HEADER_LENGTH,
            response.byteCount - MODBUS_TCP_HEADER_LENGTH);
            commandLength = CommunicationResponse::GetSubarray(commandCode, 0, commandCode, MODBUS_TCP_HEADER_LENGTH, 
            commandLength - MODBUS_TCP_HEADER_LENGTH);
        }
    }

    if (response.result)
    {
        response.result = response.byteCount == commandLength &&
                          response.bytes[MODBUS_FUNCTION_CODE_INDEX] == functionCode &&
                          (crcRequire == false || ModBus_CRC16(response.bytes, response.byteCount) == 0U);
    }
    return response;
}
