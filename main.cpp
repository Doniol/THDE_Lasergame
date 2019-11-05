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
    auto in0 = hwlib::target::pin_oc(hwlib::target::pins::a4);
    auto in1 = hwlib::target::pin_oc(hwlib::target::pins::a5);
    auto in2 = hwlib::target::pin_oc(hwlib::target::pins::a6);
    auto in3 = hwlib::target::pin_oc(hwlib::target::pins::a7);

    auto out_port = hwlib::port_oc_from(out0, out1, out2, out3);
    auto in_port = hwlib::port_oc_from(in0, in1, in2, in3);
    auto matrix = hwlib::matrix_of_switches(out_port, in_port);

    auto keypad = hwlib::keypad<16>(matrix, "147*369#2580ABCD");
    auto button = hwlib::target::pin_in(hwlib::target::pins::d13);

    auto scl = target::pin_oc(target::pins::scl);
    auto sda = target::pin_oc(target::pins::sda);
    auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);
    auto oled = hwlib::glcd_oled(i2c_bus, 0x3c);


    auto init_game;
    auto run_game_taak;
    auto game_parameters_taak;
    auto transfer_hits;
    auto display = display_taak(oled);
    auto input = invoer_taak(keypad, button, init_game, run_game, game_parameters, transfer_hits);
    auto decoder = decoder_taak(game_parameters_taak, run_game_taak);
    auto pulse_meter = pulse_meter_taak(decoder, tsop_signal, tsop_gnd, tsop_vdd);

    rtos::run();
}