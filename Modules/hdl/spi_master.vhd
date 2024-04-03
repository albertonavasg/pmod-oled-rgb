library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity spi_master is
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
end spi_master;

architecture Behavioral of spi_master is

    -- Signals
    signal start_delay                 : std_logic                    := '0';
    signal start_rising_edge           : std_logic                    := '0';
    signal clk_1mhz_delay              : std_logic                    := '0';
    signal clk_1mhz_rising_edge        : std_logic                    := '0';
    signal clk_1mhz_falling_edge       : std_logic                    := '0';

    signal done                        : std_logic                    := '0';
    signal bit_counter                 : unsigned (2 downto 0)        := "000";
    signal shift_data                  : std_logic_vector(7 downto 0) := "00000000";

    constant max_counter_1mhz  : integer := 125; -- From 125MHz to 1 MHz
    signal enable_counter_1mhz : std_logic := '0';
    signal counter_1mhz        : integer range 0 to max_counter_1mhz - 1 := 0;
    signal clk_1mhz            : std_logic := '0';

    constant max_timer_1us   : integer := 125; 
    signal enable_timer_1us  : std_logic := '0';
    signal timer_1us         : integer range 0 to max_timer_1us := 0;
    signal expired_timer_1us : std_logic := '0';

    -- FSM
    type state_t is (s_idle, s_tx, s_last);
    signal state : state_t := s_idle;

begin

    edge_detect_proc : process(CLK, RESET)
    begin
        if (RESET = '1') then
            start_delay           <= '0';
            start_rising_edge     <= '0';
            clk_1mhz_delay        <= '1';
            clk_1mhz_rising_edge  <= '0';
            clk_1mhz_falling_edge <= '0';
        elsif (rising_edge(CLK)) then
            -- Get delay signals
            start_delay    <= start;
            clk_1mhz_delay <= clk_1mhz;
            -- Rising edge in start
            if (start_delay = '0' and start = '1') then
                start_rising_edge <= '1';
            else
                start_rising_edge <= '0';
            end if;
            -- Rising edge in clk_1mhz
            if (clk_1mhz_delay = '0' and clk_1mhz = '1') then
                clk_1mhz_rising_edge <= '1';
            else
                clk_1mhz_rising_edge <= '0';
            end if;
            -- Falling edge in clk_1mhz
            if (clk_1mhz_delay = '1' and clk_1mhz = '0') then
                clk_1mhz_falling_edge <= '1';
            else
                clk_1mhz_falling_edge <= '0';
            end if;
        end if;
    end process;

    READY <= '1' when (state = s_idle) else '0';

    FSM_proc: process(CLK, RESET)
    begin
        if (RESET = '1') then
            state <= s_idle;
        elsif (rising_edge(CLK)) then
            case state is
                when s_idle =>
                    if (start_rising_edge = '1') then
                        state <= s_tx;
                        enable_counter_1mhz <= '1';
                    end if;
                when s_tx =>
                    if (done = '1') then
                        state <= s_last;
                        enable_counter_1mhz <= '0';
                        enable_timer_1us <= '1';
                    end if;
                when s_last => 
                    if (expired_timer_1us = '1') then
                        state <= s_idle;
                        enable_timer_1us <= '0';
                    end if;
            end case;
        end if;
    end process;


    tx_proc : process(CLK, RESET)
    begin
        if (RESET = '1') then
            bit_counter <= "000";
            shift_data  <= "00000000";
            done        <= '0';
        elsif (rising_edge(CLK)) then
            if (state = s_tx) then
                if(clk_1mhz_falling_edge = '1') then
                    if (bit_counter = 0) then
                        shift_data <= DATA;
                    else
                        shift_data <= shift_data(6 downto 0) & '0';
                    end if;
                elsif (clk_1mhz_rising_edge = '1') then
                    if (bit_counter < 7) then
                        bit_counter <= bit_counter + 1;
                    elsif (bit_counter = 7) then
                        done <= '1';
                    end if;
                end if;
            elsif (state = s_idle) then
                bit_counter <= "000";
                shift_data  <= "00000000";
                done        <= '0';
            end if;
        end if;
    end process;

    MOSI <= shift_data(7) when (state = s_tx or state = s_last) else '0';
    CS   <= '0'           when (state = s_tx or state = s_last) else '1';
    SCK  <= clk_1mhz      when (state = s_tx or state = s_last) else '1';

    
    counter_1MHz_proc: process(CLK, RESET)
    begin
        if (RESET = '1') then
            counter_1mhz <= 0;
        elsif (rising_edge(CLK)) then
                if (enable_counter_1mhz = '1') then
                    if (counter_1mhz < max_counter_1mhz -1) then
                        counter_1mhz <= counter_1mhz + 1;
                    else
                        counter_1mhz <= 0;
                    end if;
                else 
                    counter_1mhz <= 0;
                end if;
        end if;
    end process;

    clk_1mhz <= '0' when (counter_1mhz >= max_counter_1mhz/2) else '1'; -- clk stays on if disabled

    timer_1us_proc : process(CLK, RESET)
    begin
        if (RESET = '1') then
            timer_1us <= 0;
        elsif (rising_edge(CLK)) then
            if (enable_timer_1us = '1') then
                if (timer_1us < max_timer_1us) then
                    timer_1us <= timer_1us + 1;
                end if;
            else
                timer_1us <= 0;
            end if;
        end if;
    end process;

    expired_timer_1us <= '1' when (timer_1us = max_timer_1us) else '0';

    -- Debug signals
    DONE_DBG                  <= done;
    BIT_COUNTER_DBG           <= std_logic_vector(bit_counter);
    SHIFT_DATA_DBG            <= shift_data;
    START_DELAY_DBG           <= start_delay;
    START_RISING_EDGE_DBG     <= start_rising_edge;
    CLK_1_MHZ_DBG             <= clk_1mhz;
    CLK_1MHZ_DELAY_DBG        <= clk_1mhz_delay;
    CLK_1MHZ_RISING_EDGE_DBG  <= clk_1mhz_rising_edge;
    CLK_1MHZ_FALLING_EDGE_DBG <= clk_1mhz_falling_edge; 
    EXPIRED_TIMER_1US_DBG     <= expired_timer_1us;

end Behavioral;
