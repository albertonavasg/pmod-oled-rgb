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
        CLK   : in std_logic;
        RESET : in std_logic;

        SW     : in  std_logic;
        LED    : out std_logic_vector(1 downto 0);

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
    end component;

    -- Clock
    constant clk_period : time := 8 ns;

    -- Signals
    signal clk, reset : std_logic := '0';

    signal sw  : std_logic := '0';
    signal led : std_logic_vector(1 downto 0) := "00";

    signal ja_0_cs     : std_logic := '0';
    signal ja_1_mosi   : std_logic := '0';
    signal ja_2_nc     : std_logic := '0';
    signal ja_3_sck    : std_logic := '0';
    signal ja_4_dc     : std_logic := '0';
    signal ja_5_res    : std_logic := '0';
    signal ja_6_vccen  : std_logic := '0';
    signal ja_7_pmoden : std_logic := '0';

begin

    -- Port Map
    UUT: top
        Port Map (
            CLK         => clk, 
            RESET       => reset, 
            SW          => sw,
            LED         => led,
            JA_0_CS     => ja_0_cs,
            JA_1_MOSI   => ja_1_mosi,
            JA_2_NC     => ja_2_nc,
            JA_3_SCK    => ja_3_sck,
            JA_4_DC     => ja_4_dc,
            JA_5_RES    => ja_5_res,
            JA_6_VCCEN  => ja_6_vccen,
            JA_7_PMODEN => ja_7_pmoden
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
        sw <= '1';
        wait for 100000*clk_period;
        sw <= '0';
        wait for 100000*clk_period;
    end process;

end Behavioral;
