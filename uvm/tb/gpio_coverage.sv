class gpio_coverage extends uvm_subscriber #(gpio_seq_item);
    `uvm_component_utils(gpio_coverage)

    gpio_seq_item tr;

    covergroup gpio_cg;
        option.per_instance = 1;

        cp_scenario: coverpoint tr.scenario {
            bins reset_default = {GPIO_SCEN_RESET_DEFAULT};
            bins input_sample  = {GPIO_SCEN_INPUT_SAMPLE};
            bins output_drive  = {GPIO_SCEN_OUTPUT_DRIVE};
            bins mixed_dir     = {GPIO_SCEN_MIXED_DIR};
            bins odr_input     = {GPIO_SCEN_ODR_WHILE_INPUT};
            bins dir_toggle    = {GPIO_SCEN_DIR_TOGGLE};
            bins partial_wstrb = {GPIO_SCEN_PARTIAL_WSTRB};
            bins back_to_back  = {GPIO_SCEN_BACK_TO_BACK};
            bins idr_ignored   = {GPIO_SCEN_IDR_WRITE_IGNORED};
            bins random        = {GPIO_SCEN_RANDOM};
        }

        cp_direction: coverpoint tr.expected_cr {
            bins all_input  = {8'h00};
            bins all_output = {8'hff};
            bins low_out    = {8'h0f};
            bins high_out   = {8'hf0};
            bins alternate1 = {8'haa};
            bins alternate2 = {8'h55};
            bins mixed      = default;
        }

        cp_odr: coverpoint tr.expected_odr {
            bins zero = {8'h00};
            bins ones = {8'hff};
            bins aa   = {8'haa};
            bins h55  = {8'h55};
            bins low  = {[8'h01:8'h3f]};
            bins mid  = {[8'h40:8'hbf]};
            bins high = {[8'hc0:8'hfe]};
        }

        cp_idr: coverpoint tr.expected_idr {
            bins zero = {8'h00};
            bins ones = {8'hff};
            bins aa   = {8'haa};
            bins h55  = {8'h55};
            bins other = default;
        }

        cp_wstrb: coverpoint tr.wstrb {
            bins low_byte = {4'b0001};
            bins byte1    = {4'b0010};
            bins byte2    = {4'b0100};
            bins byte3    = {4'b1000};
            bins all      = {4'b1111};
            bins other    = default;
        }

        cp_completed: coverpoint tr.completed {
            bins completed = {1'b1};
            illegal_bins incomplete = {1'b0};
        }

    endgroup

    function new(string name, uvm_component parent);
        super.new(name, parent);
        gpio_cg = new();
    endfunction

    function void write(gpio_seq_item t);
        tr = t;
        gpio_cg.sample();
    endfunction

    function void report_phase(uvm_phase phase);
        super.report_phase(phase);
        `uvm_info("GPIO_COV", "====================================", UVM_LOW)
        `uvm_info("GPIO_COV", $sformatf("gpio coverage = %.2f%%", gpio_cg.get_inst_coverage()), UVM_LOW)
        `uvm_info("GPIO_COV", $sformatf("scenario      = %.2f%%", gpio_cg.cp_scenario.get_inst_coverage()), UVM_LOW)
        `uvm_info("GPIO_COV", $sformatf("direction     = %.2f%%", gpio_cg.cp_direction.get_inst_coverage()), UVM_LOW)
        `uvm_info("GPIO_COV", $sformatf("odr           = %.2f%%", gpio_cg.cp_odr.get_inst_coverage()), UVM_LOW)
        `uvm_info("GPIO_COV", $sformatf("idr           = %.2f%%", gpio_cg.cp_idr.get_inst_coverage()), UVM_LOW)
        `uvm_info("GPIO_COV", $sformatf("wstrb         = %.2f%%", gpio_cg.cp_wstrb.get_inst_coverage()), UVM_LOW)
        `uvm_info("GPIO_COV", "====================================", UVM_LOW)
    endfunction
endclass
