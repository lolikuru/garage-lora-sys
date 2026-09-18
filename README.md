# garage-lora-sys
Remote system with LoRa E220 433mgz module for checking sensors and controlling loads

## Features
- LoRa communication (E220 433mgz module)
- Sensor monitoring capabilities
- Load control functionality
- WiFi integration
- File system support (LITTLE_FS)
- JSON data handling
- Base64 encoding/decoding
- Power client management

## Hardware Requirements
- Arduino board
- LoRa E220 433mgz module
- Sensors (temperature, voltage, etc.)
- Display unit
- Power supply

## Setup Instructions
1. Install required libraries:
   - LoRa library for SX1278/SX1279
   - ESP8266WiFi (if using WiFi)
   - LittleFS for file system support
2. Connect hardware components as per schematic
3. Upload code to Arduino board
4. Configure network settings in `wifi.ino`
5. Initialize LoRa module in `LoRa.ino`

## Usage
- Monitor sensor data through serial monitor
- Control loads using predefined commands
- Access web interface for configuration (if enabled)
- Use display menu for system navigation

## Contributing
1. Fork the repository
2. Create a new branch for your feature
3. Commit changes with descriptive messages
4. Push to your fork
5. Submit pull request

## License
MIT License - see LICENSE file