----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 21.12.2023 17:22:47
-- Design Name: 
-- Module Name: spi_master - Behavioral
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
        STATE_DBG                       : out std_logic_vector(1 downto 0);
        DONE_DBG                        : out std_logic;
        BIT_COUNTER_DBG                 : out std_logic_vector(2 downto 0);
        SHIFT_DATA_DBG                  : out std_logic_vector(7 downto 0);
        START_DELAY_DBG                 : out std_logic;
        START_RISING_EDGE_DBG           : out std_logic;
        CLK_1_MHZ_DBG                   : out std_logic;
        CLK_1MHZ_DELAY_DBG              : out std_logic;
        CLK_1MHZ_RISING_EDGE_DBG        : out std_logic;
        CLK_1MHZ_FALLING_EDGE_DBG       : out std_logic;
        FIRST_CLK_1MHZ_FALLING_EDGE_DBG : out std_logic
    );
end spi_master;

architecture Behavioral of spi_master is

    -- Signals
    signal start_delay                 : std_logic             := '0';
    signal start_rising_edge           : std_logic             := '0';
    signal clk_1mhz_delay              : std_logic             := '0';
    signal clk_1mhz_rising_edge        : std_logic             := '0';
    signal clk_1mhz_falling_edge       : std_logic             := '0';
    signal first_clk_1mhz_falling_edge : std_logic             := '0';
    signal done                        : std_logic             := '0';
    signal bit_counter                 : unsigned (2 downto 0) := "000";
    signal shift_data                  : std_logic_vector(7 downto 0) := "00000000";

    constant max_counter : integer := 125; -- From 125MHz to 1 MHz
    signal counter       : integer range 0 to max_counter - 1 := 0;
    signal clk_1mhz     : std_logic := '0';

    -- FSM
    type state_t is (s_idle, s_pending, s_tx);
    signal state : state_t := s_idle;

begin

    edge_detect_proc : process(CLK, RESET)
    begin
        if (RESET = '1') then
            start_delay           <= '0';
            start_rising_edge     <= '0';
            clk_1mhz_delay        <= '0';
            clk_1mhz_rising_edge  <= '0';
            clk_1mhz_falling_edge <= '0';
        elsif (rising_edge(CLK)) then
            start_delay    <= start;
            clk_1mhz_delay <= clk_1mhz;
            if (start_delay = '0' and start = '1') then
                start_rising_edge <= '1';
            else
                start_rising_edge <= '0';
            end if;
            if (clk_1mhz_delay = '0' and clk_1mhz = '1') then
                clk_1mhz_rising_edge <= '1';
            else
                clk_1mhz_rising_edge <= '0';
            end if;
            if (clk_1mhz_delay = '1' and clk_1mhz = '0') then
                clk_1mhz_falling_edge <= '1';
            else
                clk_1mhz_falling_edge <= '0';
            end if;
        end if;
    end process;

    READY <= '1' when (state = s_idle) else '0';

    STATE_DBG <= "00" when (state = s_idle) else
                 "01" when (state = s_pending) else
                 "10" when (state = s_tx);

    FSM_proc: process(CLK, RESET)
    begin
        if (RESET = '1') then
            state <= s_idle;
        elsif (rising_edge(CLK)) then
            case state is
                when s_idle =>
                    if (start_rising_edge = '1') then
                        state <= s_pending;
                    end if;
                when s_pending =>
                    if (clk_1mhz = '1') then
                        state <= s_tx;
                    end if;
                when s_tx =>
                    if (done = '1') then
                        state <= s_idle;
                    end if;
            end case;
        end if;
    end process;


    tx_proc : process(CLK, RESET)
    begin
        if (RESET = '1') then
            bit_counter                 <= "000";
            shift_data                  <= "00000000";
            first_clk_1mhz_falling_edge <= '0';
            done                        <= '0';
        elsif (rising_edge(CLK)) then
            if (state = s_tx) then
                if(clk_1mhz_falling_edge = '1') then
                    if (first_clk_1mhz_falling_edge = '0') then
                        first_clk_1mhz_falling_edge <= '1';
                    end if;
                    if (bit_counter = 0) then
                        shift_data <= DATA;
                    else
                        shift_data <= shift_data(6 downto 0) & '0';
                    end if;
                end if;
                if (clk_1mhz_rising_edge = '1' and first_clk_1mhz_falling_edge = '1') then
                    if (bit_counter < 7) then
                        bit_counter <= bit_counter + 1;
                    elsif (bit_counter = 7) then
                        done <= '1';
                    end if;
                end if;
            else
                bit_counter                 <= "000";
                shift_data                  <= "00000000";
                first_clk_1mhz_falling_edge <= '0';
                done                        <= '0';
            end if;
        end if;
    end process;

    MOSI <= shift_data(7) when (state = s_tx) else '0';
    CS   <= '0'           when (state = s_tx) else '1';
    SCK  <= clk_1mhz      when (state = s_tx) else '1';

    
    counter_1MHz_proc: process(CLK, RESET)
    begin
        if (RESET = '1') then
            counter <= 0;
        elsif (rising_edge(CLK)) then
                if (counter < max_counter -1) then
                    counter <= counter + 1;
                else
                    counter <= 0;
                end if;
        end if;
    end process;

    clk_1mhz <= '1' when (counter >= max_counter/2) else '0';

    -- Debug signals
    DONE_DBG                        <= done;
    BIT_COUNTER_DBG                 <= std_logic_vector(bit_counter);
    SHIFT_DATA_DBG                  <= shift_data;
    START_DELAY_DBG                 <= start_delay;
    START_RISING_EDGE_DBG           <= start_rising_edge;
    CLK_1_MHZ_DBG                   <= clk_1mhz;
    CLK_1MHZ_DELAY_DBG              <= clk_1mhz_delay;
    CLK_1MHZ_RISING_EDGE_DBG        <= clk_1mhz_rising_edge;
    CLK_1MHZ_FALLING_EDGE_DBG       <= clk_1mhz_falling_edge; 
    FIRST_CLK_1MHZ_FALLING_EDGE_DBG <= first_clk_1mhz_falling_edge;
end Behavioral;
