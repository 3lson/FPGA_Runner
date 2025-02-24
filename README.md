# FPGA_Runner:Endless Runner Game with FPGA Controller

- [Project Overview](#Project-Overview)
    - [Key Features](#Key-Features)
- [Implementation & Approach](#Implementation-and-Approach)
    - [FPGA Controller (Accelerometer-Based Input)](#fpga-controller-accelerometer-based-input)
    - [Unity Game Development](#unity-game-development)
    - [Networking (TCP)](#networking-tcp-communication)
- [How to run the game](#how-to-run-the-game)
    -[Steps to run the Game](#steps-to-run-the-game)
- [Skills Gained from this Project](#skills-gained-from-this-project)
    - [FGPA & Embedded Systems](#FPGA-and-embedded-systems)
    - [Game Development (Unity & C#)](#game-development)
    - [Networking & Cloud](#networking-and-cloud)
- [Improvements](#improvements)

## Project Overview
**FPGA_Runner** is a Unity-based Endless Runner game controlled using an FPGA-based accelerometer! Instead of traditional keyboard or touch controls, players move by tilting an FPGA board, which sends real-time data to Unity via TCP communication. 

### Key Features:
✔️ Tilt to Move – Shift FPGA left/right to change lanes.
✔️ Tilt Up to Jump – Avoid obstacles by tilting up.
✔️ Tilt Down to Slide – Duck under barriers by tilting down.
✔️ Dynamic Speed Increase – The game gets progressively faster!
✔️ Real-Time TCP Communication – FPGA sends movement data via a network connection.

## Implementation and Approach

### FPGA Controller (Accelerometer-Based Input)
- **Hardware**: FPGA board with an accelerometer (SPI interface).
- **Processing**: Reads accelerometer values (X, Y axes) and transmits over TCP.
- **Data Format**: "X_Value, Y_Value" (e.g., "150, -80").

### Unity Game Development
- **TCP Listener** in Unity continuously receives FPGA data.
- **Movement logic**:
  - X-Axis: Moves the player left/right between lanes.
  - Y-Axis: Triggers jumping or sliding.
- **Physics-based CharacterController** ensures smooth motion.
- **Collision detection** manages game-over states when hitting obstacles.

### Networking (TCP Communication)
- Unity runs a **TCP Server** on port 5005 to receive FPGA input.
- The FPGA board sends real-time data, ensuring low-latency motion control.

## How to run the game

### Prerequisites:
✔️ Unity 2021+ installed
✔️ FPGA Board with Accelerometer
✔️ Python/TCP Debugging Tool (Optional)

### Steps to run the game:
1) Open Quartus Prime Lite Edition 18.1. Blast the FPGA with the .sof file

2) Open the Windows Powershell and key in
```bash
& 'C:\intelFPGA_lite\18.1\nios2eds\Nios II Command Shell.bat'
```

3) Navigate to where the .elf file in generated

4) Navigate back to the python file `sendtomac.py`

5) Open the unity project

6) Hit Run

7) On the Windows Powershell type
```bash
py sendtomac.py
```

8) Tap to start and Enjoy the Game!

https://github.com/user-attachments/assets/85d03e5f-3dc3-4fda-8345-2ea04e77485c
Video 1: Prototype Demo 1 for the game running with FPGA controller

## Skills Gained from This Project

### FPGA and Embedded Systems
- **Intel Quartus Prime** – Designed system using Platform Designer (Qsys) with a NIOS II soft processor.
- **Verilog HDL** – Integrated the accelerometer SPI interface at the top level.
- **Eclipse IDE (Nios II Software Build Tools)** – Programmed the soft processor in C to retrieve accelerometer data.
- **Socket Programming (Python & C)** – Implemented TCP communication between FPGA and Unity.

### Game Development
- **TCP Server** – Developed a real-time TCP listener in C# to receive FPGA data.
- **Character Controller** – Implemented lane movement, jumping, and sliding.
- **Physics-Based Gameplay** – Smooth player movement and collision detection.
- **Game UI & Backend** – Built an interactive Unity interface.

### Networking and Cloud
- **Low-Latency Networking**: Real-time data streaming from FPGA to Unity
- **Cloud-Based Data Storage**: AWS DynamoDB for player stats & high scores (Potential expansion)

## Improvements
- High Score System – Save and display top scores.
-  Multiplayer Mode – Compete with other FPGA-controlled players.
- Customizable Skins – Unlock different car/character designs.
- AWS Integration – Store player data and game analytics in DynamoDB.
- Enhanced Graphics & Effects – Add particle effects, better animations, and more obstacles.
- Mobile/Embedded Version – Port to Raspberry Pi or other embedded platforms.


