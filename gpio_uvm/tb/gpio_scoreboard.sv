class gpio_scoreboard extends uvm_scoreboard;
    `uvm_component_utils(gpio_scoreboard)

    uvm_analysis_imp #(gpio_seq_item, gpio_scoreboard) imp;
    int pass_count;
    int fail_count;

    function new(string name, uvm_component parent);
        super.new(name, parent);
        imp = new("imp", this);
    endfunction

    function void write(gpio_seq_item tr);
        bit pass;

        pass = 1'b1;

        if (!tr.completed) begin
            pass = 1'b0;
            `uvm_error(get_type_name(), $sformatf("GPIO transaction incomplete: %s", tr.convert2string()))
        end

        if (tr.actual_cr !== tr.expected_cr) begin
            pass = 1'b0;
            `uvm_error(get_type_name(), $sformatf("CR mismatch exp=0x%02h act=0x%02h %s",
                                                  tr.expected_cr, tr.actual_cr, tr.convert2string()))
        end

        if (tr.actual_odr !== tr.expected_odr) begin
            pass = 1'b0;
            `uvm_error(get_type_name(), $sformatf("ODR mismatch exp=0x%02h act=0x%02h %s",
                                                  tr.expected_odr, tr.actual_odr, tr.convert2string()))
        end

        if (tr.actual_idr !== tr.expected_idr) begin
            pass = 1'b0;
            `uvm_error(get_type_name(), $sformatf("IDR mismatch exp=0x%02h act=0x%02h %s",
                                                  tr.expected_idr, tr.actual_idr, tr.convert2string()))
        end

        if (tr.actual_io !== tr.expected_io) begin
            pass = 1'b0;
            `uvm_error(get_type_name(), $sformatf("io_port mismatch exp=0x%02h act=0x%02h ext_en=0x%02h ext_data=0x%02h %s",
                                                  tr.expected_io, tr.actual_io,
                                                  tr.actual_ext_drive_en, tr.actual_ext_drive_data,
                                                  tr.convert2string()))
        end

        if (pass) begin
            pass_count++;
            `uvm_info(get_type_name(), $sformatf("PASS %s", tr.convert2string()), UVM_LOW)
        end else begin
            fail_count++;
        end
    endfunction

    function void check_phase(uvm_phase phase);
        super.check_phase(phase);
        if (pass_count == 0) begin
            `uvm_error(get_type_name(), "no GPIO transactions were checked")
        end
        if (fail_count != 0) begin
            `uvm_error(get_type_name(), $sformatf("scoreboard observed %0d failures", fail_count))
        end
    endfunction

    function void report_phase(uvm_phase phase);
        super.report_phase(phase);
        `uvm_info("GPIO_SCB", "====================================", UVM_LOW)
        `uvm_info("GPIO_SCB", $sformatf("pass_count = %0d", pass_count), UVM_LOW)
        `uvm_info("GPIO_SCB", $sformatf("fail_count = %0d", fail_count), UVM_LOW)
        `uvm_info("GPIO_SCB", "====================================", UVM_LOW)
    endfunction
endclass
