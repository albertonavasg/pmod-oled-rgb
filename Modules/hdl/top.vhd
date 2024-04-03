library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity top is
    Port ( 
        CLK   : in std_logic;
        RESET : in std_logic;

        SW     : in  std_logic_vector(1 downto 0);
        LED    : out std_logic_vector(2 downto 0);

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
end top;

architecture Behavioral of top is

    -- For ILA DEBUG
    attribute mark_debug : string;

    -- Instantiate the components
    component screen_controller is
        Port (
            -- Basic
            CLK   : in std_logic;
            RESET : in std_logic;

            -- Power 
            ON_OFF      : in  std_logic;
            POWER_RESET : out std_logic;
            VCC_ENABLE  : out std_logic;
            PMOD_ENABLE : out std_logic;

            -- Control
            ON_OFF_STATUS : out std_logic_vector(1 downto 0);
            START         : in  std_logic;
            READY         : out std_logic;

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

    component screen_tester is
        Port ( 
            -- Basic
            CLK    : in std_logic;
            RESET  : in std_logic;

            -- Enable
            ENABLE : in std_logic;

            -- Control
            ON_OFF_STATUS : in  std_logic_vector(1 downto 0);
            START         : out std_logic;
            READY         : in  std_logic;

            -- Data
            DATA         : out std_logic_vector(7 downto 0);
            DATA_COMMAND : out std_logic;

            -- Debug
            ENABLE_DELAY_DBG        : out std_logic;
            ENABLE_RISING_EDGE_DBG  : out std_logic;
            ENABLE_FALLING_EDGE_DBG : out std_logic;
            SEQ_COUNTER_DBG         : out std_logic_vector(9 downto 0);
            SEND_ON_FLAG_DBG        : out std_logic;
            SEND_OFF_FLAG_DBG       : out std_logic;
            COMMAND_SENT_FLAG_DBG   : out std_logic
        );
    end component;

    -- Signals (screen_controller)
    signal on_off        : std_logic;
    signal power_reset   : std_logic;
    signal vcc_enable    : std_logic;
    signal pmod_enable   : std_logic;

    signal on_off_status    : std_logic_vector(1 downto 0);
    signal start            : std_logic := '0';
    signal ready            : std_logic;

    signal data             : std_logic_vector(7 downto 0) := "00000000";
    signal data_command_in  : std_logic                    := '0';
    signal data_command_out : std_logic;

    signal mosi : std_logic;
    signal sck  : std_logic;
    signal cs   : std_logic;

    -- Debug signals (screen_controller)
    signal seq_counter_dbg           : std_logic_vector(9 downto 0); 
    signal start_signal_dbg          : std_logic;
    signal ready_signal_dbg          : std_logic;
    signal data_signal_dbg           : std_logic_vector(7 downto 0);
    signal data_command_internal_dbg : std_logic;

    signal expired_counter_5us_dbg   : std_logic;
    signal expired_counter_20ms_dbg  : std_logic;
    signal expired_counter_25ms_dbg  : std_logic;
    signal expired_counter_100ms_dbg : std_logic;
    signal expired_counter_400ms_dbg : std_logic;
    signal expired_counter_spi_dbg   : std_logic;
    
    -- Signals (screen_tester)
    signal enable        : std_logic := '0';

    -- Debug signals (screen_tester)
    signal enable_delay_dbg              : std_logic := '0';
    signal enable_rising_edge_dbg        : std_logic := '0';
    signal enable_falling_edge_dbg       : std_logic := '0';
    signal seq_counter_dbg_screen_tester : std_logic_vector(9 downto 0) := (others => '0');
    signal send_on_flag_dbg              : std_logic := '0';
    signal send_off_flag_dbg             : std_logic := '0';
    signal command_sent_flag_dbg         : std_logic := '0';

    -- All ILA mark_debug
    attribute mark_debug of on_off : signal is "true";
    attribute mark_debug of power_reset : signal is "true";
    attribute mark_debug of vcc_enable : signal is "true";
    attribute mark_debug of pmod_enable : signal is "true";

    attribute mark_debug of on_off_status : signal is "true";
    attribute mark_debug of start : signal is "true";
    attribute mark_debug of ready : signal is "true";
    
    attribute mark_debug of data : signal is "true";
    attribute mark_debug of data_command_in : signal is "true";
    attribute mark_debug of data_command_out : signal is "true";
    
    attribute mark_debug of mosi : signal is "true";
    attribute mark_debug of sck : signal is "true";
    attribute mark_debug of cs : signal is "true";

    attribute mark_debug of seq_counter_dbg : signal is "true";
    attribute mark_debug of start_signal_dbg : signal is "true";
    attribute mark_debug of ready_signal_dbg : signal is "true";
    attribute mark_debug of data_signal_dbg : signal is "true";
    attribute mark_debug of data_command_internal_dbg : signal is "true";

    attribute mark_debug of expired_counter_5us_dbg : signal is "true";
    attribute mark_debug of expired_counter_20ms_dbg : signal is "true";
    attribute mark_debug of expired_counter_25ms_dbg : signal is "true";
    attribute mark_debug of expired_counter_100ms_dbg : signal is "true";
    attribute mark_debug of expired_counter_400ms_dbg : signal is "true";
    attribute mark_debug of expired_counter_spi_dbg : signal is "true";
    
begin

    -- Port Maping
    screen_controller_inst: screen_controller
        Port Map (
            -- Basic
            CLK   => CLK,
            RESET => RESET,

            -- Power 
            ON_OFF      => on_off,
            POWER_RESET => power_reset,
            VCC_ENABLE  => vcc_enable,
            PMOD_ENABLE => pmod_enable,

            -- Control
            ON_OFF_STATUS => on_off_status,
            START         => start, 
            READY         => ready, 

            -- Data
            DATA             => data,
            DATA_COMMAND_IN  => data_command_in,
            DATA_COMMAND_OUT => data_command_out,

            -- SPI
            MOSI => mosi,
            SCK  => sck,
            CS   => cs,

            -- Debug
            SEQ_COUNTER_DBG            => seq_counter_dbg,
            START_SIGNAL_DBG           => start_signal_dbg,
            READY_SIGNAL_DBG           => ready_signal_dbg,
            DATA_SIGNAL_DBG            => data_signal_dbg,
            DATA_COMMAND_INTERNAL_DBG  => data_command_internal_dbg,
            EXPIRED_COUNTER_5US_DBG    => expired_counter_5us_dbg,
            EXPIRED_COUNTER_20MS_DBG   => expired_counter_20ms_dbg,
            EXPIRED_COUNTER_25MS_DBG   => expired_counter_25ms_dbg,
            EXPIRED_COUNTER_100MS_DBG  => expired_counter_100ms_dbg,
            EXPIRED_COUNTER_400MS_DBG  => expired_counter_400ms_dbg,
            EXPIRED_COUNTER_SPI_DBG    => expired_counter_spi_dbg
        );
    
    screen_tester_inst: screen_tester
        Port Map (
            CLK   => CLK,
            RESET => RESET,

            -- Enable
            ENABLE => enable,

            -- Control
            ON_OFF_STATUS => on_off_status,
            START         => start,
            READY         => ready,

            -- Data
            DATA         => data,
            DATA_COMMAND => data_command_in,

            -- Debug
            ENABLE_DELAY_DBG        => enable_delay_dbg,
            ENABLE_RISING_EDGE_DBG  => enable_rising_edge_dbg,
            ENABLE_FALLING_EDGE_DBG => enable_falling_edge_dbg,
            SEQ_COUNTER_DBG         => seq_counter_dbg_screen_tester,
            SEND_ON_FLAG_DBG        => send_on_flag_dbg,
            SEND_OFF_FLAG_DBG       => send_off_flag_dbg,
            COMMAND_SENT_FLAG_DBG   => command_sent_flag_dbg
        );

    enable <= SW(1);
    on_off <= SW(0);
    led    <= ready & on_off_status;

    JA_0_CS     <= cs;
    JA_1_MOSI   <= mosi;
    JA_2_NC     <= '0';
    JA_3_SCK    <= sck;
    JA_4_DC     <= data_command_out;
    JA_5_RES    <= power_reset;
    JA_6_VCCEN  <= vcc_enable;
    JA_7_PMODEN <= pmod_enable;

end Behavioral;
