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
        clk   : in std_logic;
        reset : in std_logic;

        sw     : in  std_logic_vector(1 downto 0);
        led    : out std_logic_vector(3 downto 0);
        led4_b : out std_logic;
        led4_g : out std_logic;
        led4_r : out std_logic;
        led5_b : out std_logic;
        led5_g : out std_logic;
        led5_r : out std_logic;

        -- PmodA
        ja : out std_logic_vector(7 downto 0)
    );
end top;

architecture Behavioral of top is

    -- Instantiate the components

    component screen_controller is
        Port (
            -- Basic
            clk   : in std_logic;
            reset : in std_logic;

            -- Power 
            on_off      : in  std_logic;
            power_reset : out std_logic;
            vcc_enable  : out std_logic;
            pmod_enable : out std_logic;

            -- Control
            on_off_status : out std_logic_vector(1 downto 0);
            start         : in  std_logic;
            ready         : out std_logic;

            -- Data
            data             : in  std_logic_vector(7 downto 0);
            data_command_in  : in  std_logic;
            data_command_out : out std_logic;

            -- SPI
            mosi : out std_logic;
            sck  : out std_logic;
            cs   : out std_logic;

            -- Debug
            seq_counter_dbg           : out std_logic_vector(9 downto 0);
            start_signal_dbg          : out std_logic;
            ready_signal_dbg          : out std_logic;
            data_signal_dbg           : out std_logic_vector(7 downto 0);
            data_command_internal_dbg : out std_logic
        );
    end component;

    component freq_div is
        Port (
                clk       : in  std_logic;
                reset     : in  std_logic;
                enable    : in  std_logic;
                clk_1_MHz : out std_logic
        );
    end component;

    -- Signals 
     signal clk_1_MHz_signal     : std_logic;
     signal on_off_status_signal : std_logic_vector(1 downto 0);
     signal power_reset_signal   : std_logic;
     signal vcc_enable_signal    : std_logic;
     signal pmod_enable_signal   : std_logic;

    signal start_signal            : std_logic;
    signal ready_signal            : std_logic;
    signal data_signal             : std_logic_vector(7 downto 0);
    signal data_command_in_signal  : std_logic;
    signal data_command_out_signal : std_logic;

    signal mosi_signal : std_logic;
    signal sck_signal  : std_logic;
    signal cs_signal   : std_logic;

    -- Debug signals
    signal seq_counter_dbg_signal           : std_logic_vector(9 downto 0); 
    signal start_signal_dbg_signal          : std_logic;
    signal ready_signal_dbg_signal          : std_logic;
    signal data_signal_dbg_signal           : std_logic_vector(7 downto 0);
    signal data_command_internal_dbg_signal : std_logic;

begin

    -- Port Maping

    screen_controller_inst: screen_controller
        Port Map (
            -- Basic
            clk   => clk_1_MHz_signal,
            reset => reset,

            -- Power 
            on_off      => sw(1),
            power_reset => power_reset_signal,
            vcc_enable  => vcc_enable_signal,
            pmod_enable => pmod_enable_signal,

            -- Control
            on_off_status => on_off_status_signal,
            start         => start_signal, 
            ready         => ready_signal, 

            -- Data
            data             => data_signal,
            data_command_in  => data_command_in_signal,
            data_command_out => ja(4),

            -- SPI
            mosi => ja(1),
            sck  => ja(3),
            cs   => ja(0),

            -- Debug
            seq_counter_dbg           => seq_counter_dbg_signal,
            start_signal_dbg          => start_signal_dbg_signal,
            ready_signal_dbg          => ready_signal_dbg_signal,
            data_signal_dbg           => data_signal_dbg_signal,
            data_command_internal_dbg => data_command_internal_dbg_signal
        );

    freq_div_inst: freq_div
        Port Map (
            clk       => clk,
            reset     => reset,
            enable    => sw(0),
            clk_1_MHz => clk_1_MHz_signal
        );

    led5_b <= on_off_status_signal(1);
    led4_b <= on_off_status_signal(0);

    led(3) <= clk_1_MHz_signal;
    led(2) <= power_reset_signal;
    led(1) <= vcc_enable_signal;
    led(0) <= pmod_enable_signal;

end Behavioral;
