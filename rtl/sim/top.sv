module top(
    input wire logic clk,
    input wire logic reset_i,

    input wire logic [1:0]  op,

    input wire logic [31:0] a_value_i,
    output logic     [31:0] z_value_o,

    input wire logic        exec_strobe_i,
    output logic            done_strobe_o
    );

    logic [31:0] z_op0, z_op1;
    logic done_op0, done_op1;

    float_to_int float_to_int(
        .clk(clk),
        .reset_i(reset_i),
        .a_value_i(a_value_i),
        .z_value_o(z_op0),
        .exec_strobe_i(op == 0 && exec_strobe_i),
        .done_strobe_o(done_op0)
    );

    int_to_float int_to_float(
        .clk(clk),
        .reset_i(reset_i),
        .a_value_i(a_value_i),
        .z_value_o(z_op1),
        .exec_strobe_i(op == 1 && exec_strobe_i),
        .done_strobe_o(done_op1)
    );

    always_comb begin
        z_value_o = op == 1 ? z_op1 : z_op0;
        done_strobe_o = done_op0 | done_op1;
    end

endmodule