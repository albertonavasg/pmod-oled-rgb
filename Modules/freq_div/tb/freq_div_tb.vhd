----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 16.01.2024 18:32:43
-- Design Name: 
-- Module Name: freq_div_tb - Behavioral
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

entity freq_div_tb is
--  Port ( );
end freq_div_tb;

architecture Behavioral of freq_div_tb is

    -- Instantiate the component
    component freq_div is
    Port (
            CLK        : in  std_logic;
            RESET       : in  std_logic;
            ENABLE     : in  std_logic;
            CLK_1_MHZ : out std_logic
    );
    end component;

    -- Clock
    constant clk_period : time := 8 ns;

    -- Signals 
    signal clk, reset, enable, clk_1_MHz : std_logic := '0';

begin

    -- Port Mapping
    freq_div_inst: freq_div
        Port Map(
            CLK        => CLK,
            RESET      => reset,
            ENABLE     => enable,
            CLK_1_MHZ  => clk_1_MHz
        );


    -- Stimulus processes

    clk_proc : process
    begin
        clk <= '0';
        wait for clk_period/2;
        clk <= '1';
        wait for clk_period/2;
    end process;

    stim_proc: process
    begin
        reset <= '1';
        wait for 5*clk_period;
        reset  <= '0';
        enable <= '1';
        wait for 500*clk_period;
    end process;

end Behavioral;
