class gpio_monitor extends uvm_monitor;
    `uvm_component_utils(gpio_monitor)

    virtual gpio_if g_if;
    uvm_analysis_port #(gpio_seq_item) ap;

    function new(string name, uvm_component parent);
        super.new(name, parent);
        ap = new("ap", this);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if (!uvm_config_db #(virtual gpio_if)::get(this, "", "g_if", g_if)) begin
            `uvm_fatal(get_type_name(), "virtual interface g_if not found")
        end
    endfunction

    task protocol_watchdog();
        forever begin
            @(posedge g_if.clk);
            if (!g_if.aresetn) begin
                if (g_if.awvalid || g_if.wvalid || g_if.arvalid) begin
                    `uvm_warning(get_type_name(), "AXI valid asserted during reset")
                end
            end
            if (g_if.bvalid && (g_if.bresp != 2'b00)) begin
                `uvm_error(get_type_name(), $sformatf("AXI BRESP is not OKAY: %0b", g_if.bresp))
            end
            if (g_if.rvalid && (g_if.rresp != 2'b00)) begin
                `uvm_error(get_type_name(), $sformatf("AXI RRESP is not OKAY: %0b", g_if.rresp))
            end
        end
    endtask

    task collect_results();
        gpio_seq_item tr;

        forever begin
            @(posedge g_if.clk);
            if (g_if.mon_valid) begin
                tr = gpio_seq_item::type_id::create("tr");
                tr.tr_id                 = g_if.tr_id;
                tr.scenario              = gpio_scenario_e'(g_if.scenario_id);
                tr.completed             = g_if.mon_completed;
                tr.wstrb                 = g_if.mon_wstrb;
                tr.expected_cr           = g_if.mon_cr_expected;
                tr.expected_odr          = g_if.mon_odr_expected;
                tr.expected_idr          = g_if.mon_idr_expected;
                tr.expected_io           = g_if.mon_io_expected;
                tr.actual_cr             = g_if.mon_cr_actual;
                tr.actual_odr            = g_if.mon_odr_actual;
                tr.actual_idr            = g_if.mon_idr_actual;
                tr.actual_io             = g_if.mon_io_actual;
                tr.actual_ext_drive_en   = g_if.mon_ext_drive_en;
                tr.actual_ext_drive_data = g_if.mon_ext_drive_data;
                ap.write(tr);
            end
        end
    endtask

    task run_phase(uvm_phase phase);
        fork
            protocol_watchdog();
            collect_results();
        join
    endtask
endclass
