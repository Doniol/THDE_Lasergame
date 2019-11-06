#include "IR.hpp"

int main(){
    hwlib::wait_ms(500);

    auto tsop_signal = hwlib::target::pin_in(hwlib::target::pins::d8);
    auto tsop_gnd    = hwlib::target::pin_out(hwlib::target::pins::d9);
    auto tsop_vdd    = hwlib::target::pin_out(hwlib::target::pins::d10);

    auto out0 = hwlib::target::pin_oc(hwlib::target::pins::a0);
    auto out1 = hwlib::target::pin_oc(hwlib::target::pins::a1);
    auto out2 = hwlib::target::pin_oc(hwlib::target::pins::a2);
    auto out3 = hwlib::target::pin_oc(hwlib::target::pins::a3);
    auto in0 = hwlib::target::pin_in(hwlib::target::pins::a4);
    auto in1 = hwlib::target::pin_in(hwlib::target::pins::a5);
    auto in2 = hwlib::target::pin_in(hwlib::target::pins::a6);
    auto in3 = hwlib::target::pin_in(hwlib::target::pins::a7);

    auto out_port = hwlib::port_oc_from(out0, out1, out2, out3);
    auto in_port = hwlib::port_in_from(in0, in1, in2, in3);
    auto matrix = hwlib::matrix_of_switches(out_port, in_port);

    auto keypad = hwlib::keypad<16>(matrix, "147*2580369#ABCD");
    auto button = hwlib::target::pin_in(hwlib::target::pins::d13);

    auto scl = hwlib::target::pin_oc(hwlib::target::pins::scl);
    auto sda = hwlib::target::pin_oc(hwlib::target::pins::sda);
    auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);
    auto oled = hwlib::glcd_oled(i2c_bus, 0x3c);


    auto display = display_taak(oled);
    auto send = send_taak();
    auto init_game = init_game_taak(send, display);
    auto transfer_hits = transfer_hits_taak();
    auto game_parameters = game_parameters_taak();
    auto run_game = run_game_taak(send, game_parameters, init_game, display, transfer_hits);
    game_parameters.add_listener(&run_game);
    auto decoder = decoder_taak(game_parameters, run_game);
    auto pulse_meter = pulse_meter_taak(decoder, tsop_signal, tsop_gnd, tsop_vdd);
    auto input = invoer_taak(keypad, button, init_game, run_game, game_parameters, transfer_hits);

    (void)display;
    (void)transfer_hits;
    (void)input;
    (void)init_game;
    (void)game_parameters;
    (void)run_game;
    (void)decoder;
    (void)send;
    (void)pulse_meter;

    rtos::run();
}