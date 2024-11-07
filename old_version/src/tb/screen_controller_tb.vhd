library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity screen_controller_tb is
--  Port ( );
end screen_controller_tb;

architecture Behavioral of screen_controller_tb is

    -- Instantiate the Component Under Test
    component screen_controller is
        Port (
            -- Basic
            CLK   : in std_logic;
            RESETN : in std_logic;

            -- Control
            ON_OFF        : in  std_logic;
            ON_OFF_STATUS : out std_logic_vector(1 downto 0);
            START         : in  std_logic;
            READY         : out std_logic;

            -- Power 
            POWER_RESET : out std_logic;
            VCC_ENABLE  : out std_logic;
            PMOD_ENABLE : out std_logic;
            
            -- Data
            DATA             : in  std_logic_vector(7 downto 0);
            DATA_COMMAND_IN  : in  std_logic;
            DATA_COMMAND_OUT : out std_logic;

            -- SPI
            MOSI : out std_logic;
            SCK  : out std_logic;
            CS   : out std_logic;

            -- Debug
            SEQ_COUNTER_DBG           : out std_logic_vector(9 downto 0);
            START_SIGNAL_DBG          : out std_logic;
            READY_SIGNAL_DBG          : out std_logic;
            DATA_SIGNAL_DBG           : out std_logic_vector(7 downto 0);
            DATA_COMMAND_INTERNAL_DBG : out std_logic;
            EXPIRED_COUNTER_5US_DBG   : out std_logic;
            EXPIRED_COUNTER_20MS_DBG  : out std_logic;
            EXPIRED_COUNTER_25MS_DBG  : out std_logic;
            EXPIRED_COUNTER_100MS_DBG : out std_logic;
            EXPIRED_COUNTER_400MS_DBG : out std_logic;
            EXPIRED_COUNTER_SPI_DBG   : out std_logic
        );
    end component;

    -- Clock
    constant clk_period : time := 8 ns;

    -- Signals
    signal clk, resetn                                  : std_logic                    := '0';
    signal on_off, power_reset, vcc_enable, pmod_enable : std_logic                    := '0';
    signal on_off_status                                : std_logic_vector(1 downto 0) := "00";
    signal start, ready                                 : std_logic                    := '0';
    signal data                                         : std_logic_vector(7 downto 0) := "00000000";
    signal data_command_in, data_command_out            : std_logic                    := '0';
    signal mosi, sck, cs                                : std_logic                    := '0';

    -- Debug signals
    signal seq_counter_dbg           : std_logic_vector(9 downto 0) := (Others => '0');
    signal start_signal_dbg          : std_logic                    := '0';
    signal ready_signal_dbg          : std_logic                    := '0';
    signal data_signal_dbg           : std_logic_vector(7 downto 0) := "00000000";
    signal data_command_internal_dbg : std_logic                    := '0';
    signal expired_counter_5us_dbg   : std_logic := '0';
    signal expired_counter_20ms_dbg  : std_logic := '0';
    signal expired_counter_25ms_dbg  : std_logic := '0';
    signal expired_counter_100ms_dbg : std_logic := '0';
    signal expired_counter_400ms_dbg : std_logic := '0';
    signal expired_counter_spi_dbg   : std_logic := '0';

begin

    -- Port Map
    
    UUT : screen_controller
        Port Map (
            -- Basic       
            CLK   => clk,
            RESETN => resetn,

            -- Control  
            ON_OFF        => on_off,  
            ON_OFF_STATUS => on_off_status,
            START         => start,
            READY         => ready,

            -- Power      
            POWER_RESET => power_reset,
            VCC_ENABLE  => vcc_enable,
            PMOD_ENABLE => pmod_enable,

            -- Data       
            DATA             => data,
            DATA_COMMAND_IN  => data_command_in,
            DATA_COMMAND_OUT => data_command_out,

            -- SPI        
            MOSI => mosi,
            SCK  => sck,
            CS   => cs,

            -- Debug
            SEQ_COUNTER_DBG           => seq_counter_dbg,
            START_SIGNAL_DBG          => start_signal_dbg,
            READY_SIGNAL_DBG          => ready_signal_dbg,
            DATA_SIGNAL_DBG           => data_signal_dbg,
            DATA_COMMAND_INTERNAL_DBG => data_command_internal_dbg,
            EXPIRED_COUNTER_5US_DBG   => expired_counter_5us_dbg,
            EXPIRED_COUNTER_20MS_DBG  => expired_counter_20ms_dbg,
            EXPIRED_COUNTER_25MS_DBG  => expired_counter_25ms_dbg,
            EXPIRED_COUNTER_100MS_DBG => expired_counter_100ms_dbg,
            EXPIRED_COUNTER_400MS_DBG => expired_counter_400ms_dbg,
            EXPIRED_COUNTER_SPI_DBG   => expired_counter_spi_dbg
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
