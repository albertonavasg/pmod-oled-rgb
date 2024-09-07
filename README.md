PENDING TO UPDATE

# pmod-oled-rgb

Project for Design of Embedded Systems, usable in PYNQ-Z2 (and also in PYNQ-Z1)

***

## Contents

1. [Project Summary](#1-project-overview)
2. [Develop process](#2-develop-process)
3. [Replicate the project](#3-replicate-the-project)

***

## 1. Project Overview

The aim of this project is to design a VHDL block in the PL, package it in an IP, build a hardware platform and control the driver (in the PL) from the PS.

This driver can be controlled in from a baremetal application or from an Operating System.

Here is a little demo video of it working, executing some demo functions, such as the standard provided by the screen  
(drawing a full screen of pixels, drawing lines, squares, copy-pasting sections of the screnn) and some custom-built  
(showing an image, drawing characters and strings).

<div align="center">
    <video src="media/PMOD OLEDrgb Demo.mp4" width="80%" height="auto"> 
</div>

<div align="center">
    <video width="80%" controls>
      <source src="media/PMOD OLEDrgb Demo.mp4" type="video/mp4">
      Your browser does not support the video tag.
    </video>
</div>

***

## 2. Develop process



***

## 3. Replicate the project


### Folder structure

- Docs: documentation about PMod OLED RGB 

- PYNQ-Z2: board files (.xdc and .tcl)

- release: bitfiles ready to deploy in the PYNQ-Z2 board

- src: VHDL design files, testbenches and constraints

- screen (ignored): Vivado 2020.1 project to create the screen_controller and test it

- axi_screen (ignored) : Vivado 2020.1 project to create the AXI 4 Lite Driver