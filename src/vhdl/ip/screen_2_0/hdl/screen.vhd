library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity screen is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- Parameters of Axi Slave Bus Interface S00_AXI
		C_S00_AXI_DATA_WIDTH : integer := 32;
		C_S00_AXI_ADDR_WIDTH : integer := 4
	);
	port (
		-- Users to add ports here --
		-- Physical pins
		LED : out std_logic_vector(1 downto 0);
		PMOD: out std_logic_vector(7 downto 0);
		-- User ports ends --
		-- Do not modify the ports beyond this line

		-- Ports of Axi Slave Bus Interface S00_AXI
		s00_axi_aclk	: in  std_logic;
		s00_axi_aresetn	: in  std_logic;
		s00_axi_awaddr	: in  std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_awprot	: in  std_logic_vector(2 downto 0);
		s00_axi_awvalid	: in  std_logic;
		s00_axi_awready	: out std_logic;
		s00_axi_wdata	: in  std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_wstrb	: in  std_logic_vector((C_S00_AXI_DATA_WIDTH/8)-1 downto 0);
		s00_axi_wvalid	: in  std_logic;
		s00_axi_wready	: out std_logic;
		s00_axi_bresp	: out std_logic_vector(1 downto 0);
		s00_axi_bvalid	: out std_logic;
		s00_axi_bready	: in  std_logic;
		s00_axi_araddr	: in  std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_arprot	: in  std_logic_vector(2 downto 0);
		s00_axi_arvalid	: in  std_logic;
		s00_axi_arready	: out std_logic;
		s00_axi_rdata	: out std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_rresp	: out std_logic_vector(1 downto 0);
		s00_axi_rvalid	: out std_logic;
		s00_axi_rready	: in  std_logic
	);
end screen;

architecture arch_imp of screen is

	-- component declaration
	component screen_slave_lite_v2_0_S00_AXI is
		generic (
			C_S_AXI_DATA_WIDTH : integer := 32;
			C_S_AXI_ADDR_WIDTH : integer := 4
		);
		port (
			-- User ports --
			-- Control
			ON_OFF      : out std_logic;
			SPI_TRIGGER : out std_logic;
			-- Status
			ON_OFF_STATUS : in std_logic_vector(1 downto 0);
			SPI_READY     : in std_logic;
			--SPI data request
			SPI_DATA_REQUEST : in std_logic;
			-- Data input
			BYTE      : out std_logic_vector(7 downto 0);
			DC_SELECT : out std_logic;
			-- User ports end --
			S_AXI_ACLK    : in  std_logic;
			S_AXI_ARESETN : in  std_logic;
			S_AXI_AWADDR  : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
			S_AXI_AWPROT  : in  std_logic_vector(2 downto 0);
			S_AXI_AWVALID : in  std_logic;
			S_AXI_AWREADY : out std_logic;
			S_AXI_WDATA   : in  std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
			S_AXI_WSTRB   : in  std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
			S_AXI_WVALID  : in  std_logic;
			S_AXI_WREADY  : out std_logic;
			S_AXI_BRESP	  : out std_logic_vector(1 downto 0);
			S_AXI_BVALID  : out std_logic;
			S_AXI_BREADY  : in  std_logic;
			S_AXI_ARADDR  : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
			S_AXI_ARPROT  : in  std_logic_vector(2 downto 0);
			S_AXI_ARVALID : in  std_logic;
			S_AXI_ARREADY : out std_logic;
			S_AXI_RDATA   : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
			S_AXI_RRESP   : out std_logic_vector(1 downto 0);
			S_AXI_RVALID  : out std_logic;
			S_AXI_RREADY  : in  std_logic
		);
	end component screen_slave_lite_v2_0_S00_AXI;

	-- User component
	component screen_controller is
		Port (
			-- Sync
			CLK    : in std_logic;
			RESETN : in std_logic;
	
			-- Control
			ON_OFF      : in  std_logic;
			SPI_TRIGGER : in  std_logic;
	
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
	end component;

	-- User signals
	signal on_off           : std_logic;
	signal spi_trigger      : std_logic;
	signal on_off_status    : std_logic_vector(1 downto 0);
	signal spi_ready        : std_logic;
	signal spi_data_request : std_logic;
	signal byte             : std_logic_vector(7 downto 0);
	signal dc_select        : std_logic;

begin

	-- Instantiation of Axi Bus Interface S00_AXI
	screen_slave_lite_v2_0_S00_AXI_inst : screen_slave_lite_v2_0_S00_AXI
		generic map (
			C_S_AXI_DATA_WIDTH => C_S00_AXI_DATA_WIDTH,
			C_S_AXI_ADDR_WIDTH => C_S00_AXI_ADDR_WIDTH
		)
		port map (
			-- User ports --
			-- Control
			ON_OFF      => on_off,
			SPI_TRIGGER => spi_trigger,
			-- Status
			ON_OFF_STATUS => on_off_status,
			SPI_READY     => spi_ready,
			--SPI data request
			SPI_DATA_REQUEST => spi_data_request,
			-- Data input
			BYTE      => byte,
			DC_SELECT => dc_select,
			-- User ports end --
			S_AXI_ACLK    => s00_axi_aclk,
			S_AXI_ARESETN => s00_axi_aresetn,
			S_AXI_AWADDR  => s00_axi_awaddr,
			S_AXI_AWPROT  => s00_axi_awprot,
			S_AXI_AWVALID => s00_axi_awvalid,
			S_AXI_AWREADY => s00_axi_awready,
			S_AXI_WDATA   => s00_axi_wdata,
			S_AXI_WSTRB   => s00_axi_wstrb,
			S_AXI_WVALID  => s00_axi_wvalid,
			S_AXI_WREADY  => s00_axi_wready,
			S_AXI_BRESP	  => s00_axi_bresp,
			S_AXI_BVALID  => s00_axi_bvalid,
			S_AXI_BREADY  => s00_axi_bready,
			S_AXI_ARADDR  => s00_axi_araddr,
			S_AXI_ARPROT  => s00_axi_arprot,
			S_AXI_ARVALID => s00_axi_arvalid,
			S_AXI_ARREADY => s00_axi_arready,
			S_AXI_RDATA   => s00_axi_rdata,
			S_AXI_RRESP   => s00_axi_rresp,
			S_AXI_RVALID  => s00_axi_rvalid,
			S_AXI_RREADY  => s00_axi_rready
		);

	-- Add user logic here
	screen_controller_inst: screen_controller
		port map (
		-- Sync
		CLK    => s00_axi_aclk,
		RESETN => s00_axi_aresetn,

		-- Control
		ON_OFF      => on_off,
		SPI_TRIGGER => spi_trigger,

		-- Status
		ON_OFF_STATUS => on_off_status,
		SPI_READY     => spi_ready,

		-- SPI data request
		SPI_DATA_REQUEST => spi_data_request,

		-- Data input
		BYTE      => byte,
		DC_SELECT => dc_select,

		-- Pmod physical pins
		MOSI         => PMOD(1),
		SCK          => PMOD(3),
		CS           => PMOD(0),
		DATA_COMMAND => PMOD(4),
		POWER_RESET  => PMOD(5),
		VCC_ENABLE   => PMOD(6),
		PMOD_ENABLE  => PMOD(7)
		);

	LED <= on_off_status;
	PMOD(2) <= '0';
	-- User logic ends

end arch_imp;
