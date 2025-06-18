# Protocol of the project

This project utilizes a custom protocol for communication between the
`Remote Controller` and the `Air Conditioner`.  
The protocol is based on BLE (Bluetooth Low Energy), where the
`Remote Controller` acts as a client and the `Air Conditioners` act as servers.

# Protocol Overview
The protocol consists of a series of commands and responses that are exchanged
between the `Remote Controller` and the `Air Conditioner`.

## Services
- `0000fff8-0000-1000-8000-00805f9b34fb`: Identification Service
    - This service is used to identify the device while discovering it.
- `13b528f9-f225-4c8d-a3db-2c9ab927a22e` : Control Service
    - This service is used to control and monitor the air conditioner.

## Characteristics
- There are no characteristics in the Identification Service,
  as it is only used for device discovery.
- The Control Service contains the following characteristics:
    - `0000fff0-0000-1000-8000-00805f9b34fb`: Mode Characteristic
        - This characteristic is used to set the operating mode of the air conditioner
        - the modes include:
            - `0`: Off
            - `1`: Heat
            - `2`: Cool
            - `3`: Dry
            - `4`: Fan
    - `0000fff1-0000-1000-8000-00805f9b34fb`: Temperature Characteristic
        - This characteristic is used to set the target temperature of the air conditioner.
        - The temperature is set in degrees Celsius, ranging from `16` to `30`.
    - `0000fff2-0000-1000-8000-00805f9b34fb`: Fan Speed Characteristic
        - This characteristic is used to set the fan speed of the air conditioner.
        - The fan speed can be set to:
            - `0`: Auto
            - `1`: Low
            - `2`: Medium
            - `3`: High
    - `0000fff3-0000-1000-8000-00805f9b34fb`: Fan direction Characteristic
        - This characteristic is used to set the fan direction of the air conditioner.
        - The fan direction can be set to:
            - `0`: Auto
            - `1`: Up
            - `2`: Middle
            - `3`: Down
