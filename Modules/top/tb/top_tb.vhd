----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 29.01.2024 16:50:03
-- Design Name: 
-- Module Name: top_tb - Behavioral
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

entity top_tb is
--  Port ( );
end top_tb;

architecture Behavioral of top_tb is

    -- Instantiate the component
    component top is
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
    end component;

    -- Clock
    constant clk_period : time := 1 ns;

    -- Signals
    signal clk, reset : std_logic := '0';
    signal sw : std_logic_vector(1 downto 0) := "00";
    signal led : std_logic_vector(3 downto 0) := "0000";
    signal led4_b, led4_g, led4_r, led5_b, led5_g, led5_r : std_logic := '0';
    signal ja : std_logic_vector(7 downto 0);


begin

    -- Port Map
    UUT: top
        Port Map (
            clk    => clk, 
            reset  => reset, 
            sw     => sw,
            led    => led,
            led4_b => led4_b,
            led4_g => led4_g,
            led4_r => led4_r,
            led5_b => led5_b,
            led5_g => led5_g,
            led5_r => led5_r,
            ja     => ja
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
        wait for 10*clk_period;
        reset <= '0';
        sw (0) <= '1';
        wait for 10*clk_period;
        sw(1) <= '1';
        wait for 500*clk_period;
    end process;

end Behavioral;
