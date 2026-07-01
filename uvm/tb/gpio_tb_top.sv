import uvm_pkg::*;
import gpio_pkg::*;

module gpio_tb_top;
    logic clk;

    initial clk = 1'b0;
    always #5 clk = ~clk;

    gpio_if g_if(.clk(clk));

    gpio_v1_0 dut (
        .io_port          (g_if.io_port),
        .s00_axi_aclk     (g_if.clk),
        .s00_axi_aresetn  (g_if.aresetn),
        .s00_axi_awaddr   (g_if.awaddr),
        .s00_axi_awprot   (g_if.awprot),
        .s00_axi_awvalid  (g_if.awvalid),
        .s00_axi_awready  (g_if.awready),
        .s00_axi_wdata    (g_if.wdata),
        .s00_axi_wstrb    (g_if.wstrb),
        .s00_axi_wvalid   (g_if.wvalid),
        .s00_axi_wready   (g_if.wready),
        .s00_axi_bresp    (g_if.bresp),
        .s00_axi_bvalid   (g_if.bvalid),
        .s00_axi_bready   (g_if.bready),
        .s00_axi_araddr   (g_if.araddr),
        .s00_axi_arprot   (g_if.arprot),
        .s00_axi_arvalid  (g_if.arvalid),
        .s00_axi_arready  (g_if.arready),
        .s00_axi_rdata    (g_if.rdata),
        .s00_axi_rresp    (g_if.rresp),
        .s00_axi_rvalid   (g_if.rvalid),
        .s00_axi_rready   (g_if.rready)
    );

    initial begin
        uvm_config_db #(virtual gpio_if)::set(null, "*", "g_if", g_if);
        run_test();
    end

    initial begin
        $fsdbDumpfile("gpio_tb.fsdb");
        $fsdbDumpvars(0, gpio_tb_top);
        $fsdbDumpMDA();
    end
endmodule
