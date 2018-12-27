/*
    BLE_MIDI Example by neilbags 
    https://github.com/neilbags/arduino-esp32-BLE-MIDI
    
    Based on BLE_notify example by Evandro Copercini.

    Creates a BLE MIDI service and characteristic.
    Once a client subscibes, send a MIDI message every 2 seconds


    Version History:
    - BLE_MIDI Example by neilbags https://github.com/neilbags/arduino-esp32-BLE-MIDI
    - Dec 2018: "mariosgit" merged in "Update for iOS" by "utaani" (one of the forks)
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <MIDI.h>
USING_NAMESPACE_MIDI 

#define MIDI_SERVICE_UUID "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

HardwareSerial SerialMIDI(2);
MidiInterface<HardwareSerial> mididev(SerialMIDI);

uint8_t midiPacket[] = {
    0x80, // header
    0x80, // timestamp, not implemented
    0x00, // status
    0x3c, // 0x3c == 60 == middle c
    0x00  // velocity
};

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};


/**
 * \class MyCharacteristicCallbacks

    Received Values can look like this... (packet format: second one without the higher TS)
    The Format can be found in "Apple Bluetooth Low Energy MIDI Specification".

    *********
    Received Value: ,0xBD,0xCF,0x90,0x34,0x7B
    *********
    Received Value: ,0xBF,0xCA,0x90,0x3C,0x78
    *********
    Received Value: ,0x86,0x9C,0x90,0x34,0x0,0x9F,0x90,0x3C,0x0
*/

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            byte timestamp = rxValue[0];
            int i = 1;
            while(i < rxValue.length())
            {
                if(rxValue.length()-i >= 4)
                {
                    byte ts_0 = rxValue[i+0];
                    byte val1 = rxValue[i+1]; // Status 0x90 0x80 + channel
                    byte val2 = rxValue[i+2]; // Note
                    byte val3 = rxValue[i+3]; // Velocity
                    Serial.print(" BLE->midi");
                    Serial.print(" 0x"); Serial.print(val1, HEX);
                    Serial.print(" 0x"); Serial.print(val2, HEX);
                    Serial.print(" 0x"); Serial.print(val3, HEX);

                    SerialMIDI.write((const uint8_t*)&rxValue[i+1], 3); // copy to MIDI out
                }
                else
                {
                    Serial.print(" BLE->midi - unexpected length");
                    Serial.print(i);
                    Serial.print(" / ");
                    Serial.print(rxValue.length());
                }
                i += 4;
            }
            Serial.println();
        }
    }
};

void setup()
{
    Serial.begin(115200);
    SerialMIDI.begin(31250); //MIDI has 31250, seems to work on esp32

    BLEDevice::init("ESP32 BLE-MIDI");

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        BLEUUID(CHARACTERISTIC_UUID),
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE_NR);

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    // pServer->getAdvertising()->start();

    // update from https://github.com/utaani/arduino-esp32-BLE-MIDI/blob/master/BLE_MIDI.ino
    // Start advertising (for iOS)
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    oAdvertisementData.setFlags(0x04);
    oAdvertisementData.setCompleteServices(BLEUUID(MIDI_SERVICE_UUID));
    oAdvertisementData.setName("ESP32 BLE-MIDI");
    BLEAdvertising *pAdvertising;
    pAdvertising = pServer->getAdvertising();
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->start();
}

void loop()
{
    if (deviceConnected)
    {
        // note on
        // midiPacket[2] = 0x90;                     // note down, channel 0
        // midiPacket[4] = 127;                      // velocity
        // pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
        // pCharacteristic->notify();

        // play note for 500ms
        // delay(500);

        // note off
        // midiPacket[2] = 0x80;                     // note up, channel 0
        // midiPacket[4] = 0;                        // velocity
        // pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
        // pCharacteristic->notify();

        // delay(500);

        if(mididev.read())
        {
            midiPacket[2] = mididev.getType() | mididev.getChannel();
            midiPacket[3] = mididev.getData1();
            midiPacket[4] = mididev.getData2();
            pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
            pCharacteristic->notify();
            Serial.print("midi->BLE ");
            Serial.print(midiPacket[2], HEX); Serial.print(" ");
            Serial.print(midiPacket[3], HEX); Serial.print(" ");
            Serial.print(midiPacket[4], HEX); Serial.println();
        }
    }
    else
    {
        Serial.println("not connected");
        delay(1000);
    }
}
