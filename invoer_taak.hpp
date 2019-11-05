#ifndef INVOER_TAAK_HPP
#define INVOER_TAAK_HPP

#include "IR.hpp"

class invoer_listener{
public:
    virtual void button_pressed(char button_id) = 0;
};

class invoer_taak : public rtos::task<>{
private:
    hwlib::keypad<16> keypad;
    hwlib::target::pin_in button;
    rtos::clock ms100_clock;
    invoer_listener &init_game, &run_game, &game_parameters, &transfer_hits;

public:
    invoer_taak(hwlib::keypad<16> keypad, hwlib::target::pin_in button, invoer_listener &init,
                invoer_listener &run, invoer_listener &game, invoer_listener &hits):
        task("invoer_taak"),
        keypad(keypad),
        button(button),
        ms100_clock(this, 100'000, "ms100_clock"),
        init_game(init),
        run_game(run),
        game_parameters(game),
        transfer_hits(hits)
    {}

    void main() override{
        for(;;){
            char button_id;
            wait(ms100_clock);
            if(button.read()){
                button_id = 'T';
            } else {
                button_id = keypad.getc();
            }

            init_game.button_pressed(button_id);
            run_game.button_pressed(button_id);
            game_parameters.button_pressed(button_id);
            transfer_hits.button_pressed(button_id);
        }
    }
};

#endif