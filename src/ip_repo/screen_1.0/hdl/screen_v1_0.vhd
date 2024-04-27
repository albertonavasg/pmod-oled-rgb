library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity screen_v1_0 is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line


		-- Parameters of Axi Slave Bus Interface S00_AXI
		C_S00_AXI_DATA_WIDTH	: integer	:= 32;
		C_S00_AXI_ADDR_WIDTH	: integer	:= 4
	);
	port (
		-- Users to add ports here
		-- LEDs to show "ON_OFF", "READY" and "ON_OFF_STATUS"
        LED : out std_logic_vector(3 downto 0);
        -- PmodA
        JA_0_CS     : out std_logic;
        JA_1_MOSI   : out std_logic;
        JA_2_NC     : out std_logic;
        JA_3_SCK    : out std_logic;
        JA_4_DC     : out std_logic;
        JA_5_RES    : out std_logic;
        JA_6_VCCEN  : out std_logic;
        JA_7_PMODEN : out std_logic;
		-- User ports ends
		-- Do not modify the ports beyond this line


		-- Ports of Axi Slave Bus Interface S00_AXI
		s00_axi_aclk	: in std_logic;
		s00_axi_aresetn	: in std_logic;
		s00_axi_awaddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_awprot	: in std_logic_vector(2 downto 0);
		s00_axi_awvalid	: in std_logic;
		s00_axi_awready	: out std_logic;
		s00_axi_wdata	: in std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_wstrb	: in std_logic_vector((C_S00_AXI_DATA_WIDTH/8)-1 downto 0);
		s00_axi_wvalid	: in std_logic;
		s00_axi_wready	: out std_logic;
		s00_axi_bresp	: out std_logic_vector(1 downto 0);
		s00_axi_bvalid	: out std_logic;
		s00_axi_bready	: in std_logic;
		s00_axi_araddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_arprot	: in std_logic_vector(2 downto 0);
		s00_axi_arvalid	: in std_logic;
		s00_axi_arready	: out std_logic;
		s00_axi_rdata	: out std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_rresp	: out std_logic_vector(1 downto 0);
		s00_axi_rvalid	: out std_logic;
		s00_axi_rready	: in std_logic
	);
end screen_v1_0;

architecture arch_imp of screen_v1_0 is

	-- component declaration
	component screen_v1_0_S00_AXI is
		generic (
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		C_S_AXI_ADDR_WIDTH	: integer	:= 4
		);
		port (
		ON_OFF           : out std_logic;
		ON_OFF_STATUS    : in  std_logic_vector(1 downto 0);
		START	         : out std_logic;
		READY 		     : in  std_logic;
		DATA 		     : out std_logic_vector(7 downto 0);
		DATA_COMMAND_IN  : out std_logic;
		S_AXI_ACLK	: in std_logic;
		S_AXI_ARESETN	: in std_logic;
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		S_AXI_AWVALID	: in std_logic;
		S_AXI_AWREADY	: out std_logic;
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		S_AXI_WVALID	: in std_logic;
		S_AXI_WREADY	: out std_logic;
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		S_AXI_BVALID	: out std_logic;
		S_AXI_BREADY	: in std_logic;
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		S_AXI_ARVALID	: in std_logic;
		S_AXI_ARREADY	: out std_logic;
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		S_AXI_RVALID	: out std_logic;
		S_AXI_RREADY	: in std_logic
		);
	end component screen_v1_0_S00_AXI;

	-- User component
	component screen_controller is
		Port (
			-- Basic
			CLK   : in std_logic;
			RESETN : in std_logic;

			-- Control
			ON_OFF        : in  std_logic;
			ON_OFF_STATUS : out std_logic_vector(1 downto 0);
			START         : in  std_logic;
			READY         : out std_logic;

			-- Power 
			POWER_RESET : out std_logic;
			VCC_ENABLE  : out std_logic;
			PMOD_ENABLE : out std_logic;

			-- Data
			DATA             : in  std_logic_vector(7 downto 0);
			DATA_COMMAND_IN  : in  std_logic;
			DATA_COMMAND_OUT : out std_logic;

			-- SPI
			MOSI : out std_logic;
			SCK  : out std_logic;
			CS   : out std_logic;

			-- Debug
			SEQ_COUNTER_DBG           : out std_logic_vector(9 downto 0);
			START_SIGNAL_DBG          : out std_logic;
			READY_SIGNAL_DBG          : out std_logic;
			DATA_SIGNAL_DBG           : out std_logic_vector(7 downto 0);
			DATA_COMMAND_INTERNAL_DBG : out std_logic;
			EXPIRED_COUNTER_5US_DBG   : out std_logic;
			EXPIRED_COUNTER_20MS_DBG  : out std_logic;
			EXPIRED_COUNTER_25MS_DBG  : out std_logic;
			EXPIRED_COUNTER_100MS_DBG : out std_logic;
			EXPIRED_COUNTER_400MS_DBG : out std_logic;
			EXPIRED_COUNTER_SPI_DBG   : out std_logic
		);
	end component;

	-- User signals
	signal on_off_signal 		  : std_logic;
	signal on_off_status_signal   : std_logic_vector(1 downto 0);
	signal start_signal 		  : std_logic;
	signal ready_signal 		  : std_logic;
	signal data_signal			  : std_logic_vector(7 downto 0);
	signal data_command_in_signal : std_logic;

	-- Debug
	signal seq_counter_dbg_signal           : std_logic_vector(9 downto 0);
	signal start_signal_dbg_signal          : std_logic;
	signal ready_signal_dbg_signal          : std_logic;
	signal data_signal_dbg_signal 			: std_logic_vector(7 downto 0);
	signal data_command_internal_dbg_signal : std_logic;
	signal expired_counter_5us_dbg_signal   : std_logic;
	signal expired_counter_20ms_dbg_signal  : std_logic;
	signal expired_counter_25ms_dbg_signal  : std_logic;
	signal expired_counter_100ms_dbg_signal : std_logic;
	signal expired_counter_400ms_dbg_signal : std_logic;
	signal expired_counter_spi_dbg_signal   : std_logic;

begin

-- Instantiation of Axi Bus Interface S00_AXI
screen_v1_0_S00_AXI_inst : screen_v1_0_S00_AXI
	generic map (
		C_S_AXI_DATA_WIDTH	=> C_S00_AXI_DATA_WIDTH,
		C_S_AXI_ADDR_WIDTH	=> C_S00_AXI_ADDR_WIDTH
	)
	port map (
		ON_OFF 		    => on_off_signal,
		ON_OFF_STATUS   => on_off_status_signal,
		START 		    => start_signal,
		READY 		    => ready_signal,
		DATA 		    => data_signal,
		DATA_COMMAND_IN => data_command_in_signal,
		S_AXI_ACLK	=> s00_axi_aclk,
		S_AXI_ARESETN	=> s00_axi_aresetn,
		S_AXI_AWADDR	=> s00_axi_awaddr,
		S_AXI_AWPROT	=> s00_axi_awprot,
		S_AXI_AWVALID	=> s00_axi_awvalid,
		S_AXI_AWREADY	=> s00_axi_awready,
		S_AXI_WDATA	=> s00_axi_wdata,
		S_AXI_WSTRB	=> s00_axi_wstrb,
		S_AXI_WVALID	=> s00_axi_wvalid,
		S_AXI_WREADY	=> s00_axi_wready,
		S_AXI_BRESP	=> s00_axi_bresp,
		S_AXI_BVALID	=> s00_axi_bvalid,
		S_AXI_BREADY	=> s00_axi_bready,
		S_AXI_ARADDR	=> s00_axi_araddr,
		S_AXI_ARPROT	=> s00_axi_arprot,
		S_AXI_ARVALID	=> s00_axi_arvalid,
		S_AXI_ARREADY	=> s00_axi_arready,
		S_AXI_RDATA	=> s00_axi_rdata,
		S_AXI_RRESP	=> s00_axi_rresp,
		S_AXI_RVALID	=> s00_axi_rvalid,
		S_AXI_RREADY	=> s00_axi_rready
	);

	-- Add user logic here
	screen_controller_inst: screen_controller
		Port Map (
			-- Basic
			CLK   => s00_axi_aclk,
			RESETN => s00_axi_aresetn,

			-- Control
			ON_OFF        => on_off_signal,
			ON_OFF_STATUS => on_off_status_signal,
			START         => start_signal,
			READY         => ready_signal,

			-- Power 
			POWER_RESET => JA_5_RES,
			VCC_ENABLE  => JA_6_VCCEN,
			PMOD_ENABLE => JA_7_PMODEN,

			-- Data
			DATA             => data_signal,
			DATA_COMMAND_IN  => data_command_in_signal,
			DATA_COMMAND_OUT => JA_4_DC,

			-- SPI
			MOSI => JA_1_MOSI,
			SCK  => JA_3_SCK,
			CS   => JA_0_CS,

			-- Debug
			SEQ_COUNTER_DBG           => seq_counter_dbg_signal,
			START_SIGNAL_DBG          => start_signal_dbg_signal,
			READY_SIGNAL_DBG          => ready_signal_dbg_signal,
			DATA_SIGNAL_DBG           => data_signal_dbg_signal,
			DATA_COMMAND_INTERNAL_DBG => data_command_internal_dbg_signal,
			EXPIRED_COUNTER_5US_DBG   => expired_counter_5us_dbg_signal,
			EXPIRED_COUNTER_20MS_DBG  => expired_counter_20ms_dbg_signal,
			EXPIRED_COUNTER_25MS_DBG  => expired_counter_25ms_dbg_signal,
			EXPIRED_COUNTER_100MS_DBG => expired_counter_100ms_dbg_signal,
			EXPIRED_COUNTER_400MS_DBG => expired_counter_400ms_dbg_signal,
			EXPIRED_COUNTER_SPI_DBG   => expired_counter_spi_dbg_signal
		);

		LED     <= on_off_signal & ready_signal & on_off_status_signal;
		JA_2_NC <= '0';
	-- User logic ends

end arch_imp;
