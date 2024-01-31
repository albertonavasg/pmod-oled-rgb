----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 16.01.2024 19:04:30
-- Design Name: 
-- Module Name: top - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity top is
    Port ( 
        CLK   : in std_logic;
        RESET : in std_logic;

        SW     : in  std_logic_vector(1 downto 0);
        LED    : out std_logic_vector(3 downto 0);
        LED4_B : out std_logic;
        LED4_G : out std_logic;
        LED4_R : out std_logic;
        LED5_B : out std_logic;
        LED5_G : out std_logic;
        LED5_R : out std_logic;

        -- PmodA
        JA_0_CS     : out std_logic;
        JA_1_MOSI   : out std_logic;
        JA_2_NC     : out std_logic;
        JA_3_SCK    : out std_logic;
        JA_4_DC     : out std_logic;
        JA_5_RES    : out std_logic;
        JA_6_VCCEN  : out std_logic;
        JA_7_PMODEN : out std_logic
    );
end top;

architecture Behavioral of top is

    -- Instantiate the components

    component screen_controller is
        Port (
            -- Basic
            CLK   : in std_logic;
            RESET : in std_logic;

            -- Power 
            ON_OFF      : in  std_logic;
            POWER_RESET : out std_logic;
            VCC_ENABLE  : out std_logic;
            PMOD_ENABLE : out std_logic;

            -- Control
            ON_OFF_STATUS : out std_logic_vector(1 downto 0);
            START         : in  std_logic;
            READY         : out std_logic;

            -- Data
            DATA             : in  std_logic_vector(7 downto 0);
            DATA_COMMAND_IN  : in  std_logic;
            DATA_COMMAND_OUT : out std_logic;

            -- SPI
            MOSI : out std_logic;
            SCK  : out std_logic;
            CS   : out std_logic;

            -- Debug
            SEQ_COUNTER_DBG           : out std_logic_vector(9 downto 0);
            START_SIGNAL_DBG          : out std_logic;
            READY_SIGNAL_DBG          : out std_logic;
            DATA_SIGNAL_DBG           : out std_logic_vector(7 downto 0);
            DATA_COMMAND_INTERNAL_DBG : out std_logic;
            EXPIRED_COUNTER_5US_DBG   : out std_logic;
            EXPIRED_COUNTER_20MS_DBG  : out std_logic;
            EXPIRED_COUNTER_25MS_DBG  : out std_logic;
            EXPIRED_COUNTER_100MS_DBG : out std_logic;
            EXPIRED_COUNTER_400MS_DBG : out std_logic;
            EXPIRED_COUNTER_SPI_DBG   : out std_logic
        );
    end component;

    component freq_div is
        Port (
                CLK       : in  std_logic;
                RESET     : in  std_logic;
                ENABLE    : in  std_logic;
                CLK_1_MHZ : out std_logic
        );
    end component;

    -- Signals 
     signal clk_1_MHz     : std_logic;
     signal enable        : std_logic;

     signal on_off        : std_logic;
     signal power_reset   : std_logic;
     signal vcc_enable    : std_logic;
     signal pmod_enable   : std_logic;

    signal on_off_status    : std_logic_vector(1 downto 0);
    signal start            : std_logic;
    signal ready            : std_logic;

    signal data             : std_logic_vector(7 downto 0);
    signal data_command_in  : std_logic;
    signal data_command_out : std_logic;

    signal mosi : std_logic;
    signal sck  : std_logic;
    signal cs   : std_logic;

    -- Debug signals
    signal seq_counter_dbg           : std_logic_vector(9 downto 0); 
    signal start_signal_dbg          : std_logic;
    signal ready_signal_dbg          : std_logic;
    signal data_signal_dbg           : std_logic_vector(7 downto 0);
    signal data_command_internal_dbg : std_logic;

    signal expired_counter_5us_dbg   : std_logic;
    signal expired_counter_20ms_dbg  : std_logic;
    signal expired_counter_25ms_dbg  : std_logic;
    signal expired_counter_100ms_dbg : std_logic;
    signal expired_counter_400ms_dbg : std_logic;
    signal expired_counter_spi_dbg   : std_logic;

begin

    -- Port Maping

    screen_controller_inst: screen_controller
        Port Map (
            -- Basic
            CLK   => clk_1_MHz,
            RESET => reset,

            -- Power 
            ON_OFF      => on_off,
            POWER_RESET => power_reset,
            VCC_ENABLE  => vcc_enable,
            PMOD_ENABLE => pmod_enable,

            -- Control
            ON_OFF_STATUS => on_off_status,
            START         => start, 
            READY         => ready, 

            -- Data
            DATA             => data,
            DATA_COMMAND_IN  => data_command_in,
            DATA_COMMAND_OUT => data_command_out,

            -- SPI
            MOSI => mosi,
            SCK  => sck,
            CS   => cs,

            -- Debug
            SEQ_COUNTER_DBG            => seq_counter_dbg,
            START_SIGNAL_DBG           => start_signal_dbg,
            READY_SIGNAL_DBG           => ready_signal_dbg,
            DATA_SIGNAL_DBG            => data_signal_dbg,
            DATA_COMMAND_INTERNAL_DBG  => data_command_internal_dbg,
            EXPIRED_COUNTER_5US_DBG    => expired_counter_5us_dbg,
            EXPIRED_COUNTER_20MS_DBG   => expired_counter_20ms_dbg,
            EXPIRED_COUNTER_25MS_DBG   => expired_counter_25ms_dbg,
            EXPIRED_COUNTER_100MS_DBG  => expired_counter_100ms_dbg,
            EXPIRED_COUNTER_400MS_DBG  => expired_counter_400ms_dbg,
            EXPIRED_COUNTER_SPI_DBG    => expired_counter_spi_dbg
        );

    freq_div_inst: freq_div
        Port Map (
            CLK       => clk,
            RESET     => reset,
            ENABLE    => enable,
            CLK_1_MHZ => clk_1_MHz
        );

    enable <= sw(1);
    on_off <= sw(0);

    LED5_G <= on_off_status(1);
    LED4_G <= on_off_status(0);

    LED(3) <= clk_1_MHz;
    LED(2) <= power_reset;
    LED(1) <= pmod_enable;
    LED(0) <= expired_counter_20ms_dbg;

    JA_0_CS     <= cs;
    JA_1_MOSI   <= mosi;
    JA_2_NC     <= '0';
    JA_3_SCK    <= sck;
    JA_4_DC     <= data_command_out;
    JA_5_RES    <= power_reset;
    JA_6_VCCEN  <= vcc_enable;
    JA_7_PMODEN <= pmod_enable;

end Behavioral;
