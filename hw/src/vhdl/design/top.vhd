library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity top is
    Port ( 
        CLK   : in std_logic;
        RESET : in std_logic;

        SW  : in  std_logic_vector(1 downto 0);
        LED : out std_logic_vector(3 downto 0);

        -- PmodA
        JA : out std_logic_vector(7 downto 0)
    );
end top;

architecture Behavioral of top is

    -- Components
    component screen_controller is
        Port (
            -- Sync
            CLK    : in std_logic;
            RESETN : in std_logic;
    
            -- Control
            ON_OFF      : in  std_logic;
            SPI_TRIGGER : in  std_logic;
    
            -- Status
            ON_OFF_STATUS : out std_logic_vector(1 downto 0);
            SPI_READY     : out std_logic;
            
            -- SPI data request
            SPI_DATA_REQUEST : out std_logic;
            
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
            SPI_READY     : in  std_logic;
    
            -- Data
            BYTE      : out std_logic_vector(7 downto 0);
            DC_SELECT : out std_logic
        );
    end component;

    -- Signals (screen_controller)
    signal on_off      : std_logic;
    signal spi_trigger : std_logic := '0';

    signal on_off_status : std_logic_vector(1 downto 0);
    signal spi_ready     : std_logic;

    signal byte      : std_logic_vector(7 downto 0) := "00000000";
    signal dc_select : std_logic := '0';

    signal mosi          : std_logic;
    signal sck           : std_logic;
    signal cs            : std_logic;
    signal data_command  : std_logic;
    signal power_reset   : std_logic;
    signal vcc_enable    : std_logic;
    signal pmod_enable   : std_logic;

    -- Signals (screen_tester)
    signal enable_screen_tester : std_logic := '0';

    -- Signal resetn
    signal resetn : std_logic;

begin

    -- Invert reset
    resetn <= not RESET;

    -- Port Map
    screen_controller_inst: screen_controller
        Port Map (
            -- Sync
            CLK    => CLK,
            RESETN => resetn,
    
            -- Control
            ON_OFF      => on_off,
            SPI_TRIGGER => spi_trigger,
    
            -- Status
            ON_OFF_STATUS => on_off_status,
            SPI_READY     => spi_ready,
            
            -- SPI data request
            SPI_DATA_REQUEST => open,

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
    
    screen_tester_inst: screen_tester
        Port Map (
            -- Sync
            CLK    => CLK,
            RESETN => resetn,
    
            -- Enable
            ENABLE => enable_screen_tester,
    
            -- Control
            SPI_TRIGGER => spi_trigger,
    
            -- Status
            ON_OFF_STATUS => on_off_status,
            SPI_READY     => spi_ready,
    
            -- Data
            BYTE      => byte,
            DC_SELECT => dc_select
        );

    -- Physical pins
    enable_screen_tester <= SW(1);
    on_off               <= SW(0);

    LED <= on_off & spi_ready & on_off_status;

    JA(0) <= cs;
    JA(1) <= mosi;
    JA(2) <= '0';
    JA(3) <= sck;
    JA(4) <= data_command;
    JA(5) <= power_reset;
    JA(6) <= vcc_enable;
    JA(7) <= pmod_enable;

end Behavioral;
