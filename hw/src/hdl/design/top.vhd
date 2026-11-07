library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

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

    -- Signals for reset debounce and sync
    signal reset_meta : std_logic := '0';
    signal reset_sync : std_logic := '0';
    signal reset_db   : std_logic := '0';
    signal resetn     : std_logic := '0';

    signal db_count   : unsigned(23 downto 0) := (others => '0');
    constant DB_MAX   : integer := 1250000; -- 10ms at 125MHz

begin

    -- Sync reset
    sync_rst : process(clk)
    begin
        if rising_edge(clk) then
            reset_meta <= RESET;
            reset_sync <= reset_meta;
        end if;
    end process;

    -- Debounce reset
    debounce_rst : process(clk)
    begin
        if rising_edge(clk) then
            if reset_sync = reset_db then
                db_count <= (others => '0');
            else
                if db_count < DB_MAX then
                    db_count <= db_count + 1;
                else
                    reset_db <= reset_sync;
                    db_count <= (others => '0');
                end if;
            end if;
        end if;
    end process;

    -- Invert reset
    resetn <= not reset_db;

    screen_controller_inst: entity work.screen_controller
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

    screen_tester_inst: entity work.screen_tester
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
    JA(2) <= 'Z';  -- Unused
    JA(3) <= sck;
    JA(4) <= data_command;
    JA(5) <= power_reset;
    JA(6) <= vcc_enable;
    JA(7) <= pmod_enable;

end Behavioral;
