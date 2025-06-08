# CGEDC2025-BlueRC
The remote controller source tree of our
CGEDC2025 `BLE Air-Conditioner Remote Controller` project.

# Project Structure
The project is developed using the ESP-IDF framework and follows a standard structure.
Below is an overview of the directory layout:

```
.
├── .vscode/                # VSCode settings
├── components/             # IDF Components used in the project
│   ├── arduino             # Arduino component for ESP-IDF
│   ├── AXP173              # AXP173 PMIC driver
│   ├── lvgl                # LVGL graphics library
│   ├── nimble              # NimBLE C++ driver
│   └── TFT_eSPI            # TFT_eSPI driver for Arduino
├── main/                   # Main application code
├── CMakeLists.txt          # CMake build configuration
├── sdkconfig.defaults      # Default configuration for the project
├── README.md               # Project brief documentation
└── LICENSE                 # Project license
```