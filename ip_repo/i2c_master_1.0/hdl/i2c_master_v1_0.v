`timescale 1 ns / 1 ps

module i2c_master_v1_0 #
(
    parameter integer C_S00_AXI_DATA_WIDTH = 32,
    parameter integer C_S00_AXI_ADDR_WIDTH = 5
)
(
    inout  wire scl,
    inout  wire sda,
    output wire intr,

    input  wire                                  s00_axi_aclk,
    input  wire                                  s00_axi_aresetn,
    input  wire [C_S00_AXI_ADDR_WIDTH-1 : 0]     s00_axi_awaddr,
    input  wire [2 : 0]                          s00_axi_awprot,
    input  wire                                  s00_axi_awvalid,
    output wire                                  s00_axi_awready,
    input  wire [C_S00_AXI_DATA_WIDTH-1 : 0]     s00_axi_wdata,
    input  wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
    input  wire                                  s00_axi_wvalid,
    output wire                                  s00_axi_wready,
    output wire [1 : 0]                          s00_axi_bresp,
    output wire                                  s00_axi_bvalid,
    input  wire                                  s00_axi_bready,
    input  wire [C_S00_AXI_ADDR_WIDTH-1 : 0]     s00_axi_araddr,
    input  wire [2 : 0]                          s00_axi_arprot,
    input  wire                                  s00_axi_arvalid,
    output wire                                  s00_axi_arready,
    output wire [C_S00_AXI_DATA_WIDTH-1 : 0]     s00_axi_rdata,
    output wire [1 : 0]                          s00_axi_rresp,
    output wire                                  s00_axi_rvalid,
    input  wire                                  s00_axi_rready
);

    wire        start_pulse;
    wire        stop_enable;
    wire [7:0]  tx_data;
    wire [15:0] clk_div;
    wire [6:0]  slave_addr;
    wire        soft_reset_pulse;
    wire        core_busy;
    wire        core_done;
    wire        core_ack_error;
    wire        scl_oen;
    wire        sda_oen;
    wire        sda_i;
    reg         bus_error_pulse;
    reg [31:0]  watchdog_cnt;

    localparam [31:0] I2C_TIMEOUT_CYCLES = 32'd2000000;

    assign scl   = scl_oen ? 1'bz : 1'b0;
    assign sda   = sda_oen ? 1'bz : 1'b0;
    assign sda_i = sda;

    always @(posedge s00_axi_aclk or negedge s00_axi_aresetn) begin
        if (!s00_axi_aresetn) begin
            watchdog_cnt   <= 32'd0;
            bus_error_pulse <= 1'b0;
        end
        else begin
            bus_error_pulse <= 1'b0;
            if (core_busy) begin
                if (watchdog_cnt >= I2C_TIMEOUT_CYCLES) begin
                    watchdog_cnt   <= 32'd0;
                    bus_error_pulse <= 1'b1;
                end
                else begin
                    watchdog_cnt <= watchdog_cnt + 32'd1;
                end
            end
            else begin
                watchdog_cnt <= 32'd0;
            end
        end
    end

    i2c_master_v1_0_S00_AXI #(
        .C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
        .C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
    ) i2c_master_v1_0_S00_AXI_inst (
        .start_pulse     (start_pulse),
        .stop_enable     (stop_enable),
        .tx_data         (tx_data),
        .clk_div         (clk_div),
        .slave_addr      (slave_addr),
        .soft_reset_pulse(soft_reset_pulse),
        .core_busy       (core_busy),
        .core_done       (core_done),
        .core_ack_error  (core_ack_error),
        .bus_error_pulse (bus_error_pulse),
        .intr            (intr),
        .S_AXI_ACLK      (s00_axi_aclk),
        .S_AXI_ARESETN   (s00_axi_aresetn),
        .S_AXI_AWADDR    (s00_axi_awaddr),
        .S_AXI_AWPROT    (s00_axi_awprot),
        .S_AXI_AWVALID   (s00_axi_awvalid),
        .S_AXI_AWREADY   (s00_axi_awready),
        .S_AXI_WDATA     (s00_axi_wdata),
        .S_AXI_WSTRB     (s00_axi_wstrb),
        .S_AXI_WVALID    (s00_axi_wvalid),
        .S_AXI_WREADY    (s00_axi_wready),
        .S_AXI_BRESP     (s00_axi_bresp),
        .S_AXI_BVALID    (s00_axi_bvalid),
        .S_AXI_BREADY    (s00_axi_bready),
        .S_AXI_ARADDR    (s00_axi_araddr),
        .S_AXI_ARPROT    (s00_axi_arprot),
        .S_AXI_ARVALID   (s00_axi_arvalid),
        .S_AXI_ARREADY   (s00_axi_arready),
        .S_AXI_RDATA     (s00_axi_rdata),
        .S_AXI_RRESP     (s00_axi_rresp),
        .S_AXI_RVALID    (s00_axi_rvalid),
        .S_AXI_RREADY    (s00_axi_rready)
    );

    I2C_Master u_i2c_core (
        .clk        (s00_axi_aclk),
        .rst_n      (s00_axi_aresetn & ~soft_reset_pulse & ~bus_error_pulse),
        .start      (start_pulse),
        .stop_enable(stop_enable),
        .slave_addr (slave_addr),
        .tx_data    (tx_data),
        .clk_div    (clk_div),
        .sda_i      (sda_i),
        .busy       (core_busy),
        .done       (core_done),
        .ack_error  (core_ack_error),
        .scl_oen    (scl_oen),
        .sda_oen    (sda_oen)
    );

endmodule
