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

        -- SPI data request
        SPI_DATA_REQUEST : out std_logic;

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

    -- Components
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

    -- SPI Commands
    constant EMPTY_COMMAND                 : std_logic_vector(7 downto 0) := x"00";
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

    -- SPI Data/Command
    constant DATA_TYPE    : std_logic := '1'; -- Unused in turning on and off sequence
    constant COMMAND_TYPE : std_logic := '0';

    -- Type definitions
    type spi_array is array(natural range <>) of std_logic_vector(7 downto 0);

    -- General State Machine
    type gsm_states is (gsm_off, gsm_turning_on, gsm_on, gsm_turning_off);
    signal gsm_state : gsm_states := gsm_off;
    
    -- SPI State Machine
    type   spi_states is (spi_idle, spi_write, spi_finish);
    signal spi_state : spi_states := spi_idle;
    
    -- SPI signals
    signal spi_data_req  : std_logic := '0';
    signal spi_data      : std_logic_vector(7 downto 0) := (others => '0');
    signal spi_write_en  : std_logic := '0';
    signal spi_write_ack : std_logic := '0';
    signal spi_done      : std_logic := '0';

    signal spi_data_array : spi_array(0 to 100) := (others => (others => '0'));
    signal spi_data_array_len : integer := 0;

    -- SPI command sequences
    constant ON_SEQUENCE_1 : spi_array(0 to 100) := (UNLOCK_COMMAND, UNLOCK_DATA,
                                                    DISPLAY_OFF_COMMAND,
                                                    REMAP_COMMAND, x"72",
                                                    DISPLAY_START_LINE_COMMAND, x"00",
                                                    DISPLAY_OFFSET_COMMAND, x"00",
                                                    NORMAL_DISPLAY_COMMAND,
                                                    MUX_RATIO_COMMAND, x"3F",
                                                    MASTER_CONFIG_COMMAND, EXT_VCC_SUPPLY_DATA,
                                                    POWER_SAVE_MODE_COMMAND, DISABLE_POWER_SAVE_MODE_DATA,
                                                    PHASE_LENGTH_COMMAND, x"31",
                                                    DISP_CLK_DIV_OSC_FREQ_COMMAND, x"F0",
                                                    SECOND_PRECHARGE_A_COMMAND, x"64",
                                                    SECOND_PRECHARGE_B_COMMAND, x"78",
                                                    SECOND_PRECHARGE_C_COMMAND, x"64",
                                                    PRECHARGE_COMMAND, x"3A",
                                                    VCOMH_COMMAND, x"3E",
                                                    MASTER_CURRENT_COMMAND, x"06",
                                                    CONTRAST_A_COMMAND, x"91",
                                                    CONTRAST_B_COMMAND, x"50",
                                                    CONTRAST_C_COMMAND, x"7D",
                                                    DISABLE_SCROLL_COMMAND,
                                                    CLEAR_WINDOW_COMMAND, MIN_COLUMN, MIN_ROW, MAX_COLUMN, MAX_ROW,
                                                    others => EMPTY_COMMAND);
    constant ON_SEQUENCE_1_LEN : integer := 44;

    constant on_sequence_2 : spi_array(0 to 100) := (DISPLAY_ON_COMMAND, others => EMPTY_COMMAND);
    constant ON_SEQUENCE_2_LEN : integer := 1;

    constant off_sequence : spi_array(0 to 100) := (DISPLAY_OFF_COMMAND, others => EMPTY_COMMAND);
    constant OFF_SEQUENCE_LEN : integer := 1;

    -- Internal signals used during power on and power off transitions
    signal spi_data_internal    : std_logic_vector(7 downto 0) := (others => '0');
    signal spi_dc_internal      : std_logic := '0';
    signal spi_trigger_internal : std_logic := '0';

    -- SPI flags
    signal spi_start_flag : std_logic := '0';
    signal spi_done_flag  : std_logic := '0';

    -- Flag to indicate power_on or power_off sequence has ended
    signal transition_completed_flag : std_logic := '0';
    
    -- Delayed signals
    signal on_off_d         : std_logic := '0';
    signal spi_data_req_d   : std_logic := '0';
    signal spi_write_ack_d  : std_logic := '0';
    signal spi_start_flag_d : std_logic := '0';
    signal spi_done_flag_d  : std_logic := '0';


    -- Timer
    constant TIMER_5US        : integer := 625 - 1;      -- 125MHz clock // 625 counts        -> 5us
    constant TIMER_20MS       : integer := 2500000 - 1;  -- 125MHz clock // 2,500,000 counts  -> 20ms  2000 for simulation
    constant TIMER_25MS       : integer := 3125000 - 1;  -- 125MHz clock // 3,125,000 counts  -> 25ms  2500 for simulation
    constant TIMER_100MS      : integer := 1250000 - 1;  -- 125MHz clock // 1,250,000 counts  -> 100ms 3000 for simulation
    constant TIMER_250MS      : integer := 31250000 - 1; -- 125MHz clock // 31,250,000 counts -> 250ms 4000 for simulation
    constant TIMER_400MS      : integer := 50000000 - 1; -- 125MHZ clock // 50,000,000 counts -> 400ms 5000 for simulation
    signal   max_timer_value  : integer := 0;
    signal   timer_value      : integer := 0;
    signal   timer_enable     : std_logic := '0';
    signal   timer_expired    : std_logic := '0';
    signal   timer_auto_reset : std_logic := '0';

    -- Procedure to start/stop the timer, configure the max_timer_value and configure the timer_auto_reset
    procedure configure_timer (
        signal   timer_enable       : out std_logic;
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

    -- General State Machine
    GSM_proc : process(CLK, RESETN)
    begin
        if (RESETN = '0') then
            gsm_state <= gsm_off;
        elsif (rising_edge(CLK)) then
            case gsm_state is
                when gsm_off =>
                    if (ON_OFF = '1' and on_off_d = '0') then
                        gsm_state <= gsm_turning_on;
                    end if;
                when gsm_turning_on =>
                    if (transition_completed_flag = '1' and ON_OFF = '1') then
                        gsm_state <= gsm_on;
                    end if;
                when gsm_on =>
                    if (ON_OFF = '0' and on_off_d = '1') then
                        gsm_state <= gsm_turning_off;
                    end if;
                when gsm_turning_off =>
                    if (transition_completed_flag = '1' and ON_OFF = '0') then
                        gsm_state <= gsm_off;
                    end if;
            end case;
        end if;
    end process;

    on_off_proc: process(CLK, RESETN)
        variable seq_counter : integer := 0;
    begin
        if (RESETN = '0') then
            spi_data_array <= (others => (others => '0'));
            spi_start_flag <= '0';

            POWER_RESET <= '1';
            VCC_ENABLE  <= '0';
            PMOD_ENABLE <= '0';

            seq_counter := 0;
            transition_completed_flag <= '0';

        elsif (rising_edge(CLK)) then
            case gsm_state is 
                when gsm_off =>
                    spi_data_array <= (others => (others => '0'));
                    spi_start_flag <= '0';
        
                    POWER_RESET <= '1';
                    VCC_ENABLE  <= '0';
                    PMOD_ENABLE <= '0';
        
                    seq_counter := 0;
                    transition_completed_flag <= '0';

                when gsm_turning_on =>
                    if (seq_counter = 0) then
                        configure_timer(timer_enable, '1', max_timer_value, TIMER_20MS, timer_auto_reset, '0');
                        PMOD_ENABLE <= '1';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 1 and timer_expired = '1') then
                        configure_timer(timer_enable, '0', max_timer_value, 0, timer_auto_reset, '0');
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 2) then
                        configure_timer(timer_enable, '1', max_timer_value, TIMER_5US, timer_auto_reset, '0');
                        POWER_RESET <= '0';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 3 and timer_expired = '1') then
                        configure_timer(timer_enable, '0', max_timer_value, 0, timer_auto_reset, '0');
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 4) then
                        configure_timer(timer_enable, '1', max_timer_value, TIMER_5US, timer_auto_reset, '0');
                        POWER_RESET <= '1';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 5 and timer_expired = '1') then
                        configure_timer(timer_enable, '0', max_timer_value, 0, timer_auto_reset, '0');
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 6) then
                        spi_data_array     <= ON_SEQUENCE_1;
                        spi_data_array_len <= ON_SEQUENCE_1_LEN;
                        spi_start_flag     <= '1';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 7) then
                        spi_start_flag <= '0';
                        seq_counter    := seq_counter + 1;
                    elsif (seq_counter = 8 and spi_done_flag = '1' and spi_done_flag_d = '0') then
                        configure_timer(timer_enable, '1', max_timer_value, TIMER_25MS, timer_auto_reset, '0');
                        VCC_ENABLE  <= '1';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 9 and timer_expired = '1') then
                        configure_timer(timer_enable, '0', max_timer_value, 0, timer_auto_reset, '0');
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 10) then
                        spi_data_array     <= ON_SEQUENCE_2;
                        spi_data_array_len <= ON_SEQUENCE_2_LEN;
                        spi_start_flag     <= '1';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 11) then
                        spi_start_flag <= '0';
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 12 and spi_done_flag = '1' and spi_done_flag_d = '0') then
                        configure_timer(timer_enable, '1', max_timer_value, TIMER_100MS, timer_auto_reset, '0');
                        seq_counter := seq_counter + 1;
                    elsif (seq_counter = 13 and timer_expired = '1') then
                        configure_timer(timer_enable, '0', max_timer_value, 0, timer_auto_reset, '0');
                        transition_completed_flag <= '1';
                        seq_counter               := seq_counter + 1;
                    end if;
                when gsm_on =>
                    spi_data_array <= (others => (others => '0'));
                    spi_start_flag <= '0';
        
                    POWER_RESET <= '1';
                    VCC_ENABLE  <= '1';
                    PMOD_ENABLE <= '1';
        
                    seq_counter := 0;
                    transition_completed_flag <= '0';

                when gsm_turning_off =>
                    if (seq_counter = 0) then
                        spi_data_array     <= OFF_SEQUENCE;
                        spi_data_array_len <= OFF_SEQUENCE_LEN;
                        spi_start_flag     <= '1';
                        seq_counter        := seq_counter + 1;
                    elsif (seq_counter = 1) then
                        spi_start_flag <= '0';
                        seq_counter    := seq_counter + 1;
                    elsif (seq_counter = 2 and spi_done_flag = '1' and spi_done_flag_d = '0') then
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

    -- SPI Communication process
    spi_proc: process(CLK, RESETN)
        variable write_byte_index : integer := 0;
    begin
        if (RESETN = '0') then
            spi_done_flag        <= '0';
            spi_data_internal    <= (others => '0');
            spi_trigger_internal <= '0';
            write_byte_index := 0;
            spi_state        <= spi_idle;
        elsif (rising_edge(CLK)) then
            case spi_state is
                when spi_idle =>         
                    spi_done_flag        <= '0';
                    spi_data_internal    <= (others => '0');
                    spi_trigger_internal <= '0';
                    write_byte_index     := 0;
                    if (spi_start_flag = '1' and spi_start_flag_d = '0' and spi_done = '1') then
                        spi_data_internal    <= spi_data_array(write_byte_index);
                        spi_trigger_internal <= '1';
                        write_byte_index     := write_byte_index + 1;
                        spi_state            <= spi_write;
                    end if;
                when spi_write =>
                    if (write_byte_index < spi_data_array_len) then
                        -- Turn off write_en when write_ack is received
                        if (spi_write_ack = '1' and spi_write_ack_d = '0' and spi_trigger_internal = '1') then
                            spi_trigger_internal <= '0';
                        end if;
                        -- Write
                        if (write_byte_index < spi_data_array_len and spi_data_req = '1' and spi_data_req_d = '0') then
                            spi_data_internal    <= spi_data_array(write_byte_index);
                            spi_trigger_internal <= '1';
                            write_byte_index     := write_byte_index + 1;
                        end if;
                    else
                        -- Turn off write_en when write_ack is received
                        if (spi_write_ack = '1' and spi_write_ack_d = '0' and spi_trigger_internal = '1') then
                            spi_trigger_internal <= '0';
                        end if;
                        if (spi_done = '1') then
                            spi_state <= spi_finish;
                        end if;
                    end if;
                when spi_finish =>
                    spi_done_flag <= '1';
                    spi_state     <= spi_idle;
            end case;
        end if;
    end process;

    ON_OFF_STATUS <= "00" when (gsm_state = gsm_off) else
                     "01" when (gsm_state = gsm_turning_on) else
                     "10" when (gsm_state = gsm_turning_off) else
                     "11" when (gsm_state = gsm_on);

    -- Bypass SPI status signals when screen is ON
    -- Set them as 0 in the other cases
    SPI_READY        <= spi_done     when (gsm_state = gsm_on) else '0'; 
    SPI_DATA_REQUEST <= spi_data_req when (gsm_state = gsm_on) else '0';

    -- Bypass SPI control signals when screen is ON
    -- Control them internally in the other cases
    spi_data     <= BYTE        when (gsm_state = gsm_on) else spi_data_internal;
    DATA_COMMAND <= DC_SELECT   when (gsm_state = gsm_on) else spi_dc_internal;
    spi_write_en <= SPI_TRIGGER when (gsm_state = gsm_on) else spi_trigger_internal;

    -- Process to delay signals and detect rising and falling edges
    delay_signal_proc: process(CLK, RESETN)
    begin
        if (RESETN = '0') then
            on_off_d          <= '0';
            spi_data_req_d    <= '0';
            spi_write_ack_d   <= '0';
            spi_start_flag_d  <= '0';
            spi_done_flag_d   <= '0';
        elsif (rising_edge(CLK)) then
            on_off_d          <= ON_OFF;
            spi_data_req_d    <= spi_data_req;
            spi_write_ack_d   <= spi_write_ack;
            spi_start_flag_d  <= spi_start_flag;
            spi_done_flag_d   <= spi_done_flag;
        end if;
    end process;

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
            di_req_o   => spi_data_req,
            di_i       => spi_data,
            wren_i     => spi_write_en,
            wr_ack_o   => spi_write_ack,
            do_valid_o => open,
            do_o       => open,
            done_o     => spi_done
        );

end Behavioral;