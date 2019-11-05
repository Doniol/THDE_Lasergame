#ifndef DISPLAY_TAAK_HPP
#define DISPLAY_TAAK_HPP

#include "IR.hpp"

class display_taak : public rtos::task<>{
private:
    rtos::channel<std::array<int, 3>, 10> message_channel;
    hwlib::glcd_oled oled;

public:
    display_taak(hwlib::glcd_oled oled):
        task("display_taak"),
        message_channel(this, "message_channel"),
        oled(oled)
    {}

    void show_message(int player, int data){
        message_channel.write({player, data});
    }

    void main() override{
        auto font = hwlib::font_default_8x8();
        auto display = hwlib::terminal_from(oled, font);
        for(;;){
            auto message = message_channel.read();
            if(message[0] == 0){
                if(message[1] != 0){
                    display << "Command: " << message[1] << hwlib::flush;
                } else {
                    display << "Start Game" <<hwlib::flush;
                }
            } else {
                display << "Player: " << message[0] << "\nTime: " << message[1] << hwlib::flush;
            }
        }
    }
};

#endif