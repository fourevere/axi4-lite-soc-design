`timescale 1ns / 1ps

// Synthesizable I2C byte-write master for a PCF8574 LCD backpack.
// The core drives open-drain intent through *_oen signals:
//   0 = drive low, 1 = release/high-Z at the top level.
module I2C_Master_top #(
    parameter [15:0] CLK_DIV_DEFAULT = 16'd250
)(
    input  logic       clk,
    input  logic       rst_n,
    input  logic       start,
    input  logic       stop_enable,
    input  logic [6:0] slave_addr,
    input  logic [7:0] tx_data,
    input  logic [15:0] clk_div,
    output logic       busy,
    output logic       done,
    output logic       ack_error,
    output logic       scl_oen,
    output logic       sda_oen,
    inout  wire        scl,
    inout  wire        sda
);

    wire sda_i;

    assign scl   = scl_oen ? 1'bz : 1'b0;
    assign sda   = sda_oen ? 1'bz : 1'b0;
    assign sda_i = sda;

    I2C_Master #(
        .CLK_DIV_DEFAULT(CLK_DIV_DEFAULT)
    ) u_i2c_master (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start),
        .stop_enable(stop_enable),
        .slave_addr (slave_addr),
        .tx_data    (tx_data),
        .clk_div    (clk_div),
        .sda_i      (sda_i),
        .busy       (busy),
        .done       (done),
        .ack_error  (ack_error),
        .scl_oen    (scl_oen),
        .sda_oen    (sda_oen)
    );

endmodule

module I2C_Master #(
    parameter [15:0] CLK_DIV_DEFAULT = 16'd250
)(
    input  logic       clk,
    input  logic       rst_n,
    input  logic       start,
    input  logic       stop_enable,
    input  logic [6:0] slave_addr,
    input  logic [7:0] tx_data,
    input  logic [15:0] clk_div,
    input  logic       sda_i,
    output logic       busy,
    output logic       done,
    output logic       ack_error,
    output logic       scl_oen,
    output logic       sda_oen
);

    typedef enum logic [3:0] {
        ST_IDLE      = 4'd0,
        ST_START_A   = 4'd1,
        ST_START_B   = 4'd2,
        ST_START_C   = 4'd3,
        ST_WRITE_BIT = 4'd4,
        ST_ACK       = 4'd5,
        ST_STOP_A    = 4'd6,
        ST_STOP_B    = 4'd7,
        ST_STOP_C    = 4'd8,
        ST_DONE      = 4'd9
    } i2c_state_e;

    typedef enum logic [0:0] {
        BYTE_ADDR = 1'b0,
        BYTE_DATA = 1'b1
    } byte_phase_e;

    i2c_state_e  state;
    byte_phase_e phase;

    logic [15:0] div_cnt;
    logic        qtr_tick;
    logic [1:0]  step;
    logic [7:0]  shift_reg;
    logic [2:0]  bit_cnt;
    logic        ack_sample;

    wire [15:0] div_target = (clk_div == 16'd0) ? CLK_DIV_DEFAULT : clk_div;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            div_cnt  <= 16'd0;
            qtr_tick <= 1'b0;
        end
        else begin
            qtr_tick <= 1'b0;
            if (!busy) begin
                div_cnt <= 16'd0;
            end
            else if (div_cnt >= (div_target - 16'd1)) begin
                div_cnt  <= 16'd0;
                qtr_tick <= 1'b1;
            end
            else begin
                div_cnt <= div_cnt + 16'd1;
            end
        end
    end

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state      <= ST_IDLE;
            phase      <= BYTE_ADDR;
            step       <= 2'd0;
            shift_reg  <= 8'd0;
            bit_cnt    <= 3'd7;
            ack_sample <= 1'b1;
            busy       <= 1'b0;
            done       <= 1'b0;
            ack_error  <= 1'b0;
            scl_oen    <= 1'b1;
            sda_oen    <= 1'b1;
        end
        else begin
            done <= 1'b0;

            case (state)
                ST_IDLE: begin
                    busy    <= 1'b0;
                    scl_oen <= 1'b1;
                    sda_oen <= 1'b1;
                    step    <= 2'd0;
                    if (start) begin
                        busy      <= 1'b1;
                        ack_error <= 1'b0;
                        phase     <= BYTE_ADDR;
                        shift_reg <= {slave_addr, 1'b0};
                        bit_cnt   <= 3'd7;
                        state     <= ST_START_A;
                    end
                end

                ST_START_A: begin
                    if (qtr_tick) begin
                        scl_oen <= 1'b1;
                        sda_oen <= 1'b1;
                        state   <= ST_START_B;
                    end
                end

                ST_START_B: begin
                    if (qtr_tick) begin
                        scl_oen <= 1'b1;
                        sda_oen <= 1'b0;
                        state   <= ST_START_C;
                    end
                end

                ST_START_C: begin
                    if (qtr_tick) begin
                        scl_oen <= 1'b0;
                        sda_oen <= 1'b0;
                        step    <= 2'd0;
                        state   <= ST_WRITE_BIT;
                    end
                end

                ST_WRITE_BIT: begin
                    if (qtr_tick) begin
                        case (step)
                            2'd0: begin
                                scl_oen <= 1'b0;
                                sda_oen <= shift_reg[7] ? 1'b1 : 1'b0;
                                step    <= 2'd1;
                            end
                            2'd1: begin
                                scl_oen <= 1'b1;
                                step    <= 2'd2;
                            end
                            2'd2: begin
                                scl_oen <= 1'b1;
                                step    <= 2'd3;
                            end
                            default: begin
                                scl_oen   <= 1'b0;
                                shift_reg <= {shift_reg[6:0], 1'b0};
                                step      <= 2'd0;
                                if (bit_cnt == 3'd0) begin
                                    state <= ST_ACK;
                                end
                                else begin
                                    bit_cnt <= bit_cnt - 3'd1;
                                end
                            end
                        endcase
                    end
                end

                ST_ACK: begin
                    if (qtr_tick) begin
                        case (step)
                            2'd0: begin
                                scl_oen <= 1'b0;
                                sda_oen <= 1'b1;
                                step    <= 2'd1;
                            end
                            2'd1: begin
                                scl_oen <= 1'b1;
                                step    <= 2'd2;
                            end
                            2'd2: begin
                                scl_oen    <= 1'b1;
                                ack_sample <= sda_i;
                                if (sda_i) begin
                                    ack_error <= 1'b1;
                                end
                                step <= 2'd3;
                            end
                            default: begin
                                scl_oen <= 1'b0;
                                step    <= 2'd0;
                                if (ack_sample || ack_error) begin
                                    state <= stop_enable ? ST_STOP_A : ST_DONE;
                                end
                                else if (phase == BYTE_ADDR) begin
                                    phase     <= BYTE_DATA;
                                    shift_reg <= tx_data;
                                    bit_cnt   <= 3'd7;
                                    state     <= ST_WRITE_BIT;
                                end
                                else begin
                                    state <= stop_enable ? ST_STOP_A : ST_DONE;
                                end
                            end
                        endcase
                    end
                end

                ST_STOP_A: begin
                    if (qtr_tick) begin
                        scl_oen <= 1'b0;
                        sda_oen <= 1'b0;
                        state   <= ST_STOP_B;
                    end
                end

                ST_STOP_B: begin
                    if (qtr_tick) begin
                        scl_oen <= 1'b1;
                        sda_oen <= 1'b0;
                        state   <= ST_STOP_C;
                    end
                end

                ST_STOP_C: begin
                    if (qtr_tick) begin
                        scl_oen <= 1'b1;
                        sda_oen <= 1'b1;
                        state   <= ST_DONE;
                    end
                end

                ST_DONE: begin
                    done  <= 1'b1;
                    busy  <= 1'b0;
                    state <= ST_IDLE;
                end

                default: begin
                    state <= ST_IDLE;
                end
            endcase
        end
    end

endmodule
