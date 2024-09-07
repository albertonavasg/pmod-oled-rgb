# pmod-oled-rgb

Project for Design of Embedded Systems, a subject from my Master's Degree in Industrial Electronics.  
It is usable in `PYNQ-Z2` (and also in `PYNQ-Z1` takiig care of physical pinouts).  

***

## Contents

1. [Project Overview](#1-project-overview)
2. [Develop Process](#2-develop-process)
3. [Replicate the Project](#3-replicate-the-project)
4. [Future Plans](#4-future-plans)
5. [Folder Structure](#5-folder-structure)

***

## 1. Project Overview

The aim of this project is to design a VHDL block in the PL, package it in an IP, build a hardware platform and control the driver (in the PL) from the PS.

This driver can be controlled in from a baremetal application or from an Operating System.

Here is a little demo video of it working, executing some demo functions, such as the standard provided by the screen  
(drawing a full screen of pixels, drawing lines, squares, copy-pasting sections of the screnn) and some custom-built  
(showing an image, drawing characters and strings).

[PMOD OLED RGB Demo][youtube-demo]

***

## 2. Develop Process

1. VHDL block: spi_master block, embedded in screen_controller block
2. Baremetal: package the screen_controller VHDL block in an AXI4-lite IP and controll from a C program in the PS
3. OS: create an embedded linux and adapt the C program to run in it

***

## 3. Replicate the Project

In the directory `release` we can find three subdirectories:

- `vhdl`
- `baremetal`
- `os`

In each of them, there are the neccesary files and a file called `version.txt` indicating the version and some instructions tor replicate the project.

## 4. Future Plans

My plans for the future involve:

- Refactor the VHDL code of the SPI master for a more "professional" style and a configurable module
- Extend the C functionalities of the driver
- Use Petalinux for the Operating System, simplifying the current deployment process of the OS and general replication of the project, specially the OS version, whichc is the most interesting one.
- Add a second screen to have a richer system and some variety of functionalities.

## 5. Folder Structure

- Docs: documentation about PMod OLED RGB 

- PYNQ-Z2: board files (.xdc and .tcl)

- release: bitfiles ready to deploy in the PYNQ-Z2 board

- src: source code such as VHDL, IP library, C...

- screen (ignored): Vivado 2020.1 project to create the screen_controller and test it

- axi_screen (ignored) : Vivado 2020.1 project to create the AXI 4 Lite Driver


[youtube-demo]: https://youtu.be/TNlVlC1Tnaw
