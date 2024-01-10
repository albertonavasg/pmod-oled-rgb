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
          clk   : in std_logic;
          reset : in std_logic;
          
          -- Control
          start : in  std_logic;
          ready : out std_logic;
          
          -- SPI
          mosi         : out std_logic;
          sck          : out std_logic;
          cs           : out std_logic;
          
          -- Data
          data : in std_logic_vector(7 downto 0);
          
          -- Debug 
          done_dbg        : out std_logic;
          bit_counter_dbg : out std_logic_vector(2 downto 0);
          shift_data_dbg  : out std_logic_vector(7 downto 0);
          start_delay_signal_dbg : out std_logic;
          start_rising_edge_flag_dbg : out std_logic
        );
end spi_master;

architecture Behavioral of spi_master is
    
    -- Signals
    signal start_delay_signal : std_logic := '0';
    signal start_rising_edge_flag : std_logic := '0';
    signal sck_signal : std_logic := '0';
    signal done_signal: std_logic := '0';
    signal bit_counter_signal : unsigned (2 downto 0) := "000";
    signal shift_data_signal : std_logic_vector(7 downto 0) := "00000000";
    
    -- FSM
    type state_t is (s_ready, s_busy);
    signal state : state_t := s_ready;
    
begin
    
    start_rise_proc: process(clk, reset)
    begin
        if (reset = '1') then
            start_delay_signal     <= '0';
            start_rising_edge_flag <= '0';
        elsif (rising_edge(clk)) then
            if (start_delay_signal = '0' and start = '1') then
                start_rising_edge_flag <= '1';
            else
                start_rising_edge_flag <= '0';
            end if;
            start_delay_signal <= start;
        end if;
    end process;
    
   
    FSM_proc: process(clk, reset)
    begin
        if (reset = '1') then
            state <= s_ready;
        elsif (rising_edge(clk)) then
            case state is
                when s_ready =>
                    if (start_rising_edge_flag = '1') then
                        state <= s_busy;
                    end if;
                when s_busy =>
                    if (done_signal = '1') then
                        state <= s_ready;
                    end if;
            end case;
        end if;
    end process;
    
    ready <= '1' when (state = s_ready) else '0';
    
    tx_proc: process(clk, reset)
    begin
        if (reset = '1') then
            sck_signal         <= '1';
            bit_counter_signal <= "000";
            shift_data_signal  <= "00000000";
            done_signal        <= '0';
        elsif (rising_edge(clk)) then
            if (state = s_busy) then
                if (sck_signal = '1') then
                    sck_signal <= '0';
                    if (bit_counter_signal = 0) then
                        shift_data_signal <= data;
                    elsif (done_signal = '0') then
                        shift_data_signal <= shift_data_signal(6 downto 0) & '0';
                    else
                        shift_data_signal  <= "00000000";
                        bit_counter_signal <= "000";
                        done_signal        <= '0';
                    end if;
                else
                    sck_signal <= '1';
                    if (bit_counter_signal < 7) then
                        bit_counter_signal <= bit_counter_signal + 1;
                    else
                        done_signal <= '1';
                    end if;
                end if;
             else
                sck_signal         <= '1';
                bit_counter_signal <= "000";
                shift_data_signal  <= "00000000";
                done_signal        <= '0';
            end if;
        end if;
    end process;
    
    mosi <= shift_data_signal(7) when (state = s_busy) else '0';
    cs   <= '0' when (state = s_busy) else '1';
    sck  <= sck_signal when (state = s_busy) else '1';
    
    -- Debug signals
    done_dbg        <= done_signal;
    bit_counter_dbg <= std_logic_vector(bit_counter_signal);
    shift_data_dbg  <= shift_data_signal;
    
end Behavioral;
