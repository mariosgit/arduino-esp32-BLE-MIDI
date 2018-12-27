# arduino-esp32 BLE-MIDI

MIDI over BLE example for the ESP32 Arduino core.

Tested with Linux client (Bluez >=5.44) and Windows 10 Creators Update 1.
Tested with IOS
Extended the Example to become a SerialMidi <> BLEMidi bridge.

Timestamps are not implemented.
No SysEx support implemented.

## Prerequisites:
* https://github.com/espressif/arduino-esp32
* https://github.com/FortySevenEffects/arduino_midi_library

## Some Docs about MIDI BLE:
* https://community.cypress.com/community/wiced-studio-blueooth/wiced-studio-bluetooth-forums/blog/2018/01/07/implementing-a-ble-midi-controller
* http://www.hangar42.nl/wp-content/uploads/2017/10/BLE-MIDI-spec.pdf
