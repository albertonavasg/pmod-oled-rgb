library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity screen_controller_tb is
--  Port ( );
end screen_controller_tb;

architecture Behavioral of screen_controller_tb is

    -- Component Under Test
    component screen_controller is
        Port (
            -- Sync
            CLK    : in std_logic;
            RESETN : in std_logic;
    
            -- Control
            ON_OFF        : in  std_logic;
            SPI_TRIGGER   : in  std_logic;
    
            -- Status
            ON_OFF_STATUS : out std_logic_vector(1 downto 0);
            SPI_READY     : out std_logic;
    
            -- Data input
            BYTE      : in  std_logic_vector(7 downto 0);
            DC_SELECT : in  std_logic;
    
            -- Pmod physical pins
            MOSI         : out std_logic;
            SCK          : out std_logic;
            CS           : out std_logic;
            DATA_COMMAND : out std_logic;
            POWER_RESET  : out std_logic;
            VCC_ENABLE   : out std_logic;
            PMOD_ENABLE  : out std_logic
        );
    end component;

    -- Clock
    constant clk_period : time := 8 ns;

    -- Signals
    signal clk           : std_logic := '0';
    signal resetn        : std_logic := '0';

    signal on_off        : std_logic := '0';
    signal spi_trigger   : std_logic := '0';

    signal on_off_status : std_logic_vector(1 downto 0) := (others => '0');
    signal spi_ready     : std_logic := '0';

    signal byte          : std_logic_vector(7 downto 0) := (others => '0');
    signal dc_select     : std_logic := '0';

    signal mosi          : std_logic := '0';
    signal sck           : std_logic := '0';
    signal cs            : std_logic := '0';
    signal data_command  : std_logic := '0';
    signal power_reset   : std_logic := '0';
    signal vcc_enable    : std_logic := '0';
    signal pmod_enable   : std_logic := '0';

begin

    -- Port Map
    CUT : screen_controller
        Port Map (
            -- Sync
            CLK    => clk,
            RESETN => resetn,
    
            -- Control
            ON_OFF      => on_off,
            SPI_TRIGGER => spi_trigger,
    
            -- Status
            ON_OFF_STATUS => on_off_status,
            SPI_READY     => spi_ready,
    
            -- Data input
            BYTE      => byte,
            DC_SELECT => dc_select,
    
            -- Pmod physical pins
            MOSI         => mosi,
            SCK          => sck,
            CS           => cs,
            DATA_COMMAND => data_command,
            POWER_RESET  => power_reset,
            VCC_ENABLE   => vcc_enable,
            PMOD_ENABLE  => pmod_enable
        );

    clk_proc : process
    begin
        clk <= '0';
        wait for clk_period/2;
        clk <= '1';
        wait for clk_period/2;
    end process;

    stim_proc : process
    begin
        resetn  <= '0';
        on_off  <= '0';
        wait for 5*clk_period;
        resetn  <= '1';
        on_off  <= '0';
        wait for 1*clk_period;
        on_off <= '1';
        wait for 100000*clk_period;
        on_off <= '0';
        wait for 100000*clk_period;
    end process;

end Behavioral;
