#ifndef TRANSFER_HITS_TAAK_HPP
#define TRANSFER_HITS_TAAK_HPP

#include "IR.hpp"

class transfer_hits_taak : public rtos::task<>, public invoer_listener{
private:
    struct data{std::array<std::array<int, 2>, 30> hits; int player_id;};
    rtos::pool<data> game_done_pool;
    rtos::flag game_done_flag;
    rtos::flag trigger_pressed_flag;
    enum class states{wait_for_end, wait_for_connection};
    states state;

public:
    transfer_hits_taak():
        game_done_pool("game_done_pool"),
        game_done_flag(this, "game_done_flag"),
        trigger_pressed_flag(this, "trigger_pressed_flag")
    {}

    void button_pressed(char button_id) override{
        if(button_id == 'T'){
            trigger_pressed_flag.set();
        }
    }

    void game_done(std::array<std::array<int, 2>, 30> hits, int player_id){
        data input = {hits, player_id};
        game_done_pool.write(input);
    }

    void main() override{
        state = states::wait_for_end;
        for(;;){
            switch(state){
                case states::wait_for_end:
                    wait(game_done_flag);
                    state = states::wait_for_connection;
                    break;

                case states::wait_for_connection:
                    data message = game_done_pool.read();
                    wait(trigger_pressed_flag);
                    for(unsigned int i = 0; i < 30; i++){
                        hwlib::cout << message.player_id << ", hit by: " << message.hits[i][0] << ", with a: " << message.hits[i][1];
                    }
                    state = states::wait_for_end;
                    break;
            }
        }
    }
};

#endif