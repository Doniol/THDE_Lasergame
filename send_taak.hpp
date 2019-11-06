#ifndef SEND_TAAK_HPP
#define SEND_TAAK_HPP

#include "IR.hpp"

/// @file

/// \brief
/// send task
/// \details
/// This class waits for the message flag an sends the message

class send_taak : public rtos::task<>{
private:
    rtos::pool<std::array<int, 2>> message_pool;
    rtos::flag message_flag;
    hwlib::target::d2_36kHz ir;
    std::array<int, 16> signal_array;
    enum class states{idle, signal};
    states state;

public:
    /// \brief
    /// send task constructor
    /// \details
    /// A pool and a flag.
    send_taak():
        task("send_taak"),
        message_pool("message_pool"),
        message_flag(this, "message_flag")
    {}
    /// \brief
    /// sends message
    /// \details
    /// Allows you to write to the message pool and set the message flag.
    void send_message(int player, int data){
        message_pool.write({player, data});
        message_flag.set();
    }

    /// \brief
    /// converts to binary
    /// \details
    /// Converts 2 ints to binary and calculates the xor.
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

    /// \brief
    /// converts to signals
    /// \details
    /// Converts binary to ir signals.
    void beep(std::array< int, 16 > & array, hwlib::target::d2_36kHz & ir){ 
        for(unsigned int i = 0; i < array.size(); i++){ 
            if(array[i] == 0){
                ir.write(1);
                hwlib::wait_us_busy(800);
                ir.write(0);
                hwlib::wait_us_busy(1'600);
            } else {
                ir.write(1);
                hwlib::wait_us_busy(1'600);
                ir.write(0);
                hwlib::wait_us_busy(800);
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
                    hwlib::wait_us_busy(3'000);
                    beep(signal_array, ir);
                    state = states::idle;
                    break;
            }
        }
    }
};

#endif