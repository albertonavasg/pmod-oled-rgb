library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity screen_controller is
    Port (
        -- Sync
        CLK    : in std_logic;
        RESETN : in std_logic;

        -- Control
        ON_OFF        : in  std_logic;
        SPI_TRIGGER   : in  std_logic;

        -- Status
        ON_OFF_STATUS : out std_logic_vector(1 downto 0);
        SPI_READY     : out std_logic;

        -- Data input
        BYTE      : in  std_logic_vector(7 downto 0);
        DC_SELECT : in  std_logic;

        -- Pmod physical pins
        MOSI         : out std_logic;
        SCK          : out std_logic;
        CS           : out std_logic;
        DATA_COMMAND : out std_logic;
        POWER_RESET  : out std_logic;
        VCC_ENABLE   : out std_logic;
        PMOD_ENABLE  : out std_logic
    );
end screen_controller;

architecture Behavioral of screen_controller is

    component spi_master is
        Generic (   
            N              : positive  := 32;  -- 32bit serial word length is default
            CPOL           : std_logic := '0'; -- SPI mode selection (mode 0 default)
            CPHA           : std_logic := '0'; -- CPOL = clock polarity, CPHA = clock phase.
            PREFETCH       : positive  := 2;   -- prefetch lookahead cycles
            SPI_2X_CLK_DIV : positive  := 5    -- for a 100MHz sclk_i, yields a 10MHz SCK
        );
        Port (  
            sclk_i : in std_logic := 'X'; -- high-speed serial interface system clock
            pclk_i : in std_logic := 'X'; -- high-speed parallel interface system clock
            rst_i  : in std_logic := 'X'; -- reset core
            ---- serial interface ----
            spi_ssel_o : out std_logic;        -- spi bus slave select line
            spi_sck_o  : out std_logic;        -- spi bus sck
            spi_mosi_o : out std_logic;        -- spi bus mosi output
            spi_miso_i : in  std_logic := 'X'; -- spi bus spi_miso_i input
            ---- parallel interface ----
            di_req_o   : out std_logic;                                          -- preload lookahead data request line
            di_i       : in  std_logic_vector (N-1 downto 0) := (others => 'X'); -- parallel data in (clocked on rising spi_clk after last bit)
            wren_i     : in  std_logic := 'X';                                   -- user data write enable, starts transmission when interface is idle
            wr_ack_o   : out std_logic;                                          -- write acknowledge
            do_valid_o : out std_logic;                                          -- do_o data valid signal, valid during one spi_clk rising edge.
            do_o       : out std_logic_vector (N-1 downto 0);                    -- parallel output (clocked on rising spi_clk after last bit)
            done_o     : out std_logic											 -- handshake added to break continuous tx/rx 
        );                      
    end component;

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
    constant MIN_COLUMN                    : std_logic_vector(7 downto 0) := x"00";
    constant MAX_COLUMN                    : std_logic_vector(7 downto 0) := x"5F";
    constant MIN_ROW                       : std_logic_vector(7 downto 0) := x"00";
    constant MAX_ROW                       : std_logic_vector(7 downto 0) := x"3F";
    constant DRAW_LINE_COMMAND             : std_logic_vector(7 downto 0) := x"21";

    -- Data/Command
    constant DATA_TYPE    : std_logic := '1';
    constant COMMAND_TYPE : std_logic := '0';

    -- FSM
    type state_t is (s_off, s_turning_on, s_on, s_turning_off);
    signal state : state_t := s_off;
    
    -- SPI ignals
    signal spi_data_request : std_logic := '0';
    signal spi_data         : std_logic_vector(7 downto 0) := (others => '0');
    signal spi_write_enable : std_logic := '0';
    signal spi_write_ack    : std_logic := '0';
    signal spi_done         : std_logic := '0';

    -- Internal signals used during power on and power off transitions
    signal spi_data_internal    : std_logic_vector(7 downto 0) := (others => '0');
    signal spi_dc_internal      : std_logic := '0';
    signal spi_trigger_internal : std_logic := '0';
    
    -- Flag to indicate power on or power off sequence has ended
    signal transition_completed_flag : std_logic := '0';
    
    -- Delayed signals
    signal on_off_d : std_logic := '0';

    -- Timer
    constant TIMER_250MS      : integer := 31250000 - 1; -- 125MHz clock // 31,250,000 counts -> 250ms
    constant TIMER_400MS      : integer := 50000000 - 1; -- 125MHZ clock // 50,000,000 counts -> 400ms
    signal   max_timer_value  : integer := 0;
    signal   timer_value      : integer := 0;
    signal   timer_enable     : std_logic := '0';
    signal   timer_expired    : std_logic := '0';
    signal   timer_auto_reset : std_logic := '0';

    -- Procedure to start/stop the timer, configure the max_timer_value and configure the timer_auto_reset
    procedure configure_timer(signal   timer_enable       : out std_logic;
                              constant timer_enable_v     : in  std_logic;
                              signal   max_timer_value    : out integer;
                              constant max_timer_value_v  : in  integer;
                              signal   timer_auto_reset   : out std_logic;
                              constant timer_auto_reset_v : in  std_logic
                              ) is
    begin
        timer_enable     <= timer_enable_v;
        max_timer_value  <= max_timer_value_v;
        timer_auto_reset <= timer_auto_reset_v;
    end procedure;

begin

    -- Port Map
        spi_master_inst: spi_master
        Generic Map (
            N              => 8,   -- 1 Byte serial word length
            CPOL           => '1', -- Clock idles at high
            CPHA           => '1', -- Data sampled on (second) rising edge and shifted on (first) falling edge
            PREFETCH       => 2,   -- prefetch lookahead cycles
            SPI_2X_CLK_DIV => 10   -- 125MHz clk input // 20 clock divider => 6.25MHz SCK
        )
        Port Map (
            sclk_i => CLK,
            pclk_i => CLK,
            rst_i  => NOT RESETN,
            ---- serial interface ----
            spi_ssel_o => CS,
            spi_sck_o  => SCK,
            spi_mosi_o => MOSI,
            spi_miso_i => '0', -- Unused
            ---- parallel interface ----
            di_req_o   => spi_data_request,
            di_i       => spi_data,
            wren_i     => spi_write_enable,
            wr_ack_o   => spi_write_ack,
            do_valid_o => open,
            do_o       => open,
            done_o     => spi_done
        );

    -- Process to detect rising and falling edges in signals
    edge_detect_proc: process(CLK, RESETN)
    begin
        if (RESETN = '0') then
            on_off_d <= '0';
        elsif (rising_edge(CLK)) then
            on_off_d <= ON_OFF;
        end if;
    end process;

    -- General State Machine
    GSM_proc : process(CLK, RESETN)
    begin
        if (RESETN = '0') then
            state <= s_off;
        elsif (rising_edge(CLK)) then
            case state is
                when s_off =>
                    if (ON_OFF = '1' and on_off_d = '0') then
                        state <= s_turning_on;
                    end if;
                when s_turning_on =>
                    if (transition_completed_flag = '1' and ON_OFF = '1') then
                        state <= s_on;
                    end if;
                when s_on =>
                    if (ON_OFF = '0' and on_off_d = '1') then
                        state <= s_turning_off;
                    end if;
                when s_turning_off =>
                    if (transition_completed_flag = '1' and ON_OFF = '0') then
                        state <= s_off;
                    end if;
            end case;
        end if;
    end process;

    on_off_proc: process(CLK, RESETN)
        variable seq_counter : integer := 0;
    begin
        if (RESETN = '0') then
            spi_data_internal    <= (others => '0');
            spi_dc_internal      <= '0';
            spi_trigger_internal <= '0';

            POWER_RESET <= '1';
            VCC_ENABLE  <= '0';
            PMOD_ENABLE <= '0';

            seq_counter := 0;
            transition_completed_flag <= '0';

        elsif (rising_edge(CLK)) then
            case state is 
                when s_off =>
                    spi_data_internal    <= (others => '0');
                    spi_dc_internal      <= '0';
                    spi_trigger_internal <= '0';
        
                    POWER_RESET <= '1';
                    VCC_ENABLE  <= '0';
                    PMOD_ENABLE <= '0';
        
                    seq_counter := 0;
                    transition_completed_flag <= '0';

                when s_turning_on =>

                when s_on =>
                    spi_data_internal    <= (others => '0');
                    spi_dc_internal      <= '0';
                    spi_trigger_internal <= '0';
        
                    POWER_RESET <= '1';
                    VCC_ENABLE  <= '1';
                    PMOD_ENABLE <= '1';
        
                    seq_counter := 0;
                    transition_completed_flag <= '0';

                when s_turning_off =>

                    if (seq_counter = 0) then
                        spi_data_internal    <= DISPLAY_OFF_COMMAND;
                        spi_dc_internal      <= COMMAND_TYPE;
                        spi_trigger_internal <= '1';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 1 and spi_write_ack = '1') then
                        spi_trigger_internal <= '1';
                        seq_counter          := seq_counter + 1;
                    elsif (seq_counter = 2 and spi_done = '1') then
                        configure_timer(timer_enable, '1', max_timer_value, TIMER_400MS, timer_auto_reset, '0');
                        VCC_ENABLE  <= '0';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 3 and timer_expired = '1') then
                        configure_timer(timer_enable, '0', max_timer_value, 0, timer_auto_reset, '0');
                        transition_completed_flag <= '1';
                        seq_counter               := seq_counter + 1;
                    end if;
            end case;
        end if;
    end process;

    ON_OFF_STATUS <= "00" when (state = s_off) else
                     "01" when (state = s_turning_on) else
                     "10" when (state = s_turning_off) else
                     "11" when (state = s_on);

    SPI_READY <= spi_done when (state = s_on) else '0'; 

    spi_data         <= BYTE        when (state = s_on) else spi_data_internal;
    DATA_COMMAND     <= DC_SELECT   when (state = s_on) else spi_dc_internal;
    spi_write_enable <= SPI_TRIGGER when (state = s_on) else spi_trigger_internal;

    -- Configurable timer
    counter_proc: process(CLK, RESETN)
    begin
        if (RESETN = '0') then
            timer_value   <= 0;
            timer_expired <= '0';
        elsif (rising_edge(CLK)) then
            if (timer_enable = '1') then
                if (timer_value < max_timer_value) then
                    timer_value   <= timer_value + 1;
                    timer_expired <= '0';
                else
                    timer_expired <= '1';
                    if (timer_auto_reset = '1') then
                        timer_value <= 0;
                    end if;
                end if;
            else 
                timer_value   <= 0;
                timer_expired <= '0';
            end if;
        end if;
    end process;

end Behavioral;