# pmod-oled-rgb

Project for Design of Embedded Systems, a subject from my Master's Degree in Industrial Electronics.  
It is usable in `PYNQ-Z2` (and also in `PYNQ-Z1` taking care of physical pinouts).  

<div align="center">
<img src="images/pmod-oled-rgb.png" title="Pmod OLED RGB" width="100%" height="auto"> 
</div>

***

## Contents

- [Project Overview](#project-overview)

***

## Project Overview

The aim of this project was to design a VHDL block in the PL, package it in an IP, build a hardware platform and control the driver (in the PL) from the PS.

This driver can be controlled from a baremetal application or from an application running in an Operating System.

In this case, I control this little OLED screen via SPI commands.

Here is a little demo video of it working, executing some demo functions, such as the standard provided by the screen
(drawing a full screen of pixels, drawing lines, squares, copy-pasting sections of the screnn) and some custom-built
(showing an image, drawing characters and strings).

[PMOD OLED RGB Demo][youtube-demo]

This was the first version of the project.  

To see the code, go to the folder `old_version`  

Currently I am working on the second version, with some enhacements: add support for a second screen, build the embedded operating system with petalinux, show the FPGA IP address on one of the screens, etc.


[youtube-demo]: https://youtu.be/TNlVlC1Tnaw
