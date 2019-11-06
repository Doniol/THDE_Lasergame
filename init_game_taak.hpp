#ifndef INIT_GAME_TAAK_HPP
#define INIT_GAME_TAAK_HPP

#include "IR.hpp"

class init_game_taak : public rtos::task<>, public invoer_listener{
private:
    rtos::channel<char, 10> init_input_channel;
    rtos::flag run_init_game_flag;
    display_taak &display;
    send_taak &send;
    enum class states{idle, get_command, send_command, send_start};
    states state;

public:
    init_game_taak(send_taak &send, display_taak &display):
        task("init_game_taak"),
        init_input_channel(this, "init_input_channel"),
        run_init_game_flag(this, "run_init_game_flag"),
        display(display),
        send(send)
    {}

    void run_init_game_control(){
        run_init_game_flag.set();
    }

    void button_pressed(char button_id) override{
        init_input_channel.write(button_id);
    }

    bool check_for_num(char button_id){
        if(button_id != '*' && button_id != '#' && button_id != 'A' && button_id != 'B' && button_id != 'C' && button_id != 'D'){
            return true;
        }
        return false;
    }

    void main() override{
        char button_id;
        state = states::idle;
        int command;
        for(;;){
            switch(state){
                case states::idle:
                    wait(run_init_game_flag);
                    hwlib::cout << "init\n";
                    command = 1;
                    button_id = init_input_channel.read();
                    if(button_id == 'C'){
                        state = states::get_command;
                    }
                    break;
                
                case states::get_command:
                    hwlib::cout << "get command\n";
                    display.show_message(0, command);
                    button_id = init_input_channel.read();
                    if(check_for_num(button_id)){
                        hwlib::cout << command << " += " << button_id;
                        command += button_id - '0';
                        hwlib::cout << " = " << command << "\n";
                    } else if(button_id == '#'){
                        hwlib::cout << " is # ";
                        if(command > 16){
                            command = 1;
                            hwlib::cout << " command = 0\n";
                        } else {
                            state = states::send_command;
                            display.show_message(0, 0);
                            hwlib::cout << " command is ok\n";
                        }
                    }
                    break;
                
                case states::send_command:
                hwlib::cout << "send command\n";
                    button_id = init_input_channel.read();
                    if(button_id == '*'){
                        state = states::send_start;
                    } else if(button_id == '#'){
                        hwlib::cout << "send message\n";
                        send.send_message(0, command);
                    }
                    break;

                case states::send_start:
                    hwlib::cout << "send start message\n";
                    send.send_message(0, 0);
                    button_id = init_input_channel.read();
                    if(button_id != '*'){
                        hwlib::cout << "send\n";
                        state = states::idle;
                    }
                    break;
            }
        }
    }
};

#endif