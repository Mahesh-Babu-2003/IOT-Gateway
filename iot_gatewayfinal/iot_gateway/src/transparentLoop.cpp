#include "application.h"

#include <AsyncTCP.h>

AsyncServer *tcpServer;
AsyncClient *clients[MAX_CLIENTS] = {nullptr};

void onClientConnect(void *arg, AsyncClient *client);

void terminateTransparentServer()
{
    tcpServer->end();
    delete tcpServer;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i] = nullptr;
    }
}

// Add a client to the list
uint8_t addClient(AsyncClient *client)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == nullptr)
        { // Find an empty slot
            clients[i] = client;
            Serial.printf("Client added at index %d\n", i);
            return true;
        }
    }
    Serial.println("Maximum clients reached! Closing new connection...");
    client->close(); // Close the connection if the limit is reached
    delete client;
    return false;
}

// Remove a client from the list
void removeClient(AsyncClient *client)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == client)
        {
            clients[i]->free();
            clients[i] = nullptr; // Remove the client from the list
            Serial.printf("Client removed from index %d\n", i);
            return;
        }
    }
}

// Relay Serial data to all connected clients
void relaySerialToClients(uint8_t data)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != nullptr && clients[i]->connected())
        {
            clients[i]->write((const char *)&data, 1);
            // clients[i]->write((const char *)&data, 1, ASYNC_WRITE_FLAG_COPY); // Send data to the client
            // clients[i]->write((const char *)&data, 1, ASYNC_WRITE_FLAG_MORE);
            // clients[i]->add((const char *)&data, 1, ASYNC_WRITE_FLAG_COPY);
            // clients[i]->send();
        }
    }
}

// Callback when a new client connects
void onClientConnect(void *arg1, AsyncClient *client)
{
    Serial.println("New client connected!");

    if (addClient(client))
    {
        client->setNoDelay(0);
        // client->setRxTimeout(0);
        // Handle data received from the client
        client->onData([](void *arg, AsyncClient *client, void *data, size_t len)
                       {
                           print_bytes_in_hex(((String)"Data received: "+ WiFi.RSSI()+ ": ").c_str(), (uint8_t *)data, len);
                           //if (xSemaphoreTake(deviceMutex, portMAX_DELAY) == pdTRUE)
                           {
                               digitalWrite(RS485_EN_OP, HIGH);
                               // delay(1);
                            //    delayMicroseconds(10);
                               Serial1.write((uint8_t *)data, len);
                               Serial1.flush();
                            //    Serial.flush();
                               // delay(1);
                               digitalWrite(RS485_EN_OP, LOW);
                               //xSemaphoreGive(deviceMutex); // Release the mute
                           } },
                       nullptr);

        // Handle client disconnection
        client->onDisconnect([](void *arg, AsyncClient *client)
                             {
            Serial.println("Client disconnected.");
            removeClient(client);
            delete client; }, nullptr);
    }
}

void initializeTransaparentServer()
{

    Serial.println((String) "initialize Transaparent Server..." + liveObj.serverPort);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i] = nullptr;
    }
    tcpServer = new AsyncServer(liveObj.serverPort == 80 ? 8080 : liveObj.serverPort);

    // Start the TCP server
    tcpServer->onClient([](void *arg, AsyncClient *client)
                        {
                            Serial.println("Client attempting to connect...");
                            Serial.printf("Client pointer: %p\n", client);

                            onClientConnect(nullptr, client);
                            // client->onConnect(onClientConnect, nullptr); // Attach the onConnect callback

                            // client->onConnect([](void* arg, AsyncClient* client) {
                            //     Serial.printf("Client connected successfully: %p\n", client);
                            // }, nullptr);
                        },
                        nullptr);
    tcpServer->begin();
}

void processTransparentLoop()
{
    while (Serial1.available())
    {
        uint8_t data = Serial1.read();
        relaySerialToClients(data);
    }
}