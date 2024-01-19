----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 08.01.2024 16:33:39
-- Design Name: 
-- Module Name: screen_controller - Behavioral
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
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity screen_controller is
    Port (
        -- Basic
        clk   : in std_logic;
        reset : in std_logic;

        -- Power 
        on_off      : in  std_logic;
        power_reset : out std_logic;
        vcc_enable  : out std_logic;
        pmod_enable : out std_logic;

        -- Control
        on_off_status : out std_logic_vector(1 downto 0);
        start         : in  std_logic;
        ready         : out std_logic;

        -- Data
        data             : in  std_logic_vector(7 downto 0);
        data_command_in  : in  std_logic;
        data_command_out : out std_logic;

        -- SPI
        mosi : out std_logic;
        sck  : out std_logic;
        cs   : out std_logic;

        -- Debug
        seq_counter_dbg           : out std_logic_vector(9 downto 0);
        start_signal_dbg          : out std_logic;
        ready_signal_dbg          : out std_logic;
        data_signal_dbg           : out std_logic_vector(7 downto 0);
        data_command_internal_dbg : out std_logic
    );
end screen_controller;

architecture Behavioral of screen_controller is

    -- Instantiate the component
    component spi_master is
        Port (
            -- Basic
            clk   : in std_logic;
            reset : in std_logic;

            -- Control
            start : in  std_logic;
            ready : out std_logic;

            -- SPI
            mosi : out std_logic;
            sck  : out std_logic;
            cs   : out std_logic;

            -- Data
            data : in std_logic_vector(7 downto 0);

            -- Debug 
            done_dbg                   : out std_logic;
            bit_counter_dbg            : out std_logic_vector(2 downto 0);
            shift_data_dbg             : out std_logic_vector(7 downto 0);
            start_delay_signal_dbg     : out std_logic;
            start_rising_edge_flag_dbg : out std_logic
        );
    end component;

    -- Debug signals
    signal dbg_signals : std_logic_vector (13 downto 0);

    -- FSM
    type state_t is (s_off, s_turning_on, s_on, s_turning_off);
    signal state : state_t := s_off;

    -- Commands
    constant UNLOCK_COMMAND                : std_logic_vector(7 downto 0) := x"FD";
    constant UNLOCK_DATA                   : std_logic_vector(7 downto 0) := x"12";
    constant DISPLAY_OFF_COMMAND           : std_logic_vector(7 downto 0) := x"AE";
    constant DISPLAY_ON_COMMAND            : std_logic_vector(7 downto 0) := x"AF";
    constant CLEAR_WINDOW_COMMAND          : std_logic_vector(7 downto 0) := x"25";    
    constant REMAP_COMMAND                 : std_logic_vector(7 downto 0) := x"A0";  
    constant DISPLAY_START_LINE_COMMAND    : std_logic_vector(7 downto 0) := x"A1"; 
    constant DISPLAY_OFFSET_COMMAND        : std_logic_vector(7 downto 0) := x"A2"; 
    constant NORMAL_DISPLAY_COMMAND        : std_logic_vector(7 downto 0) := x"A4"; 
    constant ENTIRE_DISPLAY_ON_COMMAND     : std_logic_vector(7 downto 0) := x"A5";
    constant ENTIRE_DISPLAY_OFF_COMMAND    : std_logic_vector(7 downto 0) := x"A6";
    constant INVERSE_DISPLAY_COMMAND       : std_logic_vector(7 downto 0) := x"A7";
    constant MUX_RATIO_COMMAND             : std_logic_vector(7 downto 0) := x"A8";
    constant MASTER_CONFIG_COMMAND         : std_logic_vector(7 downto 0) := x"AD";
    constant EXT_VCC_SUPPLY_DATA           : std_logic_vector(7 downto 0) := x"8E";
    constant POWER_SAVE_MODE_COMMAND       : std_logic_vector(7 downto 0) := x"B0";
    constant DISABLE_POWER_SAVE_MODE_DATA  : std_logic_vector(7 downto 0) := x"0B";
    constant ENABLE_POWER_SAVE_MODE_DATA   : std_logic_vector(7 downto 0) := x"1A";
    constant PHASE_LENGTH_COMMAND          : std_logic_vector(7 downto 0) := x"B1";
    constant DISP_CLK_DIV_OSC_FREQ_COMMAND : std_logic_vector(7 downto 0) := x"B3";
    constant SECOND_PRECHARGE_A_COMMAND    : std_logic_vector(7 downto 0) := x"8A";
    constant SECOND_PRECHARGE_B_COMMAND    : std_logic_vector(7 downto 0) := x"8B";
    constant SECOND_PRECHARGE_C_COMMAND    : std_logic_vector(7 downto 0) := x"8C";
    constant PRECHARGE_COMMAND             : std_logic_vector(7 downto 0) := x"BB";
    constant VCOMH_COMMAND                 : std_logic_vector(7 downto 0) := x"BE";
    constant MASTER_CURRENT_COMMAND        : std_logic_vector(7 downto 0) := x"87";
    constant CONTRAST_A_COMMAND            : std_logic_vector(7 downto 0) := x"81";
    constant CONTRAST_B_COMMAND            : std_logic_vector(7 downto 0) := x"82";
    constant CONTRAST_C_COMMAND            : std_logic_vector(7 downto 0) := x"83";
    constant DISABLE_SCROLL_COMMAND        : std_logic_vector(7 downto 0) := x"2E";


    constant MIN_COLUMN                 : std_logic_vector(7 downto 0) := x"00";
    constant MAX_COLUMN                 : std_logic_vector(7 downto 0) := x"5F";
    constant MIN_ROW                    : std_logic_vector(7 downto 0) := x"00";
    constant MAX_ROW                    : std_logic_vector(7 downto 0) := x"3F";
    
    constant DRAW_LINE_COMMAND          : std_logic_vector(7 downto 0) := x"21";

    -- Data/Command
    constant DATA_TYPE    : std_logic := '1';
    constant COMMAND_TYPE : std_logic := '0';

    -- Signals
    signal start_signal, start_internal, ready_signal : std_logic := '0';

    signal data_signal   : std_logic_vector(7 downto 0) := "00000000";
    signal data_internal : std_logic_vector(7 downto 0) := "00000000";

    signal data_command_internal : std_logic := '0';

    signal transition_completed : std_logic := '0';

    signal seq_counter : unsigned(9 downto 0) := (others => '0');

    -- Counter signals
    signal enable_counter_5us    : std_logic                            := '0';
    signal enable_counter_20ms   : std_logic                            := '0';
    signal enable_counter_25ms   : std_logic                            := '0';
    signal enable_counter_100ms  : std_logic                            := '0';
    signal enable_counter_400ms  : std_logic                            := '0';
    signal enable_counter_spi    : std_logic                            := '0';
    constant max_counter_5us     : integer                              := 5;      -- 5      -- 10 for simulation
    constant max_counter_20ms    : integer                              := 20000;  -- 20000  -- 20 for simulation
    constant max_counter_25ms    : integer                              := 25000;  -- 25000  -- 25 for simulation
    constant max_counter_100ms   : integer                              := 100000; -- 100000 -- 30 for simulation
    constant max_counter_400ms   : integer                              := 400000; -- 400000 -- 40 for simulation
    constant max_counter_spi     : integer                              := 20;  -- Wait 20 clock cycles until trying to send new spi 
    signal counter_5us           : integer range 0 to max_counter_5us   := 0;
    signal counter_20ms          : integer range 0 to max_counter_20ms  := 0;
    signal counter_25ms          : integer range 0 to max_counter_25ms  := 0;
    signal counter_100ms         : integer range 0 to max_counter_100ms := 0;
    signal counter_400ms         : integer range 0 to max_counter_100ms := 0;
    signal counter_spi           : integer range 0 to max_counter_spi   := 0;
    signal expired_counter_5us   : std_logic                            := '0';
    signal expired_counter_20ms  : std_logic                            := '0';
    signal expired_counter_25ms  : std_logic                            := '0';
    signal expired_counter_100ms : std_logic                            := '0';
    signal expired_counter_400ms : std_logic                            := '0';
    signal expired_counter_spi   : std_logic                            := '0';

begin

    -------------------- Port Mapping --------------------
    spi_master_inst : spi_master
        Port Map (
            -- Basic
            clk   => clk,
            reset => reset,

            -- Control
            start => start_signal,
            ready => ready_signal,

            -- SPI
            mosi => mosi,
            sck  => sck,
            cs   => cs,

            -- Data
            data => data_signal,

            -- Debug 
            done_dbg                   => dbg_signals(0),
            bit_counter_dbg            => dbg_signals(3 downto 1),
            shift_data_dbg             => dbg_signals(11 downto 4),
            start_delay_signal_dbg     => dbg_signals(12),
            start_rising_edge_flag_dbg => dbg_signals(13)
        );

    -------------------- Processes --------------------

    FSM_proc : process(clk, reset)
    begin
        if (reset = '1') then
            state <= s_off;
        elsif (rising_edge(clk)) then
            case state is
                when s_off =>
                    if (on_off = '1') then
                        state <= s_turning_on;
                    end if;
                when s_turning_on =>
                    if (transition_completed = '1' and on_off = '1') then
                        state <= s_on;
                    end if;
                when s_on =>
                    if (on_off = '0') then
                        state <= s_turning_off;
                    end if;
                when s_turning_off =>
                    if (transition_completed = '1' and on_off = '0') then
                        state <= s_off;
                    end if;
            end case;
        end if;
    end process;

    on_off_status <= "00" when (state = s_off) else
                     "01" when (state = s_turning_on) else
                     "10" when (state = s_turning_off) else
                     "11";

    on_off_proc : process(clk, reset)
    begin
        if (reset = '1') then
            power_reset           <= '1';
            vcc_enable            <= '0';
            pmod_enable           <= '0';
            data_internal         <= "00000000";
            start_internal        <= '0';
            data_command_internal <= '0';
            seq_counter           <= (others => '0');
            transition_completed  <= '0';

        elsif (rising_edge(clk)) then

            case state is

                when s_off =>
                    power_reset           <= '1';
                    vcc_enable            <= '0';
                    pmod_enable           <= '0';
                    data_internal         <= "00000000";
                    start_internal        <= '0';
                    data_command_internal <= '0';
                    seq_counter           <= (others => '0');
                    transition_completed  <= '0';

                when s_turning_on =>
                    if (seq_counter = 0) then
                        pmod_enable         <= '1';
                        enable_counter_20ms <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 1 and expired_counter_20ms = '1') then
                        enable_counter_20ms <= '0';
                        power_reset         <= '0';
                        enable_counter_5us  <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 2 and expired_counter_5us = '1') then
                        enable_counter_5us  <= '0';
                        power_reset         <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 3) then
                        enable_counter_5us  <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 4 and expired_counter_5us = '1') then
                        enable_counter_5us    <= '0';
                        data_internal         <= UNLOCK_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 5 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 6 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= UNLOCK_DATA;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 7 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 8 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= DISPLAY_OFF_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 9 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 10 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= REMAP_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 11 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 12 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"72";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 13 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 14 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= DISPLAY_START_LINE_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 15 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 16 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MIN_ROW;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 17 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 18 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= DISPLAY_OFFSET_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 19 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 20 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"00";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 21 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 22 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= NORMAL_DISPLAY_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 23 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 24 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MUX_RATIO_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 25 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 26 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"3F";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 27 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 28 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MASTER_CONFIG_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 29 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 30 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= EXT_VCC_SUPPLY_DATA;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 31 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 32 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= POWER_SAVE_MODE_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 33 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 34 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= DISABLE_POWER_SAVE_MODE_DATA;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 35 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 36 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= PHASE_LENGTH_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 37 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 38 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"31";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 39 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 40 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= DISP_CLK_DIV_OSC_FREQ_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 41 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 42 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"F0";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 43 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 44 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= SECOND_PRECHARGE_A_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 45 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 46 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"64";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 47 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 48 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= SECOND_PRECHARGE_B_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 49 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 50 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"78";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 51 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 52 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= SECOND_PRECHARGE_C_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 53 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 54 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"64";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 55 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 56 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= PRECHARGE_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 57 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 58 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"3A";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 59 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 60 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= VCOMH_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 61 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 62 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"3E";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 63 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 64 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MASTER_CURRENT_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 65 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 66 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"06";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 67 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 68 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= CONTRAST_A_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 69 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 70 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"91";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 71 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 72 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= CONTRAST_B_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 73 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 74 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"50";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 75 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 76 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= CONTRAST_C_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 77 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 78 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"7D";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 79 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 80 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= DISABLE_SCROLL_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 81 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 82 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= CLEAR_WINDOW_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 83 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 84 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MIN_COLUMN;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 85 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 86 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MIN_ROW;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 87 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 88 and expired_counter_spi = '1'  and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MAX_COLUMN;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 89 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 90 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MAX_ROW;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 91 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 92 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi  <= '0';
                        vcc_enable          <= '1';
                        enable_counter_25ms <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 93 and expired_counter_25ms = '1') then
                        enable_counter_25ms   <= '0';
                        data_internal         <= DISPLAY_ON_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 94 and start_internal = '1') then
                        start_internal      <= '0';
                        enable_counter_spi  <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 95 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi   <= '0';
                        enable_counter_100ms <= '1';
                        seq_counter          <= seq_counter + 1;
                    elsif (seq_counter = 96 and expired_counter_100ms = '1') then
                        enable_counter_100ms <= '0';
                        seq_counter          <= seq_counter + 1;
                        -- transition_completed <= '1'; -- Add some instructions to check if it works
                    elsif (seq_counter = 97 and ready_signal = '1') then
                        data_internal         <= ENTIRE_DISPLAY_ON_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 98 and start_internal = '1') then
                        start_internal      <= '0';
                        enable_counter_spi  <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 99 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi   <= '0';
                        transition_completed <= '1';
                    end if;

                when s_on =>
                    power_reset           <= '1';
                    vcc_enable            <= '1';
                    pmod_enable           <= '1';
                    data_internal         <= "00000000";
                    start_internal        <= '0';
                    data_command_internal <= '0';
                    seq_counter           <= (others => '0');
                    transition_completed  <= '0';

                when s_turning_off =>
                    if (seq_counter = 0) then
                        data_internal         <= DISPLAY_OFF_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 1 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 2 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi   <= '0';
                        vcc_enable           <= '0';
                        enable_counter_400ms <= '1';
                        seq_counter          <= seq_counter + 1;
                    elsif (seq_counter = 3 and expired_counter_400ms = '1') then
                        enable_counter_400ms <= '0';
                        transition_completed <= '1';
                    end if;
            end case;
        end if;
    end process;

    data_signal      <= data            when (state = s_on) else data_internal;
    data_command_out <= data_command_in when (state = s_on) else data_command_internal;
    start_signal     <= start           when (state = s_on) else start_internal;
    ready            <= ready_signal    when (state = s_on) else '0';


    -------------------- Counters --------------------

    timer_5us_proc : process(clk, reset)
    begin
        if (reset = '1') then
            counter_5us <= 0;
        elsif (rising_edge(clk)) then
            if (enable_counter_5us = '1') then
                if (counter_5us < max_counter_5us) then
                    counter_5us <= counter_5us + 1;
                end if;
            else
                counter_5us <= 0;
            end if;
        end if;
    end process;

    expired_counter_5us <= '1' when (counter_5us = max_counter_5us) else '0';

    timer_20ms_proc : process(clk, reset)
    begin
        if (reset = '1') then
            counter_20ms <= 0;
        elsif (rising_edge(clk)) then
            if (enable_counter_20ms = '1') then
                if (counter_20ms < max_counter_20ms) then
                    counter_20ms <= counter_20ms + 1;
                end if;
            else
                counter_20ms <= 0;
            end if;
        end if;
    end process;

    expired_counter_20ms <= '1' when (counter_20ms = max_counter_20ms) else '0';

    timer_25ms_proc : process(clk, reset)
    begin
        if (reset = '1') then
            counter_25ms <= 0;
        elsif (rising_edge(clk)) then
            if (enable_counter_25ms = '1') then
                if (counter_25ms < max_counter_25ms) then
                    counter_25ms <= counter_25ms + 1;
                end if;
            else
                counter_25ms <= 0;
            end if;
        end if;
    end process;

    expired_counter_25ms <= '1' when (counter_25ms = max_counter_25ms) else '0';

    timer_100ms_proc : process(clk, reset)
    begin
        if (reset = '1') then
            counter_100ms <= 0;
        elsif (rising_edge(clk)) then
            if (enable_counter_100ms = '1') then
                if (counter_100ms < max_counter_100ms) then
                    counter_100ms <= counter_100ms + 1;
                end if;
            else
                counter_100ms <= 0;
            end if;
        end if;
    end process;

    expired_counter_100ms <= '1' when (counter_100ms = max_counter_100ms) else '0';

    timer_400ms_proc : process(clk, reset)
    begin
        if (reset = '1') then
            counter_400ms <= 0;
        elsif (rising_edge(clk)) then
            if (enable_counter_400ms = '1') then
                if (counter_400ms < max_counter_400ms) then
                    counter_400ms <= counter_400ms + 1;
                end if;
            else
                counter_400ms <= 0;
            end if;
        end if;
    end process;

    expired_counter_400ms <= '1' when (counter_400ms = max_counter_400ms) else '0';

    timer_spi_proc : process(clk, reset)
    begin
        if (reset = '1') then
            counter_spi <= 0;
        elsif (rising_edge(clk)) then
            if (enable_counter_spi = '1') then
                if (counter_spi < max_counter_spi) then
                    counter_spi <= counter_spi + 1;
                end if;
            else
                counter_spi <= 0;
            end if;
        end if;
    end process;

    expired_counter_spi <= '1' when (counter_spi = max_counter_spi) else '0';

    -------------------- Debug --------------------
    seq_counter_dbg           <= std_logic_vector(seq_counter);
    start_signal_dbg          <= start_signal;
    ready_signal_dbg          <= ready_signal;
    data_signal_dbg           <= data_signal;
    data_command_internal_dbg <= data_command_internal;

end Behavioral;