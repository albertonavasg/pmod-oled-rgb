library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity spi_master_tb is
    Generic (   
        N              : positive  := 8;   -- 1 Byte serial word length
        CPOL           : std_logic := '1'; -- Clock idles at high
        CPHA           : std_logic := '1'; -- Data sampled on (second) rising edge and shifted on (first) falling edge
        PREFETCH       : positive  := 2;   -- prefetch lookahead cycles
        SPI_2X_CLK_DIV : positive  := 10   -- 125MHz clk input // 20 clock divider => 6.25MHz SCK
    );  
    -- Port ( );
end spi_master_tb;

architecture Behavioral of spi_master_tb is

    -- Component Under Test
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
            --- debug ports: can be removed or left unconnected for the application circuit ---
            -- sck_ena_o     : out std_logic;                     -- debug: internal sck enable signal
            -- sck_ena_ce_o  : out std_logic;                     -- debug: internal sck clock enable signal
            -- do_transfer_o : out std_logic;                     -- debug: internal transfer driver
            -- wren_o        : out std_logic;                     -- debug: internal state of the wren_i pulse stretcher
            -- rx_bit_reg_o  : out std_logic;                     -- debug: internal rx bit
            -- state_dbg_o   : out std_logic_vector (3 downto 0); -- debug: internal state register
            -- core_clk_o    : out std_logic;
            -- core_n_clk_o  : out std_logic;
            -- core_ce_o     : out std_logic;
            -- core_n_ce_o   : out std_logic
            -- sh_reg_dbg_o  : out std_logic_vector (N-1 downto 0) -- debug: internal shift register
        );                      
    end component;

    -- Clock signal
    constant CLK_PERIOD : time := 8 ns;

    -- Signals for the component under test
    signal sclk_i : std_logic := '0';
    signal pclk_i : std_logic := '0';
    signal rst_i  : std_logic := '0';

    signal spi_ssel_o : std_logic := '0';
    signal spi_sck_o  : std_logic := '0';
    signal spi_mosi_o : std_logic := '0';
    signal spi_miso_i : std_logic := '0';

    signal di_req_o   : std_logic := '0';
    signal di_i       : std_logic_vector(N-1 downto 0)  := (others => '0');
    signal wren_i     : std_logic := '0';
    signal wr_ack_o   : std_logic := '0';
    signal do_valid_o : std_logic := '0';
    signal do_o       : std_logic_vector(N-1 downto 0)  := (others => '0');
    signal done_o     : std_logic := '0';

begin

    -- Port Map
    CUT: spi_master
    Generic Map (
        N              => N,             -- 1 Byte serial word length
        CPOL           => CPOL,          -- Clock idles at high
        CPHA           => CPHA,          -- Data sampled on (second) rising edge and shifted on (first) falling edge
        PREFETCH       => PREFETCH,      -- prefetch lookahead cycles
        SPI_2X_CLK_DIV => SPI_2X_CLK_DIV -- 125MHz clk input // 20 clock divider => 6.25MHz SCK
    )
    Port Map (
        sclk_i => sclk_i,
        pclk_i => pclk_i,
        rst_i  => rst_i,
        ---- serial interface ----
        spi_ssel_o => spi_ssel_o,
        spi_sck_o  => spi_sck_o,
        spi_mosi_o => spi_mosi_o,
        spi_miso_i => spi_miso_i,
        ---- parallel interface ----
        di_req_o   => di_req_o,
        di_i       => di_i,
        wren_i     => wren_i,
        wr_ack_o   => wr_ack_o,
        do_valid_o => do_valid_o,
        do_o       => do_o,
        done_o     => done_o
    );

        -- Stimulus processes
        clk_process: process
        begin
            sclk_i <= '0';
            pclk_i <= '0';
            wait for CLK_PERIOD / 2;
            sclk_i <= '1';
            pclk_i <= '1';
            wait for CLK_PERIOD / 2;
        end process;

        stimulus_process: process
        begin
            wait for 5*CLK_PERIOD;
                rst_i  <= '1';
            wait for 5*CLK_PERIOD;
                rst_i <= '0';
            wait for 5*CLK_PERIOD;
                wren_i <= '1';
                di_i   <= "01010101";
            wait for 1*CLK_PERIOD;
                wren_i <= '0';
            wait until di_req_o = '1'; -- wait until new data is requested
            wait for 1*CLK_PERIOD;
                wren_i <= '1';
                di_i   <= "11001100";
            wait for 1*CLK_PERIOD;
                wren_i <= '0';
            wait for 400*CLK_PERIOD;
                wren_i <= '1';
                di_i   <= "00110100";
            wait for 1*CLK_PERIOD;
                wren_i <= '0';
            wait until done_o = '1'; -- wait until this transaction is finished
                di_i <= "00000000";
            wait;
        end process;

end Behavioral;
