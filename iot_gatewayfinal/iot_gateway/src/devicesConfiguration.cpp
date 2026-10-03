#include "application.h"

uint8_t clearAllDevices()
{
    if (liveObj.deviceCount != 0)
    {
        if (xSemaphoreTake(deviceMutex, portMAX_DELAY) == pdTRUE)
        {
            memset(liveObj.modbusDevices, 0, sizeof(liveObj.modbusDevices));
            saveModbusDevicesConfiguration();
            liveObj.deviceCount = 0;
            xSemaphoreGive(deviceMutex); // Release the mute
        }
    }
    return true;
}

uint8_t addModbusDevice(ModbusDevice device)
{
    uint8_t isAdded = false;
    if (xSemaphoreTake(deviceMutex, portMAX_DELAY) == pdTRUE)
    {
        device.isActive = true;
        device.validityBytes = MB_VALIDITY_VALUE;

        for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
        {
            if (liveObj.modbusDevices[ix].id == device.id)
            {
                strcpy(liveObj.modbusDevices[ix].name, device.name);
                liveObj.modbusDevices[ix].baud_rate = device.baud_rate;
                liveObj.modbusDevices[ix].data_bits = device.data_bits;
                liveObj.modbusDevices[ix].parity = device.parity;
                liveObj.modbusDevices[ix].stop_bits = device.stop_bits;
                liveObj.modbusDevices[ix].regByteCount = device.regByteCount;

                liveObj.modbusDevices[ix].isActive = true;
                liveObj.modbusDevices[ix].validityBytes = MB_VALIDITY_VALUE;

                liveObj.slaveConnectionCounter[ix] = 0;
                Serial.println((String) "Device Added RS485-> " + device.name);
                isAdded = true;
                break;
            }
        }

        if (liveObj.deviceCount < NO_OF_MODBUS_METERS)
        {
            if (isAdded == false)
            {
                liveObj.slaveConnectionCounter[liveObj.deviceCount] = 0;
                Serial.println((String) "Device Added RS485-> " + device.name);
                liveObj.modbusDevices[liveObj.deviceCount++] = device;
                isAdded = true;
            }
            isAdded = saveModbusDevicesConfiguration();
        }
        xSemaphoreGive(deviceMutex); // Release the mute
    }

    return isAdded;
}

uint8_t deleteModbusDevice(short deviceId)
{
    uint8_t isDeleted = false;
    Serial.println("deleteModbusDevice" + (String)deviceId);

    if (xSemaphoreTake(deviceMutex, portMAX_DELAY) == pdTRUE)
    {
        for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
        {
            if (liveObj.modbusDevices[ix].id == deviceId)
            {
                liveObj.modbusDevices[ix].isActive = false;
                ix++;
                for (; ix < liveObj.deviceCount; ++ix)
                {
                    liveObj.modbusDevices[ix - 1] = liveObj.modbusDevices[ix];
                }
                --liveObj.deviceCount;
                liveObj.modbusDevices[liveObj.deviceCount].isActive = false;
                isDeleted = saveModbusDevicesConfiguration();
                break;
            }
        }
        xSemaphoreGive(deviceMutex); // Release the mute
    }

    return isDeleted;
}

uint8_t addModbusDeviceParameter(int deviceId, ApplicationRegister mbRegister)
{
    uint8_t isAdded = false;
    if (xSemaphoreTake(deviceMutex, portMAX_DELAY) == pdTRUE)
    {
        for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
        {
            if (liveObj.modbusDevices[ix].id == deviceId)
            {

                for (uint8_t px = 0; px < liveObj.modbusDevices[ix].noOfRegisters; ++px)
                {
                    if (liveObj.modbusDevices[ix].mb_register[px].id == mbRegister.id)
                    {
                        liveObj.modbusDevices[ix].mb_register[px] = mbRegister;
                        isAdded = true;
                        break;
                    }
                }

                if (liveObj.modbusDevices[ix].noOfRegisters < NO_OF_MODBUS_METERS)
                {
                    if (isAdded == false)
                    {
                        liveObj.modbusDevices[ix].mb_register[liveObj.modbusDevices[ix].noOfRegisters] = mbRegister;
                        ++liveObj.modbusDevices[ix].noOfRegisters;
                        isAdded = true;
                    }
                    isAdded = saveModbusDevicesConfiguration();
                }
                else
                {
                }
            }
        }
        xSemaphoreGive(deviceMutex); // Release the mute
    }

    return isAdded;
}

uint8_t deleteModbusDeviceParameter(short deviceId, short paramId)
{
    uint8_t isDeleted = false;
    if (xSemaphoreTake(deviceMutex, portMAX_DELAY) == pdTRUE)
    {
        for (uint8_t ix = 0; ix < liveObj.deviceCount; ++ix)
        {
            if (liveObj.modbusDevices[ix].id == deviceId)
            {
                for (uint8_t px = 0; px < liveObj.modbusDevices[ix].noOfRegisters; ++px)
                {
                    if (liveObj.modbusDevices[ix].mb_register[px].id == paramId)
                    {
                        px++;
                        for (; px < liveObj.modbusDevices[ix].noOfRegisters; ++px)
                        {
                            liveObj.modbusDevices[ix].mb_register[px - 1] = liveObj.modbusDevices[ix].mb_register[px];
                        }
                        --liveObj.modbusDevices[ix].noOfRegisters;
                        isDeleted = saveModbusDevicesConfiguration();
                        break;
                    }
                }
                break;
            }
        }
        xSemaphoreGive(deviceMutex); // Release the mute
    }
    return isDeleted;
}


uint8_t addSnmpDevice(SNMP_Device device)
{
    Serial.println("addSnmpDevice");
    uint8_t isAdded = false;
    if (xSemaphoreTake(snmpMutex, portMAX_DELAY) == pdTRUE)
    {
        device.isActive = true;
        device.validityBytes = MB_VALIDITY_VALUE;
        uint8_t ix = 0, emptyDevicePlaceIx = 0xFF;

        for (ix = 0; ix < NO_OF_SNMP_DEVICES; ++ix)
        {
            if (liveObj.snmpDevices[ix].id == device.id)
            {
                liveObj.snmpDevices[ix] = device;
                isAdded = true;
                break;
            }else if(liveObj.snmpDevices[ix].isActive == false && emptyDevicePlaceIx == 0xFF){
                emptyDevicePlaceIx = ix;
            }
        }

        if (ix < NO_OF_SNMP_DEVICES || emptyDevicePlaceIx != 0xFF)
        {
            if (isAdded == false)
            { 
                Serial.println((String) "Device Added SNMP-> " + device.name);
                liveObj.snmpDevices[emptyDevicePlaceIx] = device;
                isAdded = true;
            }
            isAdded = saveSnmpDevicesConfiguration();
        }
        xSemaphoreGive(snmpMutex); // Release the mute
    }

    return isAdded;
}

uint8_t deleteSnmpDevice(short deviceId)
{
    uint8_t isDeleted = false;
    Serial.println("deleteModbusDevice" + (String)deviceId);

    if (xSemaphoreTake(snmpMutex, portMAX_DELAY) == pdTRUE)
    {
        for (uint8_t ix = 0; ix < NO_OF_SNMP_DEVICES; ++ix)
        {
            if (liveObj.snmpDevices[ix].id == deviceId)
            {
                liveObj.snmpDevices[ix].isActive = false; 
                isDeleted = saveSnmpDevicesConfiguration();
                break;
            }
        }
        xSemaphoreGive(snmpMutex); // Release the mute
    }

    return isDeleted;
}