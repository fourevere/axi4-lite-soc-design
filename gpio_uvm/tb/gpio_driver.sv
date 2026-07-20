class gpio_driver extends uvm_driver #(gpio_seq_item);
    `uvm_component_utils(gpio_driver)

    virtual gpio_if g_if;
    int unsigned next_tr_id;

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if (!uvm_config_db #(virtual gpio_if)::get(this, "", "g_if", g_if)) begin
            `uvm_fatal(get_type_name(), "virtual interface g_if not found")
        end
    endfunction

    task init_bus();
        g_if.aresetn        <= 1'b0;
        g_if.awaddr         <= 4'h0;
        g_if.awprot         <= 3'h0;
        g_if.awvalid        <= 1'b0;
        g_if.wdata          <= 32'h0;
        g_if.wstrb          <= 4'h0;
        g_if.wvalid         <= 1'b0;
        g_if.bready         <= 1'b0;
        g_if.araddr         <= 4'h0;
        g_if.arprot         <= 3'h0;
        g_if.arvalid        <= 1'b0;
        g_if.rready         <= 1'b0;
        g_if.ext_drive_en   <= 8'hff;
        g_if.ext_drive_data <= 8'h00;
        g_if.mon_valid      <= 1'b0;
        g_if.tr_id          <= 0;
        g_if.scenario_id    <= GPIO_SCEN_RESET_DEFAULT;
    endtask

    task apply_reset(bit [7:0] ext_data = 8'h00);
        g_if.ext_drive_en   <= 8'hff;
        g_if.ext_drive_data <= ext_data;
        g_if.aresetn        <= 1'b0;
        repeat (5) @(posedge g_if.clk);
        g_if.aresetn        <= 1'b1;
        repeat (3) @(posedge g_if.clk);
    endtask

    task axi_write(bit [3:0] addr, bit [31:0] data, bit [3:0] strb = 4'b1111);
        @(posedge g_if.clk);
        g_if.awaddr  <= addr;
        g_if.awprot  <= 3'h0;
        g_if.awvalid <= 1'b1;
        g_if.wdata   <= data;
        g_if.wstrb   <= strb;
        g_if.wvalid  <= 1'b1;
        g_if.bready  <= 1'b1;

        do begin
            @(posedge g_if.clk);
        end while (!(g_if.awready && g_if.wready));

        g_if.awvalid <= 1'b0;
        g_if.wvalid  <= 1'b0;

        do begin
            @(posedge g_if.clk);
        end while (!g_if.bvalid);

        if (g_if.bresp != 2'b00) begin
            `uvm_error(get_type_name(), $sformatf("AXI write response error addr=0x%0h bresp=%0b", addr, g_if.bresp))
        end

        @(posedge g_if.clk);
        g_if.bready <= 1'b0;
    endtask

    task axi_read(bit [3:0] addr, output bit [31:0] data);
        @(posedge g_if.clk);
        g_if.araddr  <= addr;
        g_if.arprot  <= 3'h0;
        g_if.arvalid <= 1'b1;
        g_if.rready  <= 1'b1;

        do begin
            @(posedge g_if.clk);
        end while (!g_if.arready);

        g_if.arvalid <= 1'b0;

        do begin
            @(posedge g_if.clk);
        end while (!g_if.rvalid);

        data = g_if.rdata;
        if (g_if.rresp != 2'b00) begin
            `uvm_error(get_type_name(), $sformatf("AXI read response error addr=0x%0h rresp=%0b", addr, g_if.rresp))
        end

        @(posedge g_if.clk);
        g_if.rready <= 1'b0;
    endtask

    function bit [7:0] expected_pin_value(bit [7:0] cr, bit [7:0] odr, bit [7:0] ext);
        return (cr & odr) | (~cr & ext);
    endfunction

    task publish_result(gpio_seq_item tr,
                        logic [7:0] expected_cr,
                        logic [7:0] expected_odr,
                        logic [7:0] expected_idr,
                        logic [7:0] expected_io,
                        logic [7:0] actual_cr,
                        logic [7:0] actual_odr,
                        logic [7:0] actual_idr,
                        logic [7:0] actual_io,
                        logic [7:0] ext_en,
                        logic [7:0] ext_data);
        tr.completed             = 1'b1;
        tr.expected_cr           = expected_cr;
        tr.expected_odr          = expected_odr;
        tr.expected_idr          = expected_idr;
        tr.expected_io           = expected_io;
        tr.actual_cr             = actual_cr;
        tr.actual_odr            = actual_odr;
        tr.actual_idr            = actual_idr;
        tr.actual_io             = actual_io;
        tr.actual_ext_drive_en   = ext_en;
        tr.actual_ext_drive_data = ext_data;

        g_if.tr_id                 <= tr.tr_id;
        g_if.scenario_id           <= tr.scenario;
        g_if.mon_completed         <= 1'b1;
        g_if.mon_wstrb             <= tr.wstrb;
        g_if.mon_cr_expected       <= expected_cr;
        g_if.mon_odr_expected      <= expected_odr;
        g_if.mon_idr_expected      <= expected_idr;
        g_if.mon_io_expected       <= expected_io;
        g_if.mon_cr_actual         <= actual_cr;
        g_if.mon_odr_actual        <= actual_odr;
        g_if.mon_idr_actual        <= actual_idr;
        g_if.mon_io_actual         <= actual_io;
        g_if.mon_ext_drive_en      <= ext_en;
        g_if.mon_ext_drive_data    <= ext_data;
        @(posedge g_if.clk);
        g_if.mon_valid             <= 1'b1;
        @(posedge g_if.clk);
        g_if.mon_valid             <= 1'b0;
    endtask

    task read_gpio_regs(output bit [7:0] cr, output bit [7:0] idr, output bit [7:0] odr);
        bit [31:0] rdata;

        axi_read(4'h0, rdata);
        cr = rdata[7:0];
        axi_read(4'h4, rdata);
        idr = rdata[7:0];
        axi_read(4'h8, rdata);
        odr = rdata[7:0];
    endtask

    task drive_normal_case(gpio_seq_item tr, bit [7:0] cr, bit [7:0] odr, bit [7:0] ext_data);
        logic [7:0] actual_cr;
        logic [7:0] actual_odr;
        logic [7:0] actual_idr;
        logic [7:0] actual_io;
        logic [7:0] ext_en;
        logic [7:0] expected;

        ext_en = ~cr;
        g_if.ext_drive_data <= ext_data;
        g_if.ext_drive_en   <= ext_en;
        repeat (1) @(posedge g_if.clk);

        axi_write(4'h0, {24'h0, cr}, 4'b0001);
        axi_write(4'h8, {24'h0, odr}, 4'b0001);
        repeat (2) @(posedge g_if.clk);

        expected = expected_pin_value(cr, odr, ext_data);
        actual_io = g_if.io_sample;
        read_gpio_regs(actual_cr, actual_idr, actual_odr);

        publish_result(tr, cr, odr, expected, expected, actual_cr, actual_odr, actual_idr, actual_io, ext_en, ext_data);
    endtask

    task drive_partial_wstrb_case(gpio_seq_item tr);
        logic [7:0] actual_cr;
        logic [7:0] actual_odr;
        logic [7:0] actual_idr;
        logic [7:0] actual_io;
        logic [7:0] expected;
        logic [7:0] ext_en;

        axi_write(4'h0, {24'h0, tr.cr_value}, 4'b0001);
        axi_write(4'h8, {24'h0, tr.odr_value}, 4'b0001);

        axi_write(4'h0, 32'hff00_0000, tr.wstrb);
        axi_write(4'h8, 32'h00ff_0000, tr.wstrb);

        ext_en = ~tr.cr_value;
        g_if.ext_drive_data <= tr.ext_drive_data;
        g_if.ext_drive_en   <= ext_en;
        repeat (2) @(posedge g_if.clk);

        expected = expected_pin_value(tr.cr_value, tr.odr_value, tr.ext_drive_data);
        actual_io = g_if.io_sample;
        read_gpio_regs(actual_cr, actual_idr, actual_odr);
        publish_result(tr, tr.cr_value, tr.odr_value, expected, expected, actual_cr, actual_odr, actual_idr, actual_io, ext_en, tr.ext_drive_data);
    endtask

    task drive_idr_write_ignored_case(gpio_seq_item tr);
        logic [7:0] actual_cr;
        logic [7:0] actual_odr;
        logic [7:0] actual_idr;
        logic [7:0] actual_io;

        g_if.ext_drive_en   <= 8'hff;
        g_if.ext_drive_data <= tr.ext_drive_data;
        repeat (1) @(posedge g_if.clk);

        axi_write(4'h0, 32'h0000_0000, 4'b0001);
        axi_write(4'h8, {24'h0, tr.odr_value}, 4'b0001);
        axi_write(4'h4, 32'h0000_00ff, 4'b0001);
        repeat (2) @(posedge g_if.clk);

        actual_io = g_if.io_sample;
        read_gpio_regs(actual_cr, actual_idr, actual_odr);
        publish_result(tr, 8'h00, tr.odr_value, tr.ext_drive_data, tr.ext_drive_data,
                       actual_cr, actual_odr, actual_idr, actual_io, 8'hff, tr.ext_drive_data);
    endtask

    task drive_toggle_case(gpio_seq_item tr);
        bit [7:0] final_cr;
        bit [7:0] final_odr;
        bit [7:0] final_ext;

        drive_normal_case(tr, tr.cr_value, tr.odr_value, tr.ext_drive_data);

        final_cr  = tr.alt_cr_value;
        final_odr = tr.alt_odr_value;
        final_ext = tr.alt_ext_drive_data;
        drive_normal_case(tr, final_cr, final_odr, final_ext);
    endtask

    task drive_one(gpio_seq_item tr);
        wait (g_if.aresetn == 1'b1);
        repeat (tr.idle_cycles) @(posedge g_if.clk);

        tr.tr_id = next_tr_id++;
        g_if.tr_id       <= tr.tr_id;
        g_if.scenario_id <= tr.scenario;

        case (tr.scenario)
            GPIO_SCEN_RESET_DEFAULT: begin
                logic [7:0] actual_cr;
                logic [7:0] actual_odr;
                logic [7:0] actual_idr;
                logic [7:0] actual_io;

                apply_reset(tr.ext_drive_data);
                actual_io = g_if.io_sample;
                read_gpio_regs(actual_cr, actual_idr, actual_odr);
                publish_result(tr, 8'h00, 8'h00, tr.ext_drive_data, tr.ext_drive_data,
                               actual_cr, actual_odr, actual_idr, actual_io, 8'hff, tr.ext_drive_data);
            end

            GPIO_SCEN_INPUT_SAMPLE: begin
                drive_normal_case(tr, 8'h00, tr.odr_value, tr.ext_drive_data);
            end

            GPIO_SCEN_OUTPUT_DRIVE: begin
                drive_normal_case(tr, 8'hff, tr.odr_value, tr.ext_drive_data);
            end

            GPIO_SCEN_ODR_WHILE_INPUT: begin
                drive_normal_case(tr, 8'h00, tr.odr_value, tr.ext_drive_data);
            end

            GPIO_SCEN_PARTIAL_WSTRB: begin
                drive_partial_wstrb_case(tr);
            end

            GPIO_SCEN_IDR_WRITE_IGNORED: begin
                drive_idr_write_ignored_case(tr);
            end

            GPIO_SCEN_DIR_TOGGLE: begin
                drive_toggle_case(tr);
            end

            GPIO_SCEN_BACK_TO_BACK,
            GPIO_SCEN_RANDOM,
            GPIO_SCEN_MIXED_DIR: begin
                drive_normal_case(tr, tr.cr_value, tr.odr_value, tr.ext_drive_data);
            end

            default: begin
                drive_normal_case(tr, tr.cr_value, tr.odr_value, tr.ext_drive_data);
            end
        endcase

        `uvm_info(get_type_name(), $sformatf("driven GPIO item %s", tr.convert2string()), UVM_MEDIUM)
    endtask

    task run_phase(uvm_phase phase);
        init_bus();
        apply_reset(8'h00);

        forever begin
            seq_item_port.get_next_item(req);
            drive_one(req);
            seq_item_port.item_done();
        end
    endtask
endclass
