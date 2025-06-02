# LCOM Racing Game Project

A racing game developed in C for the LCOM (Computer Laboratory) course at FEUP, built on the MINIX operating system. This project demonstrates low-level programming concepts including device drivers, interrupt handling, and graphics programming.

## 🕹️ Demo
  [`video demo`](https://uporto.cloud.panopto.eu/Panopto/Pages/Viewer.aspx?id=06810408-5b1c-434a-b0f9-b2ef01760a52)

## 🎮 About

This is a complete racing game implementation that runs on MINIX, featuring:
- Custom device drivers for keyboard, mouse, and timer interrupts
- VGA graphics programming with multiple video modes
- Real-time game mechanics with collision detection
- Multiple racing tracks with obstacles and power-ups
- Custom font rendering system

## 🚀 Features

- **Multiple Racing Tracks**: 3 different tracks with unique layouts and backgrounds
- **Real-time Controls**: Responsive keyboard input using custom keyboard driver
- **Graphics Engine**: Custom VGA graphics implementation supporting different resolutions
- **Game Mechanics**: 
  - Collision detection with track boundaries
  - Obstacle avoidance gameplay
  - Power-up collection system
  - Pause functionality
- **Low-level Programming**: Direct hardware interaction through custom device drivers

## 🛠 Technical Implementation

### Device Drivers Implemented
- **Timer Driver** ([`shared/lab2/timer.c`](shared/lab2/timer.c)): Handles system timing and interrupts
- **Keyboard Driver** ([`shared/lab3/keyboard.c`](shared/lab3/keyboard.c)): Manages keyboard input and scancodes
- **Mouse Driver**: Handles mouse input events
- **Video Driver** ([`shared/lab5/video_card.h`](shared/lab5/video_card.h)): VGA graphics programming

### Key Components
- **Main Game Loop** ([`shared/proj/src/macros.h`](shared/proj/src/macros.h)): Event-driven architecture handling timer, keyboard, and mouse events
- **Track System**: Binary track data with surface collision detection
- **Graphics Rendering**: Custom drawing functions for sprites, backgrounds, and UI elements

### System Architecture
```
┌─────────────────┐
│   Game Logic    │
├─────────────────┤
│ Event Handling  │
├─────────────────┤
│ Device Drivers  │
├─────────────────┤
│  MINIX Kernel   │
└─────────────────┘
```

## 🏗 Building and Running

### Prerequisites
- MINIX operating system
- GCC compiler
- VirtualBox (for running MINIX VM)

### Build Instructions
```bash
# Navigate to project source directory
cd shared/proj/src

# Compile the project
make clean && make

# Run the game
lcom_run proj
```

### Lab Exercises
Each lab folder contains progressive implementations:
- **Lab 2**: Timer functionality and interrupts
- **Lab 3**: Keyboard driver implementation
- **Lab 4**: Mouse driver (if applicable)
- **Lab 5**: VGA graphics programming

## 🎯 Controls

- **Arrow Keys**: Navigate menus and control vehicle
- **Spacebar**: Select/Action
- **P**: Pause game
- **ESC**: Exit/Back

## 📁 Project Structure

```
shared/
├── lab2/           # Timer implementation
├── lab3/           # Keyboard driver
├── lab5/           # Graphics driver
└── proj/           # Final racing game
    └── src/
        ├── tracks/ # Track data files
        └── macros.h # Game constants and definitions
```

## 🏆 Learning Outcomes

Through this project, I gained hands-on experience with:
- **Low-level Programming**: Direct hardware manipulation and register programming
- **Operating Systems**: Interrupt handling, device drivers, and system calls
- **Graphics Programming**: VGA mode setting, pixel manipulation, and sprite rendering
- **Real-time Systems**: Managing timing-critical game loops and input handling
- **C Programming**: Advanced C concepts including bit manipulation and memory management

## 👥 Original Team

This project was originally developed as part of a team effort:
- Pedro Lunet
- Bernardo Brandão
- Dinis Noronha  
- Mariana Pereira

## 📜 License

This project was developed for educational purposes as part of the LCOM course at FEUP (Faculty of Engineering, University of Porto).

---

*Note: This project requires MINIX OS to run. The game demonstrates fundamental computer systems concepts through practical implementation of device drivers and real-time graphics programming.*
