`timescale 1 ns / 1 ps

module SPI_v1_0 #(
    // Users to add parameters here

    // User parameters ends
    // Do not modify the parameters beyond this line


    // Parameters of Axi Slave Bus Interface S00_AXI
    parameter integer C_S00_AXI_DATA_WIDTH = 32,
    parameter integer C_S00_AXI_ADDR_WIDTH = 4
) (
    // Users to add ports here
    output wire sclk,
    output wire mosi,
    input  wire miso,
    output wire ss_n,

    output wire intr,
    // User ports ends
    // Do not modify the ports beyond this line


    // Ports of Axi Slave Bus Interface S00_AXI
    input  wire                                  s00_axi_aclk,
    input  wire                                  s00_axi_aresetn,
    input  wire [    C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
    input  wire [                         2 : 0] s00_axi_awprot,
    input  wire                                  s00_axi_awvalid,
    output wire                                  s00_axi_awready,
    input  wire [    C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
    input  wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
    input  wire                                  s00_axi_wvalid,
    output wire                                  s00_axi_wready,
    output wire [                         1 : 0] s00_axi_bresp,
    output wire                                  s00_axi_bvalid,
    input  wire                                  s00_axi_bready,
    input  wire [    C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
    input  wire [                         2 : 0] s00_axi_arprot,
    input  wire                                  s00_axi_arvalid,
    output wire                                  s00_axi_arready,
    output wire [    C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
    output wire [                         1 : 0] s00_axi_rresp,
    output wire                                  s00_axi_rvalid,
    input  wire                                  s00_axi_rready
);

    wire       spi_valid;
    wire       spi_ready;
    wire [7:0] tx_data;
    wire [7:0] rx_data;
    wire       spi_done;

    wire       rx_ie;
    wire       ss_hold;

    assign intr = rx_ie & spi_done;

    // Instantiation of Axi Bus Interface S00_AXI
    SPI_v1_0_S00_AXI #(
        .C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
        .C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
    ) SPI_v1_0_S00_AXI_inst (
        .spi_valid(spi_valid),
        .spi_ready(spi_ready),
        .tx_data  (tx_data),
        .rx_data  (rx_data),
        .spi_done (spi_done),
        .rx_ie    (rx_ie),
        .ss_hold (ss_hold),

        .S_AXI_ACLK   (s00_axi_aclk),
        .S_AXI_ARESETN(s00_axi_aresetn),
        .S_AXI_AWADDR (s00_axi_awaddr),
        .S_AXI_AWPROT (s00_axi_awprot),
        .S_AXI_AWVALID(s00_axi_awvalid),
        .S_AXI_AWREADY(s00_axi_awready),
        .S_AXI_WDATA  (s00_axi_wdata),
        .S_AXI_WSTRB  (s00_axi_wstrb),
        .S_AXI_WVALID (s00_axi_wvalid),
        .S_AXI_WREADY (s00_axi_wready),
        .S_AXI_BRESP  (s00_axi_bresp),
        .S_AXI_BVALID (s00_axi_bvalid),
        .S_AXI_BREADY (s00_axi_bready),
        .S_AXI_ARADDR (s00_axi_araddr),
        .S_AXI_ARPROT (s00_axi_arprot),
        .S_AXI_ARVALID(s00_axi_arvalid),
        .S_AXI_ARREADY(s00_axi_arready),
        .S_AXI_RDATA  (s00_axi_rdata),
        .S_AXI_RRESP  (s00_axi_rresp),
        .S_AXI_RVALID (s00_axi_rvalid),
        .S_AXI_RREADY (s00_axi_rready)
    );

    // Add user logic here
    spi_master dut (
        .clk      (s00_axi_aclk),
        .reset    (s00_axi_aresetn),
        .spi_valid(spi_valid),
        .spi_ready(spi_ready),
        .cpol     (1'b0),
        .cpha     (1'b0),
        .clk_div  (8'd49),
        .tx_data  (tx_data),
        .rx_data  (rx_data),
        .spi_done (spi_done),
        .ss_hold (ss_hold),
        .sclk     (sclk),
        .mosi     (mosi),
        .miso     (miso),
        .ss_n     (ss_n)
    );
    // User logic ends

endmodule

module spi_master (
    // global signals
    input wire clk,
    input wire reset,

    // AXI-side transfer interface
    input  wire       spi_valid,  // 1-clock transfer request pulse
    output wire       spi_ready,  // 1 when a new transfer can be accepted
    input  wire       cpol,
    input  wire       cpha,
    input  wire [7:0] clk_div,
    input  wire [7:0] tx_data,
    output reg  [7:0] rx_data,
    output reg        spi_done,   // 1-clock transfer complete pulse
    input wire        ss_hold,

    // external SPI pins
    output wire sclk,
    output reg  mosi,
    input  wire miso,
    output reg  ss_n
);

    localparam ST_IDLE = 2'b00;
    localparam ST_START = 2'b01;
    localparam ST_DATA = 2'b10;
    localparam ST_STOP = 2'b11;

    reg [1:0] spi_state;

    reg [7:0] clk_div_r;
    reg [7:0] tx_shift_reg;
    reg [7:0] rx_shift_reg;

    reg [7:0] div_cnt;
    reg [2:0] bit_cnt;
    reg       half_tick;
    reg       edge_step;

    reg       cpol_r;
    reg       cpha_r;
    reg       sclk_r;

    // MISO 2-stage synchronizer
    reg       miso_d;
    reg       miso_sync;

    assign sclk      = sclk_r;
    assign spi_ready = (spi_state == ST_IDLE);

    always @(posedge clk or negedge reset) begin
        if (!reset) begin
            miso_d    <= 1'b1;
            miso_sync <= 1'b1;
        end else begin
            miso_d    <= miso;
            miso_sync <= miso_d;
        end
    end

    // SCLK half-period tick generator
    always @(posedge clk or negedge reset) begin
        if (!reset) begin
            div_cnt   <= 8'd0;
            half_tick <= 1'b0;
        end else begin
            if (spi_state == ST_DATA) begin
                if (div_cnt == clk_div_r) begin
                    div_cnt   <= 8'd0;
                    half_tick <= 1'b1;
                end else begin
                    div_cnt   <= div_cnt + 1'b1;
                    half_tick <= 1'b0;
                end
            end else begin
                div_cnt   <= 8'd0;
                half_tick <= 1'b0;
            end
        end
    end

    // SPI transfer FSM
    always @(posedge clk or negedge reset) begin
        if (!reset) begin
            spi_state    <= ST_IDLE;
            mosi         <= 1'b1;
            ss_n         <= 1'b1;
            spi_done     <= 1'b0;
            tx_shift_reg <= 8'd0;
            rx_shift_reg <= 8'd0;
            rx_data      <= 8'd0;
            clk_div_r    <= 8'd49;
            bit_cnt      <= 3'd0;
            edge_step    <= 1'b0;
            sclk_r       <= 1'b0;
            cpol_r       <= 1'b0;
            cpha_r       <= 1'b0;
        end else begin
            spi_done <= 1'b0;

            case (spi_state)
                ST_IDLE: begin
                    mosi   <= 1'b1;
                    ss_n   <= ss_hold ? 1'b0 : 1'b1;
                    sclk_r <= cpol;

                    if (spi_valid) begin
                        spi_state    <= ST_START;
                        cpol_r       <= cpol;
                        cpha_r       <= cpha;
                        clk_div_r    <= clk_div;
                        tx_shift_reg <= tx_data;
                        rx_shift_reg <= 8'd0;
                        bit_cnt      <= 3'd0;
                        edge_step    <= 1'b0;
                        ss_n         <= 1'b0;
                    end
                end

                ST_START: begin
                    if (!cpha_r) begin
                        mosi         <= tx_shift_reg[7];
                        tx_shift_reg <= {tx_shift_reg[6:0], 1'b0};
                    end
                    spi_state <= ST_DATA;
                end

                ST_DATA: begin
                    if (half_tick) begin
                        sclk_r <= ~sclk_r;

                        if (edge_step == 1'b0) begin
                            edge_step <= 1'b1;

                            if (!cpha_r) begin
                                rx_shift_reg <= {rx_shift_reg[6:0], miso_sync};
                            end else begin
                                mosi <= tx_shift_reg[7];
                                tx_shift_reg <= {tx_shift_reg[6:0], 1'b0};
                            end
                        end else begin
                            edge_step <= 1'b0;

                            if (!cpha_r) begin
                                if (bit_cnt < 3'd7) begin
                                    mosi <= tx_shift_reg[7];
                                    tx_shift_reg <= {tx_shift_reg[6:0], 1'b0};
                                end
                            end else begin
                                rx_shift_reg <= {rx_shift_reg[6:0], miso_sync};
                            end

                            if (bit_cnt == 3'd7) begin
                                spi_state <= ST_STOP;
                                if (!cpha_r) begin
                                    rx_data <= rx_shift_reg;
                                end else begin
                                    rx_data <= {rx_shift_reg[6:0], miso_sync};
                                end
                            end else begin
                                bit_cnt <= bit_cnt + 1'b1;
                            end
                        end
                    end
                end

                ST_STOP: begin
                    sclk_r    <= cpol_r;
                    ss_n      <= ss_hold ? 1'b0 : 1'b1;
                    mosi      <= 1'b1;
                    spi_done  <= 1'b1;
                    spi_state <= ST_IDLE;
                end

                default: begin
                    spi_state <= ST_IDLE;
                end
            endcase
        end
    end

endmodule


