`timescale 1 ns / 1 ps

module i2c_master_v1_0_S00_AXI #
(
    parameter integer C_S_AXI_DATA_WIDTH = 32,
    parameter integer C_S_AXI_ADDR_WIDTH = 5
)
(
    output reg         start_pulse,
    output wire        stop_enable,
    output wire [7:0]  tx_data,
    output wire [15:0] clk_div,
    output wire [6:0]  slave_addr,
    output reg         soft_reset_pulse,
    input  wire        core_busy,
    input  wire        core_done,
    input  wire        core_ack_error,
    input  wire        bus_error_pulse,
    output wire        intr,

    input  wire                                  S_AXI_ACLK,
    input  wire                                  S_AXI_ARESETN,
    input  wire [C_S_AXI_ADDR_WIDTH-1 : 0]       S_AXI_AWADDR,
    input  wire [2 : 0]                          S_AXI_AWPROT,
    input  wire                                  S_AXI_AWVALID,
    output wire                                  S_AXI_AWREADY,
    input  wire [C_S_AXI_DATA_WIDTH-1 : 0]       S_AXI_WDATA,
    input  wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0]   S_AXI_WSTRB,
    input  wire                                  S_AXI_WVALID,
    output wire                                  S_AXI_WREADY,
    output wire [1 : 0]                          S_AXI_BRESP,
    output wire                                  S_AXI_BVALID,
    input  wire                                  S_AXI_BREADY,
    input  wire [C_S_AXI_ADDR_WIDTH-1 : 0]       S_AXI_ARADDR,
    input  wire [2 : 0]                          S_AXI_ARPROT,
    input  wire                                  S_AXI_ARVALID,
    output wire                                  S_AXI_ARREADY,
    output wire [C_S_AXI_DATA_WIDTH-1 : 0]       S_AXI_RDATA,
    output wire [1 : 0]                          S_AXI_RRESP,
    output wire                                  S_AXI_RVALID,
    input  wire                                  S_AXI_RREADY
);

    localparam integer ADDR_LSB = (C_S_AXI_DATA_WIDTH/32) + 1;
    localparam integer OPT_MEM_ADDR_BITS = 2;

    reg [C_S_AXI_ADDR_WIDTH-1 : 0] axi_awaddr;
    reg                            axi_awready;
    reg                            axi_wready;
    reg [1 : 0]                    axi_bresp;
    reg                            axi_bvalid;
    reg [C_S_AXI_ADDR_WIDTH-1 : 0] axi_araddr;
    reg                            axi_arready;
    reg [C_S_AXI_DATA_WIDTH-1 : 0] axi_rdata;
    reg [1 : 0]                    axi_rresp;
    reg                            axi_rvalid;
    reg                            aw_en;

    reg [31:0] control_reg;
    reg [31:0] txdata_reg;
    reg [31:0] rxdata_reg;
    reg [31:0] clk_div_reg;
    reg [31:0] slave_addr_reg;
    reg [31:0] irq_enable_reg;
    reg [31:0] irq_status_reg;
    reg        done_latched;
    reg        ack_error_latched;
    reg        bus_error_latched;
    reg [31:0] reg_data_out;

    wire slv_reg_wren;
    wire slv_reg_rden;
    wire [2:0] wr_addr = axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB];
    wire [2:0] rd_addr = axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB];
    wire [2:0] irq_enable_effective;
    wire       irq_pending;
    wire [31:0] status_value;

    assign stop_enable = control_reg[1];
    assign tx_data     = txdata_reg[7:0];
    assign clk_div     = clk_div_reg[15:0];
    assign slave_addr  = slave_addr_reg[6:0];

    assign irq_enable_effective = irq_enable_reg[2:0] | {1'b0, control_reg[4], control_reg[3]};
    assign irq_pending = |(irq_status_reg[2:0] & irq_enable_effective);
    assign intr = irq_pending;

    assign status_value = {27'd0, irq_pending, bus_error_latched, ack_error_latched, done_latched, core_busy};

    assign S_AXI_AWREADY = axi_awready;
    assign S_AXI_WREADY  = axi_wready;
    assign S_AXI_BRESP   = axi_bresp;
    assign S_AXI_BVALID  = axi_bvalid;
    assign S_AXI_ARREADY = axi_arready;
    assign S_AXI_RDATA   = axi_rdata;
    assign S_AXI_RRESP   = axi_rresp;
    assign S_AXI_RVALID  = axi_rvalid;

    always @(posedge S_AXI_ACLK) begin
        if (S_AXI_ARESETN == 1'b0) begin
            axi_awready <= 1'b0;
            aw_en       <= 1'b1;
        end
        else begin
            if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en) begin
                axi_awready <= 1'b1;
                aw_en       <= 1'b0;
            end
            else if (S_AXI_BREADY && axi_bvalid) begin
                aw_en       <= 1'b1;
                axi_awready <= 1'b0;
            end
            else begin
                axi_awready <= 1'b0;
            end
        end
    end

    always @(posedge S_AXI_ACLK) begin
        if (S_AXI_ARESETN == 1'b0) begin
            axi_awaddr <= 0;
        end
        else if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en) begin
            axi_awaddr <= S_AXI_AWADDR;
        end
    end

    always @(posedge S_AXI_ACLK) begin
        if (S_AXI_ARESETN == 1'b0) begin
            axi_wready <= 1'b0;
        end
        else if (~axi_wready && S_AXI_WVALID && S_AXI_AWVALID && aw_en) begin
            axi_wready <= 1'b1;
        end
        else begin
            axi_wready <= 1'b0;
        end
    end

    assign slv_reg_wren = axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID;

    always @(posedge S_AXI_ACLK) begin
        if (S_AXI_ARESETN == 1'b0) begin
            control_reg       <= 32'h0000_0002;
            txdata_reg        <= 32'd0;
            rxdata_reg        <= 32'd0;
            clk_div_reg       <= 32'd250;
            slave_addr_reg    <= 32'h0000_0027;
            irq_enable_reg    <= 32'd0;
            irq_status_reg    <= 32'd0;
            done_latched      <= 1'b0;
            ack_error_latched <= 1'b0;
            bus_error_latched <= 1'b0;
            start_pulse       <= 1'b0;
            soft_reset_pulse  <= 1'b0;
        end
        else begin
            start_pulse      <= 1'b0;
            soft_reset_pulse <= 1'b0;
            control_reg[0]   <= 1'b0;
            control_reg[7]   <= 1'b0;

            if (core_done) begin
                done_latched <= 1'b1;
                irq_status_reg[0] <= 1'b1;
                if (core_ack_error) begin
                    ack_error_latched <= 1'b1;
                    irq_status_reg[1] <= 1'b1;
                end
            end

            if (bus_error_pulse) begin
                bus_error_latched <= 1'b1;
                irq_status_reg[2] <= 1'b1;
            end

            if (slv_reg_wren) begin
                case (wr_addr)
                    3'h0: begin
                        if (S_AXI_WSTRB[0]) begin
                            control_reg[4:1] <= S_AXI_WDATA[4:1];
                            if (S_AXI_WDATA[0]) begin
                                start_pulse       <= 1'b1;
                                done_latched      <= 1'b0;
                                ack_error_latched <= 1'b0;
                                bus_error_latched <= 1'b0;
                            end
                            if (S_AXI_WDATA[7]) begin
                                soft_reset_pulse  <= 1'b1;
                                done_latched      <= 1'b0;
                                ack_error_latched <= 1'b0;
                                bus_error_latched <= 1'b0;
                                irq_status_reg    <= 32'd0;
                            end
                        end
                    end
                    3'h2: begin
                        if (S_AXI_WSTRB[0]) begin
                            txdata_reg[7:0] <= S_AXI_WDATA[7:0];
                        end
                    end
                    3'h4: begin
                        if (S_AXI_WSTRB[0]) clk_div_reg[7:0]   <= S_AXI_WDATA[7:0];
                        if (S_AXI_WSTRB[1]) clk_div_reg[15:8]  <= S_AXI_WDATA[15:8];
                        if (S_AXI_WSTRB[2]) clk_div_reg[23:16] <= S_AXI_WDATA[23:16];
                        if (S_AXI_WSTRB[3]) clk_div_reg[31:24] <= S_AXI_WDATA[31:24];
                    end
                    3'h5: begin
                        if (S_AXI_WSTRB[0]) slave_addr_reg[6:0] <= S_AXI_WDATA[6:0];
                    end
                    3'h6: begin
                        if (S_AXI_WSTRB[0]) irq_enable_reg[2:0] <= S_AXI_WDATA[2:0];
                    end
                    3'h7: begin
                        if (S_AXI_WSTRB[0]) begin
                            irq_status_reg[2:0] <= irq_status_reg[2:0] & ~S_AXI_WDATA[2:0];
                        end
                    end
                    default: begin
                    end
                endcase
            end
        end
    end

    always @(posedge S_AXI_ACLK) begin
        if (S_AXI_ARESETN == 1'b0) begin
            axi_bvalid <= 1'b0;
            axi_bresp  <= 2'b0;
        end
        else if (axi_awready && S_AXI_AWVALID && ~axi_bvalid && axi_wready && S_AXI_WVALID) begin
            axi_bvalid <= 1'b1;
            axi_bresp  <= 2'b0;
        end
        else if (S_AXI_BREADY && axi_bvalid) begin
            axi_bvalid <= 1'b0;
        end
    end

    always @(posedge S_AXI_ACLK) begin
        if (S_AXI_ARESETN == 1'b0) begin
            axi_arready <= 1'b0;
            axi_araddr  <= 0;
        end
        else if (~axi_arready && S_AXI_ARVALID) begin
            axi_arready <= 1'b1;
            axi_araddr  <= S_AXI_ARADDR;
        end
        else begin
            axi_arready <= 1'b0;
        end
    end

    always @(posedge S_AXI_ACLK) begin
        if (S_AXI_ARESETN == 1'b0) begin
            axi_rvalid <= 1'b0;
            axi_rresp  <= 2'b0;
        end
        else if (axi_arready && S_AXI_ARVALID && ~axi_rvalid) begin
            axi_rvalid <= 1'b1;
            axi_rresp  <= 2'b0;
        end
        else if (axi_rvalid && S_AXI_RREADY) begin
            axi_rvalid <= 1'b0;
        end
    end

    assign slv_reg_rden = axi_arready & S_AXI_ARVALID & ~axi_rvalid;

    always @(*) begin
        case (rd_addr)
            3'h0: reg_data_out = control_reg;
            3'h1: reg_data_out = status_value;
            3'h2: reg_data_out = txdata_reg;
            3'h3: reg_data_out = rxdata_reg;
            3'h4: reg_data_out = clk_div_reg;
            3'h5: reg_data_out = slave_addr_reg;
            3'h6: reg_data_out = irq_enable_reg;
            3'h7: reg_data_out = irq_status_reg;
            default: reg_data_out = 32'd0;
        endcase
    end

    always @(posedge S_AXI_ACLK) begin
        if (S_AXI_ARESETN == 1'b0) begin
            axi_rdata <= 0;
        end
        else if (slv_reg_rden) begin
            axi_rdata <= reg_data_out;
        end
    end

endmodule
