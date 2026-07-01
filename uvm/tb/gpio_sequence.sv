class gpio_base_seq extends uvm_sequence #(gpio_seq_item);
    `uvm_object_utils(gpio_base_seq)

    function new(string name = "gpio_base_seq");
        super.new(name);
    endfunction

    task send_gpio(gpio_scenario_e scenario,
                   bit [7:0] cr_value,
                   bit [7:0] odr_value,
                   bit [7:0] ext_drive_data,
                   bit [7:0] alt_cr_value = 8'h00,
                   bit [7:0] alt_odr_value = 8'h00,
                   bit [7:0] alt_ext_drive_data = 8'h00,
                   bit [3:0] wstrb = 4'b0001,
                   int unsigned idle_cycles = 0);
        gpio_seq_item item;

        item = gpio_seq_item::type_id::create("item");
        start_item(item);
        item.scenario           = scenario;
        item.cr_value           = cr_value;
        item.odr_value          = odr_value;
        item.ext_drive_data     = ext_drive_data;
        item.alt_cr_value       = alt_cr_value;
        item.alt_odr_value      = alt_odr_value;
        item.alt_ext_drive_data = alt_ext_drive_data;
        item.wstrb              = wstrb;
        item.idle_cycles        = idle_cycles;
        finish_item(item);
    endtask
endclass

class gpio_input_sample_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_input_sample_seq)

    function new(string name = "gpio_input_sample_seq");
        super.new(name);
    endfunction

    task body();
        bit [7:0] patterns[8] = '{8'h00, 8'hff, 8'haa, 8'h55, 8'h80, 8'h01, 8'h7e, 8'h81};

        foreach (patterns[i]) begin
            send_gpio(GPIO_SCEN_INPUT_SAMPLE, 8'h00, 8'h00, patterns[i], 8'h00, 8'h00, 8'h00, 4'b0001, i);
        end
    endtask
endclass

class gpio_output_drive_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_output_drive_seq)

    function new(string name = "gpio_output_drive_seq");
        super.new(name);
    endfunction

    task body();
        bit [7:0] patterns[8] = '{8'h00, 8'hff, 8'haa, 8'h55, 8'h80, 8'h01, 8'h7e, 8'h81};

        foreach (patterns[i]) begin
            send_gpio(GPIO_SCEN_OUTPUT_DRIVE, 8'hff, patterns[i], 8'h00, 8'h00, 8'h00, 8'h00, 4'b0001, 0);
        end
    endtask
endclass

class gpio_mixed_dir_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_mixed_dir_seq)

    function new(string name = "gpio_mixed_dir_seq");
        super.new(name);
    endfunction

    task body();
        bit [7:0] masks[8] = '{8'h0f, 8'hf0, 8'haa, 8'h55, 8'h81, 8'h7e, 8'h18, 8'he7};

        foreach (masks[i]) begin
            send_gpio(GPIO_SCEN_MIXED_DIR,
                      masks[i],
                      8'ha5 ^ masks[i],
                      8'h5a ^ masks[i],
                      8'h00,
                      8'h00,
                      8'h00,
                      4'b0001,
                      i);
        end
    endtask
endclass

class gpio_odr_while_input_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_odr_while_input_seq)

    function new(string name = "gpio_odr_while_input_seq");
        super.new(name);
    endfunction

    task body();
        for (int i = 0; i < 8; i++) begin
            send_gpio(GPIO_SCEN_ODR_WHILE_INPUT,
                      8'h00,
                      8'h11 << i,
                      8'hee >> i,
                      8'h00,
                      8'h00,
                      8'h00,
                      4'b0001,
                      1);
        end
    endtask
endclass

class gpio_dir_toggle_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_dir_toggle_seq)

    function new(string name = "gpio_dir_toggle_seq");
        super.new(name);
    endfunction

    task body();
        send_gpio(GPIO_SCEN_DIR_TOGGLE, 8'h00, 8'h3c, 8'hc3, 8'hff, 8'ha5, 8'h5a, 4'b0001, 1);
        send_gpio(GPIO_SCEN_DIR_TOGGLE, 8'haa, 8'h96, 8'h69, 8'h55, 8'hf0, 8'h0f, 4'b0001, 1);
        send_gpio(GPIO_SCEN_DIR_TOGGLE, 8'hf0, 8'h0f, 8'h55, 8'h0f, 8'hf0, 8'haa, 4'b0001, 1);
    endtask
endclass

class gpio_partial_wstrb_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_partial_wstrb_seq)

    function new(string name = "gpio_partial_wstrb_seq");
        super.new(name);
    endfunction

    task body();
        send_gpio(GPIO_SCEN_PARTIAL_WSTRB, 8'h55, 8'haa, 8'h33, 8'h00, 8'h00, 8'h00, 4'b1000, 0);
        send_gpio(GPIO_SCEN_PARTIAL_WSTRB, 8'haa, 8'h55, 8'hcc, 8'h00, 8'h00, 8'h00, 4'b0100, 0);
        send_gpio(GPIO_SCEN_PARTIAL_WSTRB, 8'h0f, 8'hf0, 8'h99, 8'h00, 8'h00, 8'h00, 4'b0010, 0);
    endtask
endclass

class gpio_back_to_back_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_back_to_back_seq)

    function new(string name = "gpio_back_to_back_seq");
        super.new(name);
    endfunction

    task body();
        for (int i = 0; i < 12; i++) begin
            bit [7:0] v;
            v = i;
            send_gpio(GPIO_SCEN_BACK_TO_BACK,
                      v ^ 8'haa,
                      v * 8'h13,
                      ~v,
                      8'h00,
                      8'h00,
                      8'h00,
                      4'b0001,
                      0);
        end
    endtask
endclass

class gpio_idr_write_ignored_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_idr_write_ignored_seq)

    function new(string name = "gpio_idr_write_ignored_seq");
        super.new(name);
    endfunction

    task body();
        send_gpio(GPIO_SCEN_IDR_WRITE_IGNORED, 8'h00, 8'h00, 8'ha5, 8'h00, 8'h00, 8'h00, 4'b0001, 0);
        send_gpio(GPIO_SCEN_IDR_WRITE_IGNORED, 8'h00, 8'hff, 8'h5a, 8'h00, 8'h00, 8'h00, 4'b0001, 0);
    endtask
endclass

class gpio_random_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_random_seq)

    rand int unsigned num;

    constraint c_num {
        num inside {[50:300]};
    }

    function new(string name = "gpio_random_seq");
        super.new(name);
        num = 100;
    endfunction

    task body();
        gpio_seq_item item;

        repeat (num) begin
            item = gpio_seq_item::type_id::create("item");
            start_item(item);
            if (!item.randomize()) begin
                `uvm_error("SEQ", "gpio random item randomize fail")
            end
            finish_item(item);
        end
    endtask
endclass

class gpio_full_regression_seq extends gpio_base_seq;
    `uvm_object_utils(gpio_full_regression_seq)

    function new(string name = "gpio_full_regression_seq");
        super.new(name);
    endfunction

    task body();
        gpio_input_sample_seq     input_seq;
        gpio_output_drive_seq     output_seq;
        gpio_mixed_dir_seq        mixed_seq;
        gpio_odr_while_input_seq  odr_input_seq;
        gpio_dir_toggle_seq       toggle_seq;
        gpio_partial_wstrb_seq    wstrb_seq;
        gpio_back_to_back_seq     b2b_seq;
        gpio_idr_write_ignored_seq idr_seq;
        gpio_random_seq           random_seq;

        send_gpio(GPIO_SCEN_RESET_DEFAULT, 8'h00, 8'h00, 8'ha5);

        input_seq     = gpio_input_sample_seq::type_id::create("input_seq");
        output_seq    = gpio_output_drive_seq::type_id::create("output_seq");
        mixed_seq     = gpio_mixed_dir_seq::type_id::create("mixed_seq");
        odr_input_seq = gpio_odr_while_input_seq::type_id::create("odr_input_seq");
        toggle_seq    = gpio_dir_toggle_seq::type_id::create("toggle_seq");
        wstrb_seq     = gpio_partial_wstrb_seq::type_id::create("wstrb_seq");
        b2b_seq       = gpio_back_to_back_seq::type_id::create("b2b_seq");
        idr_seq       = gpio_idr_write_ignored_seq::type_id::create("idr_seq");
        random_seq    = gpio_random_seq::type_id::create("random_seq");

        input_seq.start(m_sequencer);
        output_seq.start(m_sequencer);
        mixed_seq.start(m_sequencer);
        odr_input_seq.start(m_sequencer);
        toggle_seq.start(m_sequencer);
        wstrb_seq.start(m_sequencer);
        b2b_seq.start(m_sequencer);
        idr_seq.start(m_sequencer);

        send_gpio(GPIO_SCEN_RANDOM, 8'hff, 8'haa, 8'h55, 8'h00, 8'h00, 8'h00, 4'b1111, 0);
        send_gpio(GPIO_SCEN_RANDOM, 8'h00, 8'h55, 8'haa, 8'h00, 8'h00, 8'h00, 4'b0011, 0);

        if (!random_seq.randomize() with { num == 150; }) begin
            `uvm_error("SEQ", "gpio_random_seq randomize fail")
            random_seq.num = 150;
        end
        random_seq.start(m_sequencer);
    endtask
endclass
