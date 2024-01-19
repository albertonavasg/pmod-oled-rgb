----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 16.01.2024 17:27:05
-- Design Name: 
-- Module Name: freq_div - Behavioral
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
  use IEEE.STD_LOGIC_1164.all;

  -- Uncomment the following library declaration if using
  -- arithmetic functions with Signed or Unsigned values
  --use IEEE.NUMERIC_STD.ALL;
  -- Uncomment the following library declaration if instantiating
  -- any Xilinx leaf cells in this code.
  --library UNISIM;
  --use UNISIM.VComponents.all;

entity freq_div is
    Port (
            clk       : in  std_logic;
            reset     : in  std_logic;
            enable    : in  std_logic;
            clk_1_MHz : out std_logic
    );
end entity;

architecture Behavioral of freq_div is

    constant max_counter : integer := 125; -- From 125MHz to 1 MHz
    signal counter       : integer range 0 to max_counter := 0;

begin

    counter_proc: process(clk, reset)
    begin
        if (reset = '1') then
            counter <= 0;
        elsif (rising_edge(clk)) then
            if (enable = '1') then
                if (counter < max_counter) then
                    counter <= counter + 1;
                else
                    counter <= 0;
                end if;
            else 
                counter <= 0;
            end if;
        end if;
    end process;

    clk_1_MHz <= '1' when (counter >= max_counter/2) else '0';

end architecture;
