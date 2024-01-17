----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10.01.2024 19:06:22
-- Design Name: 
-- Module Name: screen_controller_tb - Behavioral
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

entity screen_controller_tb is
--  Port ( );
end screen_controller_tb;

architecture Behavioral of screen_controller_tb is

    -- Instantiate the Component Under Test
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

    -- Clock
    constant clk_period : time := 1 ns;

    -- Signals
    signal clk, reset                                   : std_logic                    := '0';
    signal on_off, power_reset, vcc_enable, pmod_enable : std_logic                    := '0';
    signal on_off_status                                : std_logic_vector(1 downto 0) := "00";
    signal start, ready                                 : std_logic                    := '0';
    signal data                                         : std_logic_vector(7 downto 0) := "00000000";
    signal data_command_in, data_command_out            : std_logic                    := '0';
    signal mosi, sck, cs                                : std_logic                    := '0';

    -- Debug signals
    signal seq_counter_dbg           : std_logic_vector(9 downto 0) := (Others => '0');
    signal start_signal_dbg          : std_logic                    := '0';
    signal ready_signal_dbg          : std_logic                    := '0';
    signal data_signal_dbg           : std_logic_vector(7 downto 0) := "00000000";
    signal data_command_internal_dbg : std_logic                    := '0';
begin

    -- Port Map
    UUT : screen_controller
        Port Map (
            -- Basic       
            clk   => clk,
            reset => reset,

            -- Power      
            on_off      => on_off,
            power_reset => power_reset,
            vcc_enable  => vcc_enable,
            pmod_enable => pmod_enable,

            -- Control    
            on_off_status => on_off_status,
            start         => start,
            ready         => ready,

            -- Data       
            data             => data,
            data_command_in  => data_command_in,
            data_command_out => data_command_out,

            -- SPI        
            mosi => mosi,
            sck  => sck,
            cs   => cs,

            -- Debug
            seq_counter_dbg           => seq_counter_dbg,
            start_signal_dbg          => start_signal_dbg,
            ready_signal_dbg          => ready_signal_dbg,
            data_signal_dbg           => data_signal_dbg,
            data_command_internal_dbg => data_command_internal_dbg
        );

    -- Stimulus processes

    clk_proc : process
    begin
        clk <= '0';
        wait for clk_period/2;
        clk <= '1';
        wait for clk_period/2;
    end process;

    stim_proc : process
    begin
        reset  <= '1';
        on_off <= '0';
        wait for 5*clk_period;
        reset  <= '0';
        on_off <= '0';
        wait for 1*clk_period;
        reset  <= '0';
        on_off <= '1';
        wait for 1500*clk_period;
        on_off <= '0';
        wait for 1500*clk_period;
    end process;

end Behavioral;
