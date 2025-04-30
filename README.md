# pmod-oled-rgb

Project for Design of Embedded Systems, a subject from my Master's Degree in Industrial Electronics.  
It is usable in `PYNQ-Z2` (and also in `PYNQ-Z1` taking care of physical pinouts).  

<div align="center">
<img src="images/pmod-oled-rgb.png" title="Pmod OLED RGB" width="100%" height="auto"> 
</div>

***

## Contents

- [Project Overview](#project-overview)
- [Design](#design)
    - [The VHDL Block](#the-vhdl-block)
    - [Vivado project: screen](#vivado-project-screen)
    - [Packaging the IP](#packaging-the-ip)
    - [Vivado project: axi_screen](#vivado-project-axi_screen)

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

## Design

### The VHDL block

The base of this design is the VHDL block, which will later be packaged into an AXI4Lite IP.

The architecture is really simple:

```
└── screen_controller
    └── spi_master
```

The core block is `spi_master`.  

In the first version of the project, I created from scratch an `spi_master` block with a fixed speed (1MHz SCK) and also fixed to work in SPI mode 3.

For this second version, I decided to get an `spi_master` block from [OpenCores][open-cores], which is configurable and much more sofisticated.  
I configured it for SPI mode 3 (required by the Pmod OLED RGB) and used a 6.25MHz SCK (125MHz clk input with 20 clock divider). The maximum SCK allowed by the screen is 6.6MHz.  

```
entity spi_master is
    Generic (   
        N              : positive  := 32;  -- 32bit serial word length is default
        CPOL           : std_logic := '0'; -- SPI mode selection (mode 0 default)
        CPHA           : std_logic := '0'; -- CPOL = clock polarity, CPHA = clock phase.
        PREFETCH       : positive  := 2;   -- prefetch lookahead cycles
        SPI_2X_CLK_DIV : positive  := 5    -- for a 100MHz sclk_i, yields a 10MHz SCK
    );
    Port (  
        sclk_i : in std_logic := 'X'; -- high-speed serial interface system clock
        pclk_i : in std_logic := 'X'; -- high-speed parallel interface system clock
        rst_i  : in std_logic := 'X'; -- reset core
        ---- serial interface ----
        spi_ssel_o : out std_logic;        -- spi bus slave select line
        spi_sck_o  : out std_logic;        -- spi bus sck
        spi_mosi_o : out std_logic;        -- spi bus mosi output
        spi_miso_i : in  std_logic := 'X'; -- spi bus spi_miso_i input
        ---- parallel interface ----
        di_req_o   : out std_logic;                                          -- preload lookahead data request line
        di_i       : in  std_logic_vector (N-1 downto 0) := (others => 'X'); -- parallel data in (clocked on rising spi_clk after last bit)
        wren_i     : in  std_logic := 'X';                                   -- user data write enable, starts transmission when interface is idle
        wr_ack_o   : out std_logic;                                          -- write acknowledge
        do_valid_o : out std_logic;                                          -- do_o data valid signal, valid during one spi_clk rising edge.
        do_o       : out std_logic_vector (N-1 downto 0);                    -- parallel output (clocked on rising spi_clk after last bit)
        done_o     : out std_logic                                           -- handshake added to break continuous tx/rx
        --- debug ports: can be removed or left unconnected for the application circuit ---
        -- sck_ena_o     : out std_logic;                     -- debug: internal sck enable signal
        -- sck_ena_ce_o  : out std_logic;                     -- debug: internal sck clock enable signal
        -- do_transfer_o : out std_logic;                     -- debug: internal transfer driver
        -- wren_o        : out std_logic;                     -- debug: internal state of the wren_i pulse stretcher
        -- rx_bit_reg_o  : out std_logic;                     -- debug: internal rx bit
        -- state_dbg_o   : out std_logic_vector (3 downto 0); -- debug: internal state register
        -- core_clk_o    : out std_logic;
        -- core_n_clk_o  : out std_logic;
        -- core_ce_o     : out std_logic;
        -- core_n_ce_o   : out std_logic
        -- sh_reg_dbg_o  : out std_logic_vector (N-1 downto 0) -- debug: internal shift register
    );                      
end spi_master;
```

<br>

The upper level block is `screen_controller` and behaves as follows:

When putting a `1` in `ON_OFF`, the turning_on sequence is executed (some required steps to correctly start using the Pmod OLED RGB, such as managing `POWER_RESET`, `VCC_ENABLE` and `PMOD_ENABLE`, and sending some SPI commands to set the default settings). More info about this can be found in the [Reference Manual][pmod-oled-rgb-reference-manual].  

`ON_OFF_STATUS` indicates:
- `00` OFF
- `01` Turning ON
- `10` Turning OFF
- `11` ON

When the screen is `ON`, the block bypasses the control signals to the `spi_master` block. In the other states, these control signals are managed internally to execute the aforementioned sequences.  

When `SPI_READY` shows a `1`, we can send a byte through SPI, because there is no SPI transtaction in progress.

To start an SPI transaction:
- Put the 8 bit data in `BYTE`
- Set `DC_SELECT` to `0` (data) or `1` (command)
- Set `SPI_TRIGGER` to `1`

The byte will be sent and the block will request another byte putting a `1` in `SPI_DATA_REQUEST`. This allows for continous data transmit (useful for example to send a lot of pixels to draw an image).  
If no more bytes are sent, the transaction will end and `SPI_READY` will show a `1`.

It is still pending to see if the PS will be fast enough to put another `BYTE`, `DC_SELECT` and `SPI_TRIGGER` when required by `SPI_DATA_REQUEST`. If not, the transactions will be done one by one.  

```
entity screen_controller is
    Port (
        -- Sync
        CLK    : in std_logic;
        RESETN : in std_logic;

        -- Control
        ON_OFF      : in  std_logic;
        SPI_TRIGGER : in  std_logic;

        -- Status
        ON_OFF_STATUS : out std_logic_vector(1 downto 0);
        SPI_READY     : out std_logic;

        -- SPI data request
        SPI_DATA_REQUEST : out std_logic;

        -- Data input
        BYTE      : in  std_logic_vector(7 downto 0);
        DC_SELECT : in  std_logic;

        -- Pmod physical pins
        MOSI         : out std_logic;
        SCK          : out std_logic;
        CS           : out std_logic;
        DATA_COMMAND : out std_logic;
        POWER_RESET  : out std_logic;
        VCC_ENABLE   : out std_logic;
        PMOD_ENABLE  : out std_logic
    );
end screen_controller;
```

***

### Vivado project: screen

To create and test the `screen_controller` block, there is a vivado project called `screen`.  
This project can be re-generated using the script `build_screen.bat` (Windows) or `build_screen.sh` (Linux).  
These scripts will invoke the `screen.tcl` script, that generates the Vivado project, contained in its own folder.  

This project has the following hierarchy:

```
└── top
    ├── screen_controller
    │   └── spi_master
    └── screen_tester
```

The behaviour of `screen_controller` is explained above.  

The block `screen_tester` works as follows: if `ON_OFF_STATUS` is ON and `screen_tester` receives a rising edge in  its `ENABLE`, it sends the "ENTIRE_DISPLAY_ON" command, making all the pixels in the screen white.  
If it receives a falling edge in its `ENABLE`, it sends the "ENTIRE_DISPLAY_OFF" command, putting all pixels to black.  

The `top` module instantiates `screen_controller` and `screen_tester` and makes the necessary connections
This way, we have a physical test to see that the commands sent to the screen work.

- SW0: `ON_OFF` of screen_controller
- SW1: `ENABLE` of screen_tester
- LED0: `ON_OFF_STATUS`(0)
- LED1: `ON_OFF_STATUS`(1)
- LED2: `SPI_READY`
- LED3: `ON_OFF`

In this project, there are also testbenches, for the `spi_master`, `screen_controller` and `screen_tester`.
they can be set as top in the simulation folder to be individually executed, and see how each block works.

***

### Packaging the IP

Once I had the VHDL block working and tested, it is the moment to package it in an AXI4Lite IP.
In a Vivado project, in the top bar:  
 - Tools > Create and Package New IP > Create AXI4 Peripheral > [...] > Add IP to repository.

Then, going to the IP Catalog, it can be edited with the IP Packager. It creates a project, where we need to add the user logic blocks (scree_controller and spi_master).

With this IP, I basically connected the control/write ports (ON_OFF, SPI_TRIGGER, BYTE, DC_SELECT) and the status/read ports(ON_OFF_STATUS, SPI_READY, SPI_REQUEST_DATA) to the AXI registers of the IP block.

After this is done, we can re-package the IP and close the temporal project.

***

### Vivado project: axi_screen

To generate what will be the final hardware platform, I created the vivado project `axi_screen`.  
This project can be re-generated using the script `build_axi_screen.bat` (Windows) or `build_axi_screen.sh` (Linux).  
These scripts will invoke the `axi_screen.tcl` script, that generates the Vivado project, contained in its own folder.  
This project only has one block, the HDL wrapper of the block diagram from the picture.

<br>

<div align="center">
<img src="images/screen_block.jpg" title="AXI Screen block Diagram" width="100%" height="auto"> 
</div>

<br>

In the block diagram, we can see the ZYNQ Processing System connected through AXI interface to 2 `screen` IP blocks.  
The physical output ports of the screen IP are the PMOD and 2 LEDs (which show ON_OFF_STATUS).  

We can generate the bitstream and after that export the hardware platform (a file with .xsa extension).
 - Files > Export > Export Hardware > Include Bitstream

To test if this hardware platform works correctly, we can open Vitis (Tools > Launch Vitis IDE).  
We select a workspace or create a new one in a desired folder

> [!TIP]
> It is recommended to use a Vitis workspace with a short path, such as `C:/vitis_ws/` or else we can have some problems when trying to compile the hardware platform or the application

Once we have a Vitis workspace, we create a hardware platform based on the .xsa we exported before (available in this repo: `src/hw/axi_screen_platform.xsa`) and compile it.  

Then we create an application project from the templates: Empty Application.  
In the `Sources > src` we add the C program `screen_platform_test.c` (available in this repo: `src/hw/screen_platform_test.c`).
We can now build it and run it.  
We will see how the LEDs will turn ON (meaning ON_OFF_STATUS from screens A and B are ON "11") and how the screens receive the command "ENTIRE_DISPLAY_ON".

<br>

<div align="center">
<img src="images/screen_platform_test.jpeg" title="Screen Platform test" width="100%" height="auto"> 
</div>

<br>

This setup with two screens is kind of temporal, as one of them needs to be attached with cables because both can't fit in the PMOD connectors side by side.

I decided not to create any scripts to replicate the Vitis project, as it is easier than the Vivado projects and its only purpose is to perform a quick test of the hardware platform.  
The further development will be done with the Petalinux OS: the C application to control the two screens.

> [!CAUTION]  
> There is a typo in the Makefile created by the Vivado IP packager.  
> This typo doesn't affect the standalone project created by Vitis, but generates an error when building the petalinux OS.  
> The typo needs to be corrected as follows:  
>   ```
>   LIBSOURCES=($wildcard *.c) --> LIBSOURCES=$(wildcard *.c)  
>   OUTS = ($wildcard *.o)     --> OUTS = $(wildcard *.o) 
>   ``` 
> Once this is done, it is necessary to repackage the IP and regenerate the hardware platform.  

[comment]: (Links)
[youtube-demo]: https://youtu.be/TNlVlC1Tnaw
[open-cores]: https://opencores.org/
[pmod-oled-rgb-reference-manual]: https://digilent.com/reference/pmod/pmodoledrgb/reference-manual