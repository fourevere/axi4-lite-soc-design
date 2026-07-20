interface gpio_if(input logic clk);
    logic        aresetn;

    logic [3:0]  awaddr;
    logic [2:0]  awprot;
    logic        awvalid;
    logic        awready;
    logic [31:0] wdata;
    logic [3:0]  wstrb;
    logic        wvalid;
    logic        wready;
    logic [1:0]  bresp;
    logic        bvalid;
    logic        bready;

    logic [3:0]  araddr;
    logic [2:0]  arprot;
    logic        arvalid;
    logic        arready;
    logic [31:0] rdata;
    logic [1:0]  rresp;
    logic        rvalid;
    logic        rready;

    tri [7:0]    io_port;
    logic [7:0]  ext_drive_en;
    logic [7:0]  ext_drive_data;
    logic [7:0]  io_sample;

    int unsigned tr_id;
    int unsigned scenario_id;
    logic        mon_valid;
    logic        mon_completed;
    logic [3:0]  mon_wstrb;
    logic [7:0]  mon_cr_expected;
    logic [7:0]  mon_odr_expected;
    logic [7:0]  mon_idr_expected;
    logic [7:0]  mon_io_expected;
    logic [7:0]  mon_cr_actual;
    logic [7:0]  mon_odr_actual;
    logic [7:0]  mon_idr_actual;
    logic [7:0]  mon_io_actual;
    logic [7:0]  mon_ext_drive_en;
    logic [7:0]  mon_ext_drive_data;

    genvar i;
    generate
        for (i = 0; i < 8; i++) begin : gen_ext_drive
            assign io_port[i] = ext_drive_en[i] ? ext_drive_data[i] : 1'bz;
        end
    endgenerate

    assign io_sample = io_port;
endinterface
