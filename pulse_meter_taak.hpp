#ifndef PULSE_METER_TAAK_HPP
#define PULSE_METER_TAAK_HPP

#include "IR.hpp"


/// @file

/// \brief
/// pulse_meter_taak class
/// \details
///Class for receiving data from the IR-Sensor

class pulse_meter_taak : public rtos::task<>{
private:
    // Pins to which the IR-sensor is connected
    hwlib::target::pin_in &sensor;
    hwlib::target::pin_out &gnd;
    hwlib::target::pin_out &vdd;
    // Decoder to which the pulse_meter sends it's data
    decoder_taak &decoder;
    // Clock and timer for triggering events
    rtos::clock us100_clock;
    rtos::timer pause_timer;
    // States created based on the STD
    enum class states{start_pulse_meter, waiting_for_signal, signal_received, pause_timer, check_pause};
    states state;

public:

    /// \brief
    /// Constructor for task
    /// \details
    /// This constructor makes a clock, and a timer, and variables 

    pulse_meter_taak(decoder_taak &decoder, hwlib::target::pin_in &sensor, hwlib::target::pin_out &gnd, hwlib::target::pin_out &vdd):
        task("pulse_meter"),
        sensor(sensor),
        gnd(gnd),
        vdd(vdd),
        decoder(decoder),
        us100_clock(this, 100, "us100_clock"),
        pause_timer(this, "pause_timer")
    {}

    // Main function for pulse_meter_taak
    void main() override{
        // Set VDD and GND pins for sensor
        gnd.write(0);
        vdd.write(1);
        gnd.flush();
        vdd.flush();

        // Create variables that will be used later on
        int startsignal;
        int endsignal;
        int i;
        int reading;
        std::array<int, 16> array;
        std::array<int, 16> test_array;
        std::array<int, 16> temp_array;

        // Select default state
        state = states::start_pulse_meter;
        for(;;){
            switch(state){
                // Default state that checks every 100us whether a pulse has been detected
                case states::start_pulse_meter:
                    i = 0;
                    wait(us100_clock);
                    reading = sensor.read();
                    if(reading == 1){
                        state = states::waiting_for_signal;
                    }
                    break;
                
                // State that checks for the start of a pulse every 100us
                case states::waiting_for_signal:
                    wait(us100_clock);
                    reading = sensor.read();
                    // If a pulse has started, set the current time as the beginning of the signal and go to the next state
                    if(reading == 0){
                        startsignal = hwlib::now_us();
                        state = states::signal_received;
                    }
                    break;

                // State that waits for the end of the signal
                case states::signal_received:
                    wait(us100_clock);
                    reading = sensor.read();
                    // If the pulse has ended, set the current time as the end of the signal, calculate the difference,
                    // save the corresponding bit to array_temp, and go to the next state
                    if(reading == 1){
                        endsignal = hwlib::now_us();
                        int signaltime = endsignal - startsignal;
                        // A pulse shorter than 1200us mean a 0, and a pulse longer than 1200us but shorter than 2000us means a 1
                        if(signaltime <= 1200){
                            temp_array[i] = 0;
                            state = states::pause_timer;
                            // Set the timer to 4600 instead of the regular 300 the IR-Protocol requires because of the 800us or 1600us
                            // that follows every pulse and has to be taken into consideration
                            pause_timer.set(4600);
                        } else if(signaltime <= 2000){
                            temp_array[i] = 1;
                            state = states::pause_timer;
                            pause_timer.set(4600);
                        } else {
                            // If an incredibly long pulse has been detected, >2000us, it's probably a mistake and will reset the saved arrays
                            state = states::start_pulse_meter;
                        }
                    }
                    break;

                // State that waits for the end of the pause
                case states::pause_timer:{
                    // Checks every 100us whether the new pulse has started
                    auto event = wait(us100_clock + pause_timer);
                    reading = sensor.read();
                    // If a new pulse has been detected, a new startsignal is saved, i goes up with 1 to reflect the place the new bit will
                    // have to be saved to in the temp_array, also selects the next state
                    if(event == us100_clock && reading == 0){
                        i++;
                        startsignal = hwlib::now_us();
                        state = states::signal_received;
                    // If the pause has taken longer than 4600us it's safe to assume that this is the end of the first array of bits or the 
                    // end of both arrays, a timer is set to check which of the two is the case in the next state and selects this very state
                    } else if(event == pause_timer){
                        pause_timer.set(1000);
                        state = states::check_pause;
                    }
                    break;
                }

                // State that checks whether a long pause is the end of the first array or the end of the entire message
                case states::check_pause:{
                    // Checks every 100us whether the new pulse has started
                    auto event = wait(us100_clock + pause_timer);
                    reading = sensor.read();
                    // If a pulse starts within 1000us of the start of the state, it is assumed that this is the start of the second array of
                    // bits in the message, as such is saves the previous array of bits to the variable called "array" for future use and 
                    // loops back to the waiting_for_signal state
                    if(event == us100_clock && reading == 0){
                        array = temp_array;
                        state = states::waiting_for_signal;
                    // If a 1000us passes by without any hint of a new pulse, it is assumed that this is the end of the entire message and as such
                    // the temporary array is saved to the variable called "test_array" to be sent to the decoder
                    // The end of the message resets the STD and returns to the default state of start_pulse_meter
                    } else if(event == pause_timer){
                        if(event == pause_timer){
                            test_array = temp_array;
                            decoder.decode(array, test_array);
                            state = states::start_pulse_meter;
                        }
                    }
                    break;
                }
            }
        }
    }
};

#endif