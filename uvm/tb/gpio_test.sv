class gpio_base_test extends uvm_test;
    `uvm_component_utils(gpio_base_test)

    gpio_env env;

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        env = gpio_env::type_id::create("env", this);
    endfunction

    function void end_of_elaboration_phase(uvm_phase phase);
        super.end_of_elaboration_phase(phase);
        uvm_top.print_topology();
    endfunction
endclass

class gpio_basic_test extends gpio_base_test;
    `uvm_component_utils(gpio_basic_test)

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    task run_phase(uvm_phase phase);
        gpio_mixed_dir_seq seq;

        phase.raise_objection(this);
        seq = gpio_mixed_dir_seq::type_id::create("seq");
        seq.start(env.agt.sqr);
        #100;
        phase.drop_objection(this);
    endtask
endclass

class gpio_directed_test extends gpio_base_test;
    `uvm_component_utils(gpio_directed_test)

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    task run_phase(uvm_phase phase);
        gpio_full_regression_seq seq;

        phase.raise_objection(this);
        seq = gpio_full_regression_seq::type_id::create("seq");
        seq.start(env.agt.sqr);
        #1000;
        phase.drop_objection(this);
    endtask
endclass

class gpio_random_test extends gpio_base_test;
    `uvm_component_utils(gpio_random_test)

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    task run_phase(uvm_phase phase);
        gpio_random_seq seq;

        phase.raise_objection(this);
        seq = gpio_random_seq::type_id::create("seq");
        if (!seq.randomize() with { num == 300; }) begin
            `uvm_error("TEST", "gpio_random_seq randomize fail")
            seq.num = 300;
        end
        seq.start(env.agt.sqr);
        #1000;
        phase.drop_objection(this);
    endtask
endclass

class gpio_full_regression_test extends gpio_base_test;
    `uvm_component_utils(gpio_full_regression_test)

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    task run_phase(uvm_phase phase);
        gpio_full_regression_seq seq;

        phase.raise_objection(this);
        seq = gpio_full_regression_seq::type_id::create("seq");
        seq.start(env.agt.sqr);
        #2000;
        phase.drop_objection(this);
    endtask
endclass
