#ifndef DISPLAY_TAAK_HPP
#define DISPLAY_TAAK_HPP

#include "IR.hpp"

class display_taak : public rtos::task<>{
private:
    rtos::channel<std::string, 10> texts;
    hwlib::glcd_oled oled;

public:
    display_taak(hwlib::glcd_oled oled):
        task("display_taak"),
        texts(this, "texts"),
        oled(oled)
    {}

    void show_message(std::string text){
        texts.write(text);
    }

    void main() override{
        auto font = hwlib::font_default_8x8();
        auto display = hwlib::terminal_from(oled, font);
        for(;;){
            std::string text = texts.read();
            display << text << hwlib::flush;
        }
    }
};

#endif