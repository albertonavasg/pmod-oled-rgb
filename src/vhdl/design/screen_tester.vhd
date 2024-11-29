library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity screen_tester is
    Port ( 
        -- Sync
        CLK    : in std_logic;
        RESETN : in std_logic;

        -- Enable
        ENABLE : in std_logic;

        -- Control
        SPI_TRIGGER : out std_logic;

        -- Status
        ON_OFF_STATUS : in  std_logic_vector(1 downto 0);
        READY         : in  std_logic;

        -- Data
        BYTE      : out std_logic_vector(7 downto 0);
        DC_SELECT : out std_logic
    );
end screen_tester;

architecture Behavioral of screen_tester is

    -- Flags
    signal send_on_flag      : std_logic := '0';
    signal send_off_flag     : std_logic := '0';
    signal command_sent_flag : std_logic := '0';

    -- Delayed signals
    signal enable_d : std_logic := '0';
    
begin

    edge_detect_proc: process(CLK, RESETN)
    begin
        if (RESETN = '0') then
            enable_d <= '0';
        elsif (rising_edge(CLK)) then
            enable_d <= ENABLE;
        end if;
    end process;

    trigger_proc: process(CLK, RESETN)
    begin
        if (RESETN = '0') then
            send_on_flag  <= '0';
            send_off_flag <= '0';
        elsif (rising_edge(CLK)) then
            if (ENABLE = '1' and enable_d = '0' and ON_OFF_STATUS = "11" and READY = '1') then
                send_on_flag <= '1';
            end if;
            if (ENABLE = '0' and enable_d = '1' and ON_OFF_STATUS = "11" and READY = '1') then
                send_off_flag <= '1';
            end if;
            if (command_sent_flag = '1') then
                send_on_flag  <= '0';
                send_off_flag <= '0';
            end if;
        end if;
    end process;

    main_proc: process(CLK, RESETN)
        variable seq_counter : integer := 0;
    begin
        if (RESETN = '0') then
            BYTE           <= (others => '0');
            DC_SELECT      <= '0';
            SPI_TRIGGER    <= '0';
            seq_counter    := 0;
            command_sent_flag <= '0';
        elsif (rising_edge(CLK)) then
            if (send_on_flag = '1') then
                if (seq_counter = 0) then
                    BYTE        <= x"A5"; -- ENTIRE_DISPLAY_ON_COMMAND
                    DC_SELECT   <= '0';   -- COMMAND
                    SPI_TRIGGER <= '1';
                    seq_counter := seq_counter + 1;
                elsif (seq_counter = 1) then
                    SPI_TRIGGER             <= '0';
                    command_sent_flag <= '1';
                end if;
            elsif (send_off_flag = '1') then
                if (seq_counter = 0) then
                    BYTE        <= x"A6"; -- ENTIRE_DISPLAY_OFF_COMMAND
                    DC_SELECT   <= '0';   -- COMMAND
                    SPI_TRIGGER <= '1';
                    seq_counter := seq_counter + 1;
                elsif (seq_counter = 1) then
                    SPI_TRIGGER       <= '0';
                    command_sent_flag <= '1';
                end if;
            else
                SPI_TRIGGER       <= '0';
                seq_counter       := 0;
                command_sent_flag <= '0';
            end if;
        end if;
    end process;

end Behavioral;
