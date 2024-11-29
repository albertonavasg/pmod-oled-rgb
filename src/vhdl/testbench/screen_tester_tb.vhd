library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity screen_tester_tb is
--  Port ( );
end screen_tester_tb;

architecture Behavioral of screen_tester_tb is

    -- Component Under Test
    component screen_tester is
        Port ( 
            -- Sync
            CLK    : in std_logic;
            RESETN : in std_logic;

            -- Enable
            ENABLE : in std_logic;

            -- Control
            SPI_TRIGGER : out std_logic;

            -- Status
            ON_OFF_STATUS : in  std_logic_vector(1 downto 0);
            READY         : in  std_logic;

            -- Data
            BYTE      : out std_logic_vector(7 downto 0);
            DC_SELECT : out std_logic
        );
    end component;

    -- Clock
    constant clk_period : time := 8 ns;

    -- Signals
    signal clk           : std_logic := '0';
    signal resetn        : std_logic := '0';

    signal enable        : std_logic := '0';

    signal spi_trigger   : std_logic := '0';

    signal on_off_status : std_logic_vector(1 downto 0) := "00";
    signal ready         : std_logic := '0';

    signal byte          : std_logic_vector(7 downto 0) := "00000000";
    signal dc_select     : std_logic := '0';

begin

    -- Port Map
    screen_tester_inst: screen_tester
        Port Map (
            CLK    => clk,
            RESETN => resetn,

            -- Enable
            ENABLE => enable,

            -- Control
            SPI_TRIGGER => spi_trigger,

            -- Status
            ON_OFF_STATUS => on_off_status,
            READY         => ready,

            -- Data
            BYTE      => byte,
            DC_SELECT => dc_select
        );

    clk_proc : process
    begin
        clk <= '0';
        wait for clk_period/2;
        clk <= '1';
        wait for clk_period/2;
    end process;

    stim_proc: process
    begin
        wait for 5*clk_period;
            resetn <= '1';
        wait for 5*clk_period;
            on_off_status <= "11";
            ready         <= '1';
        wait for 5*clk_period;
            enable <= '1';
        wait for 50*clk_period;
            enable <= '0';
        wait;
    end process;

end Behavioral;
