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
        DONE_DBG              : out std_logic;
        BIT_COUNTER_DBG       : out std_logic_vector(2 downto 0);
        SHIFT_DATA_DBG        : out std_logic_vector(7 downto 0);
        START_DELAY_DBG       : out std_logic;
        START_RISING_EDGE_DBG : out std_logic
    );
end spi_master;

architecture Behavioral of spi_master is

    -- Signals
    signal start_delay        : std_logic             := '0';
    signal start_rising_edge  : std_logic             := '0';
    signal sck_internal       : std_logic             := '1';
    signal done               : std_logic             := '0';
    signal bit_counter        : unsigned (2 downto 0) := "000";
    signal shift_data  : std_logic_vector(7 downto 0) := "00000000";

    -- FSM
    type state_t is (s_ready, s_busy);
    signal state : state_t := s_ready;

begin

    start_edge_proc : process(CLK, RESET)
    begin
        if (RESET = '1') then
            start_delay     <= '0';
            start_rising_edge <= '0';
        elsif (rising_edge(CLK)) then
            if (start_delay = '0' and start = '1') then
                start_rising_edge <= '1';
            else
                start_rising_edge <= '0';
            end if;
            start_delay <= start;
        end if;
    end process;


    FSM_proc : process(CLK, RESET)
    begin
        if (RESET = '1') then
            state <= s_ready;
        elsif (rising_edge(CLK)) then
            case state is
                when s_ready =>
                    if (start_rising_edge = '1') then
                        state <= s_busy;
                    end if;
                when s_busy =>
                    if (done = '1') then
                        state <= s_ready;
                    end if;
            end case;
        end if;
    end process;

    ready <= '1' when (state = s_ready) else '0';

    tx_proc : process(CLK, RESET)
    begin
        if (RESET = '1') then
            sck_internal         <= '1';
            bit_counter <= "000";
            shift_data  <= "00000000";
            done        <= '0';
        elsif (rising_edge(CLK)) then
            if (state = s_busy) then
                if (sck_internal = '1') then
                    sck_internal <= '0';
                    if (bit_counter = 0) then
                        shift_data <= data;
                    else
                        shift_data <= shift_data(6 downto 0) & '0';
                    end if;
                else
                    sck_internal <= '1';
                    if (bit_counter < 7) then
                        bit_counter <= bit_counter + 1;
                    else
                        done <= '1';
                    end if;
                end if;
            else
                sck_internal         <= '1';
                bit_counter <= "000";
                shift_data  <= "00000000";
                done        <= '0';
            end if;
        end if;
    end process;

    MOSI <= shift_data(7) when (state = s_busy) else '0';
    CS   <= '0'           when (state = s_busy) else '1';
    SCK  <= sck_internal    when (state = s_busy) else '1';

    -- Debug signals
    DONE_DBG              <= done;
    BIT_COUNTER_DBG       <= std_logic_vector(bit_counter);
    SHIFT_DATA_DBG        <= shift_data;
    START_DELAY_DBG       <= start_delay;
    START_RISING_EDGE_DBG <= start_rising_edge;

end Behavioral;
