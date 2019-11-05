#ifndef SEND_TAAK_HPP
#define SEND_TAAK_HPP

#include "IR.hpp"

class send_taak : public rtos::task<>{
private:
    rtos::pool<std::array<int, 2>> message_pool;
    rtos::flag message_flag;
    hwlib::target::d2_36kHz ir;
    std::array<int, 16> signal_array;
    rtos::timer timed_out;
    rtos::timer sender;
    enum class states{idle, signal};
    states state;

public:
    send_taak():
        task("send_taak"),
        message_pool("message_pool"),
        message_flag(this, "message_flag"),
        timed_out(this, "timed_out timer"),
        sender(this, "sender timer")
    {}

    void send_message(int player, int data){
        message_pool.write({player, data});
        message_flag.set();
    }

    void fill_array(std::array< int, 16 > & array, const uint8_t player, const uint8_t data){
        array[0] = 0;
        for(unsigned int i = 0; i < (array.size() - 11); i++){ 
            array[5 - i] = (player >> i) & 1;
        }
        for(unsigned int i = 0; i < (array.size() - 11); i++){ 
            array[10 - i] = (data >> i) & 1;
        }
        for(unsigned int i = 0; i < (array.size() - 11); i++){ 
            array[15 - i] = array[5 - i] ^ array[10 - i];
        }
    }

    void beep(std::array< int, 16 > & array, hwlib::target::d2_36kHz & ir){ 
        for(unsigned int i = 0; i < array.size(); i++){ 
            if(array[i] == 0){
                ir.write(1);
                sender.set(800);
                wait(sender);
                ir.write(0);
                sender.set(1'600);
                wait(sender);
            } else {
                ir.write(1);
                sender.set(1'600);
                wait(sender);
                ir.write(0);
                sender.set(800);
                wait(sender);
            }
        }
    }

    void main() override {
        state = states::idle;
        std::array<int, 2> msg;
        for(;;){       
            switch(state){
                case states::idle:
                    wait(message_flag);
                    state = states::signal;
                    break;

                case states::signal:
                    msg = message_pool.read();
                    fill_array(signal_array, msg[0], msg[1]);
                    beep(signal_array, ir);
                    timed_out.set(3'000);
                    wait(timed_out);
                    beep(signal_array, ir);
                    state = states::idle;
                    break;
            }
        }
    }
};

#endif