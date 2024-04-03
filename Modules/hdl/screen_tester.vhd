library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity screen_tester is
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
end screen_tester;

architecture Behavioral of screen_tester is

    signal enable_delay        : std_logic := '0';
    signal enable_rising_edge  : std_logic := '0';
    signal enable_falling_edge : std_logic := '0';

    signal seq_counter : unsigned(9 downto 0) := (others => '0');

    signal send_on_flag      : std_logic := '0';
    signal send_off_flag     : std_logic := '0';
    signal command_sent_flag : std_logic := '0';

begin

    enable_edge_detect_proc: process(CLK, RESET)
    begin
        if (RESET = '1') then
            enable_delay        <= '0';
            enable_rising_edge  <= '0';
            enable_falling_edge <= '0';
        elsif (rising_edge(CLK)) then
            -- Get delay signal
            enable_delay <= ENABLE;
            -- Rising edge in ENABLE
            if (enable_delay = '0' and ENABLE = '1') then
                enable_rising_edge <= '1';
            else
                enable_rising_edge <= '0';
            end if;
            -- Rising edge in ENABLE
            if (enable_delay = '1' and ENABLE = '0') then
                enable_falling_edge <= '1';
            else
                enable_falling_edge <= '0';
            end if;
        end if;
    end process;

    trigger_proc: process(CLK, RESET)
    begin
        if (RESET = '1') then
            send_on_flag  <= '0';
            send_off_flag <= '0';
        elsif (rising_edge(CLK)) then
            if (enable_rising_edge = '1' and ON_OFF_STATUS = "11" and READY = '1') then
                send_on_flag <= '1';
            end if;
            if (enable_falling_edge = '1' and ON_OFF_STATUS = "11" and READY = '1') then
                send_off_flag <= '1';
            end if;
            if (command_sent_flag = '1') then
                send_on_flag  <= '0';
                send_off_flag <= '0';
            end if;
        end if;
    end process;

    main_proc: process(CLK, RESET)
    begin
        if (RESET = '1') then
            DATA              <= (others => '0');
            DATA_COMMAND      <= '0';
            START             <= '0';
            seq_counter       <= (others => '0');
            command_sent_flag <= '0';
        elsif (rising_edge(CLK)) then
            if (send_on_flag = '1') then
                if (seq_counter = 0) then
                    DATA         <= x"A5";  -- ENTIRE_DISPLAY_ON_COMMAND
                    DATA_COMMAND <= '0';    -- COMMAND
                    START        <= '1';
                    seq_counter  <= seq_counter + 1;
                elsif (seq_counter = 1) then
                    START             <= '0';
                    command_sent_flag <= '1';
                end if;
            elsif (send_off_flag = '1') then
                if (seq_counter = 0) then
                    DATA         <= x"A6";  -- ENTIRE_DISPLAY_OFF_COMMAND
                    DATA_COMMAND <= '0';    -- COMMAND
                    START        <= '1';
                    seq_counter  <= seq_counter + 1;
                elsif (seq_counter = 1) then
                    START             <= '0';
                    command_sent_flag <= '1';
                end if;
            else
                START             <= '0';
                seq_counter       <= (others => '0');
                command_sent_flag <= '0';
            end if;
        end if;
    end process;

    ------------------- Debug -------------------
    ENABLE_DELAY_DBG        <= enable_delay;
    ENABLE_RISING_EDGE_DBG  <= enable_rising_edge;
    ENABLE_FALLING_EDGE_DBG <= enable_falling_edge;
    SEQ_COUNTER_DBG         <= std_logic_vector(seq_counter);
    SEND_ON_FLAG_DBG        <= send_on_flag;
    SEND_OFF_FLAG_DBG       <= send_off_flag;
    COMMAND_SENT_FLAG_DBG   <= command_sent_flag;

end Behavioral;
