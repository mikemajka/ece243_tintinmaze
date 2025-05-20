# 243 Maze Project – RISC-V on DE1-SoC FPGA

## Overview
This project implements a **3D projected maze game** on the **DE1-SoC FPGA board**, using a custom **RISC-V processor**. The player navigates the maze using a **PS/2 keyboard**, and game events are enhanced with **audio feedback**.

## Features
- ✅ 3D projected maze rendering
- ✅ RISC-V architecture implemented on FPGA
- ✅ Real-time user interaction via PS/2 keyboard
- ✅ Audio output for actions and events
- ✅ Modular and optimized for performance on DE1-SoC

## Hardware Used
- **DE1-SoC FPGA Board**
- **VGA Display** (for maze rendering)
- **PS/2 Keyboard** (for input)
- **Audio Output** (3.5mm jack or onboard audio codec)

## Controls
- Arrow keys: Move forward, turn left/right
- Escape: Exit game/reset
- Optional: Add key bindings for special interactions or debug mode

## Architecture
- **Custom RISC-V CPU**: Lightweight, pipelined
- **VGA Controller**: For 3D maze projection
- **PS/2 Controller**: Decodes keyboard inputs
- **Audio Module**: Outputs sound on events
- **Game Logic**: Handles position, collisions, and rendering state

## Build & Run
1. Open project in **Intel Quartus Prime**
2. Compile and program the FPGA
3. Connect VGA display, PS/2 keyboard, and optional speakers
4. Power on and start navigating the maze!

## Future Improvements
- Add sound variety and effects
- Improve rendering with texture mapping or shading
- Add multiple maze levels or procedural generation

## Authors
- [Enpei Gu](https://github.com/engu1)
- [Minghan Wei](https://github.com/mikemajka)
  
Course project for ECE243 – Computer Organization  
University of Toronto  
