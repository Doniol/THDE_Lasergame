#ifndef GAME_PARAMETERS_TAAK_HPP
#define GAME_PARAMETERS_TAAK_HPP

#include "IR.hpp"

/// @file

/// \brief
/// parameter listener
/// \details
/// This class is a listener.

class parameters_listener{
public:
    virtual void player_parameters(int player, int weapon, int time) = 0;
};

/// \brief
/// game parameter task
/// \details
/// This class makes a player and gives them a weapon and time.

class game_parameters_taak : public rtos::task<>, public invoer_listener, public decoder_listener{
private:
    rtos::channel<char, 10> parameters_input_channel;
    rtos::pool<std::array<int, 2>> message_pool;
    rtos::flag message_flag;
    rtos::flag run_parameters_flag;
    enum class states{wait_new_game, get_player_id, get_weapon, wait_time, wait_start};
    states state;
    parameters_listener* listener;

public:
    /// \brief
    /// game parameter constructor
    /// \details
    /// Makes a channel, a pool and 2 flags.
    game_parameters_taak():
        task("game_parameters"),
        parameters_input_channel(this, "parameters_input_channel"),
        message_pool("message_pool"),
        message_flag(this, "message_flag"),
        run_parameters_flag(this, "run_parameters_flag")
    {}

    /// \brief
    /// add listener
    /// \details
    /// sets the listener.
    void add_listener(parameters_listener* given_listener){
        listener = given_listener;
    }

    /// \brief
    /// send player parameters
    /// \details
    /// sends the player parameters
    void send_player_parameters(int player, int weapon, int time){
        listener->player_parameters(player, weapon, time);
    }

    /// \brief
    /// button pressed
    /// \details
    /// Allows you to write to the input channel.
    void button_pressed(char button_id) override{
        parameters_input_channel.write(button_id);
    }

    /// \brief
    /// signal log
    /// \details
    /// Allows you to write to the message pool and set the message flag.
    void signal_log(int player, int data) override{
        message_pool.write({player, data});
        message_flag.set();
    }

    /// \brief
    /// run game parameters
    /// \details
    /// Allows you to set the run game flag
    void run_game_parameters(){
        run_parameters_flag.set();
    }
    /// \brief
    /// check for num
    /// \details
    /// checks if the character is a number.
    bool check_for_num(char button_id){
        if(button_id != '*' && button_id != '#' && button_id != 'A' && button_id != 'B' && button_id != 'C' && button_id != 'D'){
            return true;
        }
        return false;
    }

    void main() override{
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
                    hwlib::cout << "Parameters start\n\n";
                    state = states::get_player_id;
                    break;
                
                case states::get_player_id:
                    button_id = parameters_input_channel.read();
                    hwlib::cout << button_id << " button\n";
                    if(check_for_num(button_id)){
                        hwlib::cout << "button = num\n";
                        player_id = button_id - '0';
                        hwlib::cout << "playerid = " << player_id << "\n";
                    } else if(button_id == 'B' && player_id != 0){
                        hwlib::cout << "get_weapon\n\n";
                        state = states::get_weapon;
                    }
                    break;

                case states::get_weapon:
                    hwlib::cout << "arrived weapon\n";
                    button_id = parameters_input_channel.read();
                    hwlib::cout << button_id << "\n";
                    if(check_for_num(button_id)){
                        weapon = button_id - '0';
                        state = states::wait_time;
                        hwlib::cout << weapon << "\n\n";
                    }
                    break;

                case states::wait_time:{
                    hwlib::cout << "wait time engaged\n"; 
                    wait(message_flag);
                    hwlib::cout << "message flag accepted\n";
                    auto message = message_pool.read();
                    player = message[0];
                    data = message[1];
                    hwlib::cout << message[0] << " " << message[1] << "\n";
                    if(player == 0 && data > 0){
                        time = data;
                        state = states::wait_start;
                        hwlib::cout << "time = " << time << " and wait_start\n\n";
                    }
                    break;
                }
                
                case states::wait_start:{
                    hwlib::cout << "wait start engaged\n";
                    wait(message_flag);
                    hwlib::cout << "message flag accepted\n";
                    auto message = message_pool.read();
                    player = message[0];
                    data = message[1];
                    hwlib::cout << message[0] << " " << message[1] << "\n";
                    if(player == 0 && data == 0){
                        send_player_parameters(player_id, weapon, time);
                        state = states::wait_new_game;
                        hwlib::cout << "sent parameters\n\n";
                    }
                    break;
                }
            }
        }
    }
};

#endif