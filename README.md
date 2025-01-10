# Foot Sensor
This is a foot sensor project that uses a force sensitive resistor to detect the pressure of the foot.  
The whole circuit is integrated in a shoe sole. The data is sent to a phone via Bluetooth.  
The data is then processed and displayed on the phone. The data is also sent to a server for further analysis.

## Hardware
- Force Sensitive Resistor
- MCU and Bluetooth module(ESP32H2)
- Battery
- Battery charger(TP4054)

## Software
This project uses the ESP-IDF and Arduino framework.  

### Build
0. Install the ESP-IDF environment.

1. Clone the repository:
```bash
git clone https://github.com/HQU-gxy/footSensor.git
```

2. Change the directory to the project and export ESP-IDF path:
```bash
cd footSensor
. path/to/idf/export.sh 
```

3. Build the project:
```bash
idf.py build
```

4. Flash the executable to the chip:
```bash
idf.py -p /dev/ttyUSB0 flash # Change the port to the one you are using
```


