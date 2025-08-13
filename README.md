## CODE-PKM-2024

C++ source code for the PKM 2024 project, organized into modular components (pH sensor, TDS sensor, relay control) and an integrated setup.

### Repository structure
- `all`: Integrated build combining multiple modules (e.g., pH, TDS, relay)
- `ph`: pH sensor module
- `tds`: TDS sensor module
- `relay`: Relay control module

### Prerequisites
- Arduino IDE 2.x or PlatformIO (VS Code)
- Appropriate USB drivers for your board
- Select the correct Board and Port in your IDE (e.g., Arduino Uno, ESP32, etc.)

### Getting started
1. Clone the repository:
   - `git clone https://github.com/lek34/CODE-PKM-2024.git`
2. Open your IDE:
   - Arduino IDE: Open the folder for the module you want (e.g., `ph`, `tds`, `relay`, or `all`) and open the main sketch/source file inside it.
   - PlatformIO: Open the repository folder as a project; if no `platformio.ini` exists, create one for your board.
3. Configure board/port:
   - Arduino IDE: Tools → Board, Tools → Port
   - PlatformIO: Set `platform`, `board`, `framework` in `platformio.ini`
4. Build and upload:
   - Arduino IDE: Verify → Upload
   - PlatformIO: `Build` → `Upload`

### Configuration
- Check the top of each module’s main file for parameters (pins, calibration constants, thresholds).
- Adjust pin mappings to match your wiring.
- For sensors:
  - pH: Calibrate using known buffer solutions.
  - TDS: Ensure proper voltage divider and probe calibration.

### Usage
- Open Serial Monitor (9600/115200 baud, depending on code) to view readings and debug logs.
- Test modules individually (`ph`, `tds`, `relay`) before using the integrated `all` folder.

### Troubleshooting
- Build errors: Confirm the correct board is selected and required libraries are installed.
- No serial output: Match the sketch’s baud rate in Serial Monitor.
- Sensor noise: Use proper grounding, shielding, and stable power; consider averaging/filtering in code.

### Contributing
- Fork the repo and open a pull request with clear commit messages and testing notes.
- Keep modules self-contained; avoid breaking changes to shared interfaces.

### License
No license file is present. By default, all rights are reserved. If you intend others to use or modify this code, consider adding a license (e.g., MIT).

### Reference
- Repository: [CODE-PKM-2024 on GitHub](https://github.com/lek34/CODE-PKM-2024.git)
