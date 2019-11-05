#ifndef GAME_PARAMETERS_TAAK_HPP
#define GAME_PARAMETERS_TAAK_HPP

#include "IR.hpp"

class game_parameters : public rtos::task<> : public invoer_listener : public decoder_listener{
private:
    rtos::channel<char, 10> parameters_input_channel;
    rtos::pool<std::array<int, 2>> message_pool;
    rtos::flag message_flag;
    rtos::flag run_parameters_flag;
    enum class states{wait_new_game, get_player_id, get_weapon, wait_time, wait_start};
    states state;

public:
    game_parameters():
        task("game_parameters"),
        parameters_input_channel(this, "parameters_input_channel"),
        message_pool("message_pool"),
        message_flag(this, "message_flag"),
        run_parameters_flag(this, "run_parameters_flag")
    {}

    void button_pressed(char button_id) override{
        parameters_input_channel.write(button_id);
    }

    void signal_log(int player, int data) override{
        message_pool.write({player, data});
        message_flag.set();
    }

    void run_game_parameters(){
        run_parameters_flag.set();
    }

    bool check_for_num(char button_id){
        if(button_id != '*' && button_id != '#' && button_id != 'A' && button_id != 'B' && button_id != 'C' && button_id != 'D'){
            return true;
        }
        return false;
    }

    void main override{
        state = states::wait_new_game;
        char button_id;
        int player_id;
        int weapon;
        int time;
        int player;
        int data;
        for(;;){
            switch(state){
                case states::wait_new_game:
                    wait(run_parameters_flag);
                    button_id = parameters_input_channel.read();
                    state = states::get_player_id;
                    break;
                
                case states::get_player_id:
                    button_id = parameters_input_channel.read();
                    if(check_for_num(button_id)){
                        player_id = int(button_id);
                    } else if(button_id == 'B'){
                        state = states::get_weapon;
                    }
                    break;

                case states::get_weapon:
                    button_id = parameters_input_channel.read();
                    if(check_for_num(button_id)){
                        weapon = int(button_id);
                        state = states::wait_time;
                    }
                    break;

                case states::wait_time:{
                    wait(message_flag);
                    auto message = message_pool.read();
                    player = message[0];
                    data = message[1];
                    if(player == 0 && data > 0){
                        time = data;
                        state = states::wait_start;
                    }
                    break;
                }
                
                case states::wait_start:{
                    wait(message_flag);
                    auto message = message_pool.read();
                    player = message[0];
                    data = message[1];
                    if(player == 0 && message == 0){
                        run_game_control.player_parameters(player_id, weapon, time);
                        state = states::wait_new_game;
                    }
                    break;
                }
            }
        }
    }
};

#endif