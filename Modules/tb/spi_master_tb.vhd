library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

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
            DONE_DBG                  : out std_logic;
            BIT_COUNTER_DBG           : out std_logic_vector(2 downto 0);
            SHIFT_DATA_DBG            : out std_logic_vector(7 downto 0);
            START_DELAY_DBG           : out std_logic;
            START_RISING_EDGE_DBG     : out std_logic;
            CLK_1_MHZ_DBG             : out std_logic;
            CLK_1MHZ_DELAY_DBG        : out std_logic;
            CLK_1MHZ_RISING_EDGE_DBG  : out std_logic;
            CLK_1MHZ_FALLING_EDGE_DBG : out std_logic;
            EXPIRED_TIMER_1US_DBG     : out std_logic
        );
    end component;

    -- Clock
    constant clk_period : time := 8 ns;

    -- Signals
    signal clk, reset, start, ready, mosi, sck, cs : std_logic;
    signal data                                    : std_logic_vector(7 downto 0);

    signal done_dbg                  : std_logic;
    signal bit_counter_dbg           : std_logic_vector(2 downto 0);
    signal shift_data_dbg            : std_logic_vector(7 downto 0);
    signal start_delay_dbg           : std_logic;
    signal start_rising_edge_dbg     : std_logic;
    signal clk_1_mhz_dbg             : std_logic;
    signal clk_1mhz_delay_dbg        : std_logic;
    signal clk_1mhz_rising_edge_dbg  : std_logic;
    signal clk_1mhz_falling_edge_dbg : std_logic;
    signal expired_timer_1us_dbg     : std_logic;

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
            DONE_DBG                  => done_dbg,
            BIT_COUNTER_DBG           => bit_counter_dbg,
            SHIFT_DATA_DBG            => shift_data_dbg,
            START_DELAY_DBG           => start_delay_dbg,
            START_RISING_EDGE_DBG     => start_rising_edge_dbg,
            CLK_1_MHZ_DBG             => clk_1_mhz_dbg,
            CLK_1MHZ_DELAY_DBG        => clk_1mhz_delay_dbg,
            CLK_1MHZ_RISING_EDGE_DBG  => clk_1mhz_rising_edge_dbg,
            CLK_1MHZ_FALLING_EDGE_DBG => clk_1mhz_falling_edge_dbg,
            EXPIRED_TIMER_1US_DBG     => expired_timer_1us_dbg
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
            wait for 1300*clk_period;
        start <= '1';
        data  <= "00110010";
            wait for 1*clk_period;
        start <= '0';
            wait for 500*clk_period;
        start <= '1'; --Useless start to test robustness 
        data  <= "00110010";
            wait for 2*clk_period;
        start <= '0';
            wait for 1500*clk_period;
    end process;

end Behavioral;
