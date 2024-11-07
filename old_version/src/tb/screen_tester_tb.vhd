library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity screen_tester_tb is
--  Port ( );
end screen_tester_tb;

architecture Behavioral of screen_tester_tb is

    -- Instantiate the COmponent Under Test
    component screen_tester is
        Port ( 
            -- Basic
            CLK    : in std_logic;
            RESETN  : in std_logic;

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

    -- Clock
    constant clk_period : time := 8 ns;


    -- Signals
    signal clk, resetn   : std_logic := '0';
    signal enable        : std_logic := '0';
    signal on_off_status : std_logic_vector(1 downto 0) := "00";
    signal start         : std_logic := '0';
    signal ready         : std_logic := '0';
    signal data          : std_logic_vector(7 downto 0) := "00000000";
    signal data_command  : std_logic := '0';
    
    signal enable_delay_dbg        : std_logic := '0';
    signal enable_rising_edge_dbg  : std_logic := '0';
    signal enable_falling_edge_dbg : std_logic := '0';
    signal seq_counter_dbg         : std_logic_vector(9 downto 0) := (others => '0');
    signal send_on_flag_dbg        : std_logic := '0';
    signal send_off_flag_dbg       : std_logic := '0';
    signal command_sent_flag_dbg   : std_logic := '0';

begin

    -- Port Map
    
    screen_tester_inst: screen_tester
        Port Map (
            CLK    => clk,
            RESETN => resetn,

            -- Enable
            ENABLE => enable,

            -- Control
            ON_OFF_STATUS => on_off_status,
            START         => start,
            READY         => ready,

            -- Data
            DATA         => data,
            DATA_COMMAND => data_command,

            -- Debug
            ENABLE_DELAY_DBG        => enable_delay_dbg,
            ENABLE_RISING_EDGE_DBG  => enable_rising_edge_dbg,
            ENABLE_FALLING_EDGE_DBG => enable_falling_edge_dbg,
            SEQ_COUNTER_DBG         => seq_counter_dbg,
            SEND_ON_FLAG_DBG        => send_on_flag_dbg,
            SEND_OFF_FLAG_DBG       => send_off_flag_dbg,
            COMMAND_SENT_FLAG_DBG   => command_sent_flag_dbg
        );

    -- Stimulus processes

    clk_proc : process
    begin
        clk <= '0';
        wait for clk_period/2;
        clk <= '1';
        wait for clk_period/2;
    end process;

    stim_proc: process
    begin
        on_off_status <= "11";
        ready         <= '1';
        resetn        <= '0';
            wait for 5*clk_period;
        resetn <= '1';
        enable <= '1';
            wait for 50*clk_period;
        enable <= '0';
            wait for 100*clk_period;
    end process;

end Behavioral;
