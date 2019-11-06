#ifndef RUN_GAME_TAAK_HPP
#define RUN_GAME_TAAK_HPP

#include "IR.hpp"

/// @file

/// \brief
/// run game task
/// \details
/// This task controls several other tasks and checks health and time.
class run_game_taak : public rtos::task<>, public decoder_listener, public invoer_listener, public parameters_listener{
private:
   send_taak &send;
   game_parameters_taak &game_parameters;
   init_game_taak &init_game;
   display_taak &display;
   transfer_hits_taak &transfer_hits;
   rtos::clock second_clock;
   rtos::flag message_flag;
   rtos::pool< std::array<int, 2> > message_pool;
   rtos::flag parameters_flag;
   rtos::pool< std::array<int, 3> > parameters_pool;
   rtos::channel<char, 10> run_input_channel;
   char button_id;
   int hp;
   int player_id;
   int weapon;
   int time;
   int i;
   int firerate;
   std::array<int, 2> message;
   int hit_by;
   int enemy_weapon;
   std::array<int, 3> par_msg;
   std::array<std::array<int, 2>, 30> hits;
   enum class states{create_player_profile, waiting_for_parameters, running_game, game_over};
   states state;
   enum class gun_states{gun_ready, gun_not_ready};
   gun_states gun_state;

public:
   /// \brief
   /// run game constructor constructor
   /// \details
   /// Makes a clock, a channel, 2 pools and 2 flags.
   run_game_taak(send_taak &send, game_parameters_taak &game_parameters, init_game_taak &init_game, 
            display_taak &display, transfer_hits_taak &transfer_hits):
      task("run_game"),
      send(send),
      game_parameters(game_parameters),
      init_game(init_game),
      display(display),
      transfer_hits(transfer_hits),
      second_clock(this, 2'000'000, "second_clock"),
	   message_flag(this, "message_flag"),
	   message_pool("message_pool"),
      parameters_flag(this, "parameters_flag"),
	   parameters_pool("parameters_pool"),
      run_input_channel(this, "run_input_channel")
   {}

   /// \brief
   /// signal log
   /// \details
   /// Allows you to write to the message pool and set the message flag.
   void signal_log(int player, int data) override{
      message_pool.write({player, data});
      message_flag.set();
   }

   /// \brief
   /// signal log
   /// \details
   /// Allows you to write to the parameters pool and set the parameters flag.
   void player_parameters(int player, int weapon, int time) override{
      parameters_pool.write({player, weapon, time});
      parameters_flag.set();
   }

   /// \brief
   /// button pressed
   /// \details
   /// Allows you to write to the input channel.
   void button_pressed(char button_id) override{
      run_input_channel.write(button_id);
   }

   /// \brief
   /// weapon firerate
   /// \details
   /// changes your firerate to you the correct one. 
   void weapon_firerate(int & weapon){
      switch(weapon){
         case 0:
            firerate = 1;
         case 1:
            firerate = 2;
         case 2:
            firerate = 3;
         case 3:
            firerate = 4;
         case 4:
            firerate = 5;
         case 5:
            firerate = 6;
         case 6:
            firerate = 8;
         case 7:
            firerate = 9;
         case 8:
            firerate = 12;
         case 9:
            firerate = 18;
      }
   }

   /// \brief
   /// enemy weapon damage
   /// \details
   /// Changes the enemy damage to the correct one. 
   int enemy_weapon_damage(int & enemy_weapon){
      switch(enemy_weapon){
         case 0:
            return 4;
         case 1:
            return 8;
         case 2:
            return 10;
         case 3:
            return 15;
         case 4:
            return 19;
         case 5:
            return 25;
         case 6:
            return 30;
         case 7:
            return 50;
         case 8:
            return 75;
         case 9:
            return 100;
      }
      return 0;
   }

   void main() override {
      state = states::create_player_profile;
      gun_state = gun_states::gun_ready;
      hwlib::cout << "run game\n";
      int reload;
      for(;;){       
         switch(state){
            case states::create_player_profile:
               hp = 100;
               i = 0;
               button_id = run_input_channel.read();
               if(button_id == 'C'){
                  init_game.run_init_game_control();
               }
               else if(button_id == 'A'){
                  game_parameters.add_listener(this);
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
               // weapon_firerate(weapon);
               hits[0] = {player_id, weapon};
               state = states::running_game;
               hwlib::cout << player_id << " " << weapon << " " << time << "\n\n";
               break;
            
            case states::running_game:{
               auto event = wait(message_flag + second_clock + run_input_channel);
               hwlib::cout << "event\n";
               if(event == message_flag){
                  hwlib::cout << "hit by\n";
                  message = message_pool.read();
                  hit_by = message[0];
                  enemy_weapon = message[1];
                  int enemy_damage = enemy_weapon_damage(enemy_weapon);
                  hp -= enemy_damage;
                  hwlib::cout << hit_by << " " << weapon << " " << hp << "\n";
                  hits[i] = {hit_by, enemy_weapon};
                  i++;
                  if(hp <= 0){
                     state = states::game_over;
                  }
               } else if(event == run_input_channel){
                  hwlib::cout << "button pressed\n";
                  button_id = run_input_channel.read();
                  if(button_id == 'D'){
                     switch(gun_state){
                        case gun_states::gun_ready:
                           hwlib::cout << "shoot\n";
                           send.send_message(player_id, weapon);
                           reload = firerate;
                           gun_state = gun_states::gun_not_ready;
                           break;
                        
                        case gun_states::gun_not_ready:
                           hwlib::cout << "reloading\n";
                           if(reload <= 0){
                              gun_state = gun_states::gun_ready;
                           }
                           break;
                     }
                  }
               } else if(event == second_clock){
                  hwlib::cout << "second over\n";
                  time -= 2;
                  reload -= 2;
                  if(time == 0){
                     state = states::game_over;
                  }
               }
               display.show_message(hp, time);
               break;
            }

            case states::game_over:
               hwlib::cout << "GAME OVER";
               display.show_message(-1, -1);
               if(time > 0){
                  time--;
               } else {
                  transfer_hits.game_done(hits, player_id);
                  state = states::create_player_profile;
               }
               break;
         }
      }
   }
};

#endif
