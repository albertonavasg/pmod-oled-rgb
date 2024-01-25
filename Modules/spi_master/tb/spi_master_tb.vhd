----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 21.12.2023 18:09:46
-- Design Name: 
-- Module Name: spi_master_tb - Behavioral
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

entity spi_master_tb is
--  Port ( );
end spi_master_tb;

architecture Behavioral of spi_master_tb is

    -- Instantiate the Component Under Test
    component spi_master is
        Port (
            -- Basic
            CLK   : in std_logic;
            RESET : in std_logic;

            -- Control
            START : in  std_logic;
            READY : out std_logic;

            -- SPI
            MOSI : out std_logic;
            SCK  : out std_logic;
            CS   : out std_logic;

            -- Data
            DATA : in std_logic_vector(7 downto 0);

            -- Debug 
            DONE_DBG              : out std_logic;
            BIT_COUNTER_DBG       : out std_logic_vector(2 downto 0);
            SHIFT_DATA_DBG        : out std_logic_vector(7 downto 0);
            START_DELAY_DBG       : out std_logic;
            START_RISING_EDGE_DBG : out std_logic
        );
    end component;

    -- Clock
    constant clk_period : time := 1 ns;

    -- Signals
    signal clk, reset, start, ready, mosi, sck, cs : std_logic;
    signal data                                    : std_logic_vector(7 downto 0);

    signal done_dbg              : std_logic;
    signal bit_counter_dbg       : std_logic_vector(2 downto 0);
    signal shift_data_dbg        : std_logic_vector(7 downto 0);
    signal start_delay_dbg       : std_logic;
    signal start_rising_edge_dbg : std_logic;

begin

    -- Port Mapping
    UUT: spi_master
        Port Map ( 
            -- Basic
            CLK   => clk,
            RESET => reset,

            -- Control
            START => start,
            READY => ready,

            -- SPI
            MOSI => mosi,
            SCK  => sck,
            CS   => cs,

            -- Data
            DATA => data,

            -- Debug
            DONE_DBG              => done_dbg,
            BIT_COUNTER_DBG       => bit_counter_dbg,
            SHIFT_DATA_DBG        => shift_data_dbg,
            START_DELAY_DBG       => start_delay_dbg,
            START_RISING_EDGE_DBG => start_rising_edge_dbg
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
        reset <= '1';
        start <= '0';
        data  <= "00000000";
        wait for 2*clk_period;
        reset <= '0';
        start <= '0';
        data  <= "10011001";
        wait for 2*clk_period;
        start <= '1';
        wait for 3*clk_period;
        start <= '0';
        wait for 10*clk_period;
        start <= '1';
        data  <= "00110010";
        wait for 1*clk_period;
        start <= '0';
        wait for 5*clk_period;
        start <= '1';
        wait for 1*clk_period;
        start <= '0';
        wait for 20*clk_period;
    end process;

end Behavioral;
