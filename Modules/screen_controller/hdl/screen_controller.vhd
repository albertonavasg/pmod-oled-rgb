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
    constant UNLOCK_COMMAND       : std_logic_vector(7 downto 0) := x"FD";
    constant UNLOCK_DATA          : std_logic_vector(7 downto 0) := x"12";
    constant DISPLAY_OFF_COMMAND  : std_logic_vector(7 downto 0) := x"AE";
    constant DISPLAY_ON_COMMAND   : std_logic_vector(7 downto 0) := x"AF";
    constant CLEAR_WINDOW_COMMAND : std_logic_vector(7 downto 0) := x"25";
    constant MIN_COLUMN           : std_logic_vector(7 downto 0) := x"00";
    constant MAX_COLUMN           : std_logic_vector(7 downto 0) := x"5F";
    constant MIN_ROW              : std_logic_vector(7 downto 0) := x"00";
    constant MAX_ROW              : std_logic_vector(7 downto 0) := x"3F";
    constant DRAW_LINE_COMMAND    : std_logic_vector(7 downto 0) := x"21";

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
    constant max_counter_5us     : integer                              := 5; -- 5      -- 10 for simulation
    constant max_counter_20ms    : integer                              := 5; -- 20000  -- 20 for simulation
    constant max_counter_25ms    : integer                              := 5; -- 25000  -- 25 for simulation
    constant max_counter_100ms   : integer                              := 5; -- 100000 -- 30 for simulation
    constant max_counter_400ms   : integer                              := 5; -- 400000 -- 30 for simulation
    constant max_counter_spi     : integer                              := 3;  -- Wait 3 clock cycles until trying to send new spi 
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
                    if (transition_completed = '1') then
                        state <= s_on;
                    end if;
                when s_on =>
                    if (on_off = '0') then
                        state <= s_turning_off;
                    end if;
                when s_turning_off =>
                    if (transition_completed = '1') then
                        state <= s_off;
                    end if;
            end case;
        end if;
    end process;

    on_off_status <= "00" when (state = s_off) else
                     "01" when (state = s_turning_on) else
                     "10" when (state = s_turning_off) else
                     "11" when (state = s_on);

    turning_on_proc : process(clk, reset)
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
                        power_reset <= '1';
                        seq_counter <= seq_counter + 1;
                    elsif (seq_counter = 3 and expired_counter_5us = '1') then
                        enable_counter_5us    <= '0';
                        data_internal         <= UNLOCK_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 4 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 5 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= UNLOCK_DATA;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 6 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 7 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= DISPLAY_OFF_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 8 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    --------------------------------------------------
                    -- Add all instructions to put default settings --
                    --------------------------------------------------
                    elsif (seq_counter = 9 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= CLEAR_WINDOW_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 10 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 11 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MIN_COLUMN;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 12 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 13 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MIN_ROW;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 14 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 15 and expired_counter_spi = '1'  and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MAX_COLUMN;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 16 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 17 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= MAX_ROW;
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 18 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 19 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi  <= '0';
                        vcc_enable          <= '1';
                        enable_counter_25ms <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 20 and expired_counter_25ms = '1') then
                        enable_counter_25ms   <= '0';
                        data_internal         <= DISPLAY_ON_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 21 and start_internal = '1') then
                        start_internal      <= '0';
                        enable_counter_spi  <= '1';
                        seq_counter         <= seq_counter + 1;
                    elsif (seq_counter = 22 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi   <= '0';
                        enable_counter_100ms <= '1';
                        seq_counter          <= seq_counter + 1;
                    elsif (seq_counter = 23 and expired_counter_100ms = '1') then
                        enable_counter_100ms <= '0';
-------------------------------------------------------------------------------------------------------------------
                        -- transition_completed <= '1'; -- Add some instructions to check if it works
                        seq_counter <= seq_counter + 1;
                    elsif (seq_counter = 24) then
                        data_internal         <= DRAW_LINE_COMMAND;
                        data_command_internal <= COMMAND_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 25 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 26 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi <= '0';
                        data_internal         <= x"01";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 27 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 28 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"10";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 29 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 30 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"28";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 31 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 32 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi <= '0';
                        data_internal         <= x"04";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 33 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 34 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi <= '0';
                        data_internal         <= x"35";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 35 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 36 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"00";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 37 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 38 and expired_counter_spi = '1' and ready_signal = '1') then
                        enable_counter_spi    <= '0';
                        data_internal         <= x"00";
                        data_command_internal <= DATA_TYPE;
                        start_internal        <= '1';
                        seq_counter           <= seq_counter + 1;
                    elsif (seq_counter = 39 and start_internal = '1') then
                        start_internal     <= '0';
                        enable_counter_spi <= '1';
                        seq_counter        <= seq_counter + 1;
                    elsif (seq_counter = 40 and expired_counter_spi = '1' and ready_signal = '1') then
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