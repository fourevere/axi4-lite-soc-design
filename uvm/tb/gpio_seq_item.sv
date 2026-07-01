typedef enum int unsigned {
    GPIO_SCEN_RESET_DEFAULT  = 0,
    GPIO_SCEN_INPUT_SAMPLE   = 1,
    GPIO_SCEN_OUTPUT_DRIVE   = 2,
    GPIO_SCEN_MIXED_DIR      = 3,
    GPIO_SCEN_ODR_WHILE_INPUT = 4,
    GPIO_SCEN_DIR_TOGGLE     = 5,
    GPIO_SCEN_PARTIAL_WSTRB  = 6,
    GPIO_SCEN_BACK_TO_BACK   = 7,
    GPIO_SCEN_IDR_WRITE_IGNORED = 8,
    GPIO_SCEN_RANDOM         = 9
} gpio_scenario_e;

class gpio_seq_item extends uvm_sequence_item;
    rand gpio_scenario_e scenario;
    rand bit [7:0]       cr_value;
    rand bit [7:0]       odr_value;
    rand bit [7:0]       ext_drive_data;
    rand bit [7:0]       alt_cr_value;
    rand bit [7:0]       alt_odr_value;
    rand bit [7:0]       alt_ext_drive_data;
    rand bit [3:0]       wstrb;
    rand int unsigned    idle_cycles;

    int unsigned         tr_id;
    bit                  completed;
    logic [7:0]          expected_cr;
    logic [7:0]          expected_odr;
    logic [7:0]          expected_idr;
    logic [7:0]          expected_io;
    logic [7:0]          actual_cr;
    logic [7:0]          actual_odr;
    logic [7:0]          actual_idr;
    logic [7:0]          actual_io;
    logic [7:0]          actual_ext_drive_en;
    logic [7:0]          actual_ext_drive_data;

    constraint c_scenario {
        scenario dist {
            GPIO_SCEN_RESET_DEFAULT   := 4,
            GPIO_SCEN_INPUT_SAMPLE    := 15,
            GPIO_SCEN_OUTPUT_DRIVE    := 15,
            GPIO_SCEN_MIXED_DIR       := 25,
            GPIO_SCEN_ODR_WHILE_INPUT := 10,
            GPIO_SCEN_DIR_TOGGLE      := 10,
            GPIO_SCEN_PARTIAL_WSTRB   := 8,
            GPIO_SCEN_BACK_TO_BACK    := 8,
            GPIO_SCEN_IDR_WRITE_IGNORED := 6,
            GPIO_SCEN_RANDOM          := 20
        };
    }

    constraint c_idle {
        idle_cycles inside {[0:5]};
    }

    constraint c_wstrb {
        wstrb dist {
            4'b0001 := 12,
            4'b0011 := 2,
            4'b0010 := 2,
            4'b0100 := 2,
            4'b1111 := 4,
            4'b1000 := 1
        };
    }

    constraint c_partial_wstrb {
        if (scenario == GPIO_SCEN_PARTIAL_WSTRB) {
            wstrb inside {4'b0010, 4'b0100, 4'b1000};
        }
    }

    `uvm_object_utils_begin(gpio_seq_item)
        `uvm_field_enum(gpio_scenario_e, scenario, UVM_ALL_ON)
        `uvm_field_int(cr_value,              UVM_ALL_ON)
        `uvm_field_int(odr_value,             UVM_ALL_ON)
        `uvm_field_int(ext_drive_data,        UVM_ALL_ON)
        `uvm_field_int(alt_cr_value,          UVM_ALL_ON)
        `uvm_field_int(alt_odr_value,         UVM_ALL_ON)
        `uvm_field_int(alt_ext_drive_data,    UVM_ALL_ON)
        `uvm_field_int(wstrb,                 UVM_ALL_ON)
        `uvm_field_int(idle_cycles,           UVM_ALL_ON)
        `uvm_field_int(tr_id,                 UVM_ALL_ON)
        `uvm_field_int(completed,             UVM_ALL_ON)
        `uvm_field_int(expected_cr,           UVM_ALL_ON)
        `uvm_field_int(expected_odr,          UVM_ALL_ON)
        `uvm_field_int(expected_idr,          UVM_ALL_ON)
        `uvm_field_int(expected_io,           UVM_ALL_ON)
        `uvm_field_int(actual_cr,             UVM_ALL_ON)
        `uvm_field_int(actual_odr,            UVM_ALL_ON)
        `uvm_field_int(actual_idr,            UVM_ALL_ON)
        `uvm_field_int(actual_io,             UVM_ALL_ON)
        `uvm_field_int(actual_ext_drive_en,   UVM_ALL_ON)
        `uvm_field_int(actual_ext_drive_data, UVM_ALL_ON)
    `uvm_object_utils_end

    function new(string name = "gpio_seq_item");
        super.new(name);
        scenario = GPIO_SCEN_MIXED_DIR;
        wstrb = 4'b0001;
    endfunction

    function string scenario_name();
        case (scenario)
            GPIO_SCEN_RESET_DEFAULT   : return "reset_default";
            GPIO_SCEN_INPUT_SAMPLE    : return "input_sample";
            GPIO_SCEN_OUTPUT_DRIVE    : return "output_drive";
            GPIO_SCEN_MIXED_DIR       : return "mixed_dir";
            GPIO_SCEN_ODR_WHILE_INPUT : return "odr_while_input";
            GPIO_SCEN_DIR_TOGGLE      : return "dir_toggle";
            GPIO_SCEN_PARTIAL_WSTRB   : return "partial_wstrb";
            GPIO_SCEN_BACK_TO_BACK    : return "back_to_back";
            GPIO_SCEN_IDR_WRITE_IGNORED : return "idr_write_ignored";
            GPIO_SCEN_RANDOM          : return "random";
            default                   : return "unknown";
        endcase
    endfunction

    function string convert2string();
        return $sformatf("id=%0d scen=%s cr=0x%02h odr=0x%02h ext=0x%02h exp(cr/odr/idr/io)=0x%02h/0x%02h/0x%02h/0x%02h act=0x%02h/0x%02h/0x%02h/0x%02h",
                         tr_id, scenario_name(), cr_value, odr_value, ext_drive_data,
                         expected_cr, expected_odr, expected_idr, expected_io,
                         actual_cr, actual_odr, actual_idr, actual_io);
    endfunction
endclass
