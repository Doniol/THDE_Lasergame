#ifndef RUNGAME_HPP
#define RUNGAME_HPP

#include "IR.hpp"

class run_game_taak : public rtos::task<>, public decoder_listener, public invoer_listener, public parameters_listener{
private:
   struct recieved_hits{int player; int weapon;};
   send_taak &send;
   game_parameters_taak &game_parameters;
   init_game_taak &init_game;
   display_taak &display;
   transfer_hits_taak &transfer_hits;
   rtos::timer second_clock;
   rtos::timer cooldown_clock;
   rtos::flag message_flag;
   rtos::pool< std::array<int, 2> > message_pool;
   rtos::flag parameters_flag;
   rtos::pool< std::array<int, 3> > parameters_pool;
   rtos::channel<char, 10> run_input_channel;
   char button_id;
   bool cooldown;
   int hp;
   int player_id;
   int weapon;
   int time;
   int i;
   std::array<int, 3> par_msg;
   std::array<recieved_hits, 30> hits;
   enum class states{create_player_profile, waiting_for_parameters, running_game, gun_ready, gun_not_ready, game_over};
   states state;

public:
   run_game(send_taak &send, game_parameters_taak &game_parameters, init_game_taak &init_game, 
            display_taak &display, transfer_hits_taak &transfer_hits):
      task("run_game"),
      send(send),
      game_parameters(game_parameters),
      init_game(init_game),
      display(display),
      transfer_hits(transfer_hits),
      second_clock(this, "second_clock"),
      cooldown_clock(this, "cooldown_clock"),
	   message_flag(this, "message_flag"),
	   message_pool("message_pool"),
      parameters_flag(this, "parameters_flag"),
	   parameters_pool("parameters_pool"),
      run_input_channel(this, "run_input_channel")
   {}

   void signal_log(int player, int weapon) override{
      message_pool.write({player, weapon});
      message_flag.set();
   }

   void player_parameters(int player, int weapon, int time) override{
      parameters_pool.write({player, weapon, time});
      parameters_flag.set();
   }

   void button_pressed(char message) override{
      run_input_channel.write(message);
   }

   void main() override {
      state = states::create_player_profile;
      for(;;){       
         switch(state){
            case states::create_player_profile:
               hp = 100;
               second_clock.set(1'000'000);
               cooldown_clock.set(0);
               state = states::idle;
               i = 0;
               button_id = run_input_channel.read();
               if(button_id == 'C'){
                  init_game.run_init_game_control();
                  state = states::create_player_profile;
               }
               else if(button_id == 'A'){
                  game_parameters.run_game_parameters();
                  state = states::waiting_for_parameters;
               }
               break;

            case states::waiting_for_parameters:
               wait(parameters_flag);
               par_msg = parameters_pool.read();
               player_id = par_msg[0];
               weapon = par_msg[1];
               time = (par_msg[2] * 60);
               hits[0] = {player_id, weapon};
               state = states::running_game;
               break;
            
            case states::running_game:
               auto event = wait(message_flag + second_clock cooldown_clock+ run_input_channel);
               if(event == message_flag){
                  message = message_pool.read();
                  hit_by = message[0];
                  enemy_weapon = message[1];
                  hp -= weapon::damage;
                  hits[i] = {hit_by, enemy_weapon};
                  i++;
                  if(i == 30){
                     state = states::game_over;
                  }
                  else{
                     state = states::running_game;
                  }
               }
               if(event == second_clock){
                  time--;
                  if(time == 0){
                     state == states::game_over;
                  }
                  else{
                     state == states::running_game;
                  }
               }
               if(event == cooldown_clock){
                  cooldown = 0;
                  cooldown_clock.set(0);
                  state::states::running_game;
               }
               if(event == run_input_channel && cooldown == 0){
                  button_id = run_input_channel.read();
                  if(button_id == 'T'){
                     state = states::gun_ready;
                  }
               }
               break;
            
            case states::gun_ready:
               send.send_message(player_id, weapon);
               state = states::gun_not_ready;
               break;

            case states::gun_not_ready:
               cooldown = 1;
               cooldown_clock.set(weapon::firerate);
               state = states::running_game;
               break;

            case states::game_over:
               display.show_message("GAME OVER")
               if(time > 0){
                  time--;
                  state = states::game_over;
               }
               else{
                  transfer_hits.game_done(hits, player_id);
                  state = states::create_player_profile;
               }
               break;
         }
      }
   }
};

#endif