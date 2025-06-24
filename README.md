# FPGA_Runner:Endless Runner Game with FPGA Controller

<p align="left">
  <a href="https://www.intel.com/content/www/us/en/products/programmable/fpga/de10-lite.html">
    <img src="https://img.shields.io/badge/Platform-DE10--Lite%20FPGA-F26222.svg" alt="Platform">
  </a>
  <a href="https://en.wikipedia.org/wiki/Verilog">
    <img src="https://img.shields.io/badge/HDL-Verilog-1DBA5A.svg" alt="Hardware Description Language">
  </a>
  <a href="https://www.altera.com/products/processors/overview.html">
    <img src="https://img.shields.io/badge/Processor-Nios%20II-007ACC.svg" alt="Soft Processor">
  </a>
  <a href="https://www.python.org/">
    <img src="https://img.shields.io/badge/Scripting-Python-3776AB.svg" alt="Python">
  </a>
  <a href="https://unity.com/">
    <img src="https://img.shields.io/badge/Game%20Engine-Unity-000000.svg" alt="Unity">
  </a>
  <a href="https://www.intel.com/content/www/us/en/software/programmable/quartus-prime/overview.html">
    <img src="https://img.shields.io/badge/Tools-Quartus%20Prime%20Lite-76b900.svg" alt="Quartus Prime">
  </a>
</p>

An endless runner game built in Unity, controlled in real-time using an FPGA board with an accelerometer. Move, jump, and slide by tilting the FPGA — no keyboard needed!

### Key Features:
✔️ Tilt to Move – Shift FPGA left/right to change lanes.
✔️ Tilt Up to Jump – Avoid obstacles by tilting up.
✔️ Tilt Down to Slide – Duck under barriers by tilting down.
✔️ Dynamic Speed Increase – The game gets progressively faster!
✔️ Real-Time TCP Communication – FPGA sends movement data via a network connection.

## Implementation and Approach

### FPGA Controller (Accelerometer-Based Input)
- FPGA reads X, Y axis data from an SPI-based accelerometer
- Sends formatted data (e.g. 150, -80) over TCP
- NIOS II soft-core processor handles data processing and transmission

### Unity Game Development
- TCP Server in Unity listens for incoming data
- **Movement logic**:
  - X-Axis: Lane switching
  - Y-Axis: Jump/slide detection
- **Physics-based controller** for smooth gameplay
- Collision logic for obstacles and game-over events

### Networking (TCP Communication)
- FPGA acts as a TCP client, continuoulsy sending sensor data
- Unity receives and maps input to game actions with minmal latency

## How to run the game

### Prerequisites:
✔️ Unity 2021+ installed
✔️ FPGA Board with Accelerometer (e.g DE10-Lite)
✔️ Python/TCP Debugging Tool

### Steps to run the game:
1) Open Quartus Prime Lite Edition 18.1. Blast the FPGA with the .sof file

2) Open the Windows Powershell and key in
```bash
& 'C:\intelFPGA_lite\18.1\nios2eds\Nios II Command Shell.bat'
```

3) Navigate to the compiled `.elf` and upload it to the board.

4) Navigate back to the python file `sendtomac.py`

5) Open the unity project

6) Press Play in Unity

7) On the Windows Powershell type
```bash
py sendtomac.py
```

8) Tap to start the game and tilt to play!

### Demo Video

https://github.com/user-attachments/assets/85d03e5f-3dc3-4fda-8345-2ea04e77485c

Prototype running with real-time FPGA input

## Skills Gained from This Project

### FPGA and Embedded Systems
- Designed custom systems in Platform Designer (Qsys)
- Integrated SPI accelerometer using Verilog
- Programmed NIOS II soft-core with Eclipse SBT
- Built TCP clients for real-time data streaming

### Game Development
- Developed custom TCP listener in Unity
- Built smooth lane-switching, jump, and slide mechanics
- Implemented collision detection and UI elements
- Focused on player experience, physics, and animations

### Networking and Cloud
- Used socket programming for fast communication
- (Planned) Integration with AWS DynamoDB for storing player data
- Designed for future multiplyaer and cloud analytics

## Future Improvements
- High Score System – Save and display top scores.
-  Multiplayer Mode – Compete with other FPGA-controlled players.
- Customizable Skins – Unlock different car/character designs.
- AWS Integration – Store player data and game analytics in DynamoDB.
- Enhanced Graphics & Effects – Add particle effects, better animations, and more obstacles.
- Mobile/Embedded Version – Port to Raspberry Pi or other embedded platforms.


