#ifndef DECODER_TAAK_HPP
#define DECODER_TAAK_HPP

#include "IR.hpp"

// Class created to be used as a listener for the decoder_taak class
class decoder_listener{
public:
    // A virtual function created to be used as desired by the subclasses of this function
    virtual void signal_log(int player, int data) == 0;
};

// Class used to decode the output from the pulse_meter_taak and send it to the desired tasks
class decoder_taak : public rtos::task<>{
private:
    // A pool and a flag for receiving data through the function decode()
    rtos::pool<std::array<std::array<int, 16>, 2>> bits_pool;
    rtos::flag bits_flag;
    // The listeners where the decoded data will be sent to
    decoder_listener &run_game_listener, &game_parameters_listener;

public:
    // Constructor for decoder_taak
    decoder_taak(decoder_listener &run, decoder_listener &game):
        task("decoder_taak"),
        bits_pool("bits_pool"),
        bits_flag(this, "bits_flag"),
        run_game_listener(run),
        game_parameters_listener(game)
    {}

    // Function used to write data to the bits_pool and to set the bits_flag
    void decode(std::array<int, 16> array, std::array<int, 16> test_array){
        bits_pool.write({array, test_array});
        bits_flag.set();
    }

    // Function to compare 2 arrays and returns whether they're the same or not
    bool compare_arrays(std::array<int, 16> array1, std::array<int, 16> array2){
        // For each integer in array1, check whether it's the same as the one in the same place in array2
        for(unsigned int i = 0; i < 16; i++){
            if(array1[i] != array2[i]){
                // If the integers are different, immediatly return false
                return false;
            }
        }
        // If everything is alright true will be returned
        return true;
    }

    // Tests whether the xor_test in the received array equals to true
    bool xor_test(std::array<int, 16> array){
        // Check whether the last 5 bits equal to the xor of the 2 pairs of 5 bits before them
        for(unsigned int i = 0; i < 5; i++){
            if(array[15 - i] != (array[5 - i] ^ array[10 - i])){
                // If not, immediatly return false
                return false;
            }
        }
        // If everything goes fine then return true
        return true;
    }

    // Turn an array of 5 ints to an integer
    int to_int(std::array<int, 5> array){
        uint8_t calculation = 0;
        // For every integer in the array, check whether it's 1 and if so: calculate the new integer using bitwise operators
        for(unsigned int i = 0; i < 5; i++){
            if(array[4 - i] == 1){
                calculation |= 1 << i;
            }
        }
        // Return the calculated integer
        return calculation;
    }

    // Main function for decoder_taak
    void main() override{
        for(;;){
            // Wait for incoming data
            wait(bits_flag);

            // Read the incoming data and split it into the desired arrays
            auto arrays = bits_pool.read();
            auto array = arrays[0];
            auto test_array = arrays[1];
            // Compare the two arrays and check whether the first bit of the array is 1, if not: immediatly reset the for-loop
            if(!compare_arrays(array, test_array) || array[0] != 1){
                continue;
            }

            // Split the array into even more arrays
            std::array<int, 5> player_array = {array[1], array[2], array[3], array[4], array[5]};
            std::array<int, 5> data_array = {array[6], array[7], array[8], array[9], array[10]};
            
            // Do the xor_test to verify the message integrity
            if(!xor_test(array)){
                continue;
            }

            // Create integers with the data from the splitoff arrays
            int player = to_int(player_array);
            int data = to_int(data_array);
            
            // Send this data to the listeners
            run_game_listener.signal_log(player, data);
            game_parameters_listener.signal_log(player, data);
        }
    }
};

#endif