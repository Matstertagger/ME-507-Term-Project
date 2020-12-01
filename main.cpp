/** @file main.cpp
 *    This file contains a program that measures some angles.
 *    
 *
 *  @author  Peyton Ulrich, Jose Chavez, Tagupa
 * 
 *  @date    5 Nov 2020 Original file
 */

#include <Arduino.h>
#include <PrintStream.h>
#include <STM32FreeRTOS.h>
#include "taskshare.h"
#include "motorcontrol.h"

// Shares and queues should go here
/// Share that carries an integer from user interface to simulation task
Share<float_t> US_distance ("cm");
Share<float_t> IR_distance ("cm");
Share<int32_t> M1_duty_cycle ("PWM");
Share<int32_t> M2_duty_cycle ("PWM");
Share<bool> turn ("Turn");
float us_duration;
float us_distance;
int ir_reading;
int ir_distance;





 /**@brief   Task which interacts with a user. 
 *  @details This task demonstrates how to use a FreeRTOS task for interacting
 *           with some user while other more important things are going on.
 *  @param   p_params A pointer to function parameters which we don't use.
 */
 
void task_process_sensor_data (void* p_params)
{
    (void)p_params;            // Shuts up a compiler warning

    float us_distance;
    float ir_distance;
    int state;  //0 = running, 1 = stopped

    for (;;)
    {
        US_distance.get(us_distance);
        IR_distance.get(ir_distance);
        if(us_distance > 40){
            Serial << "Keep Running" << endl;
            state = 0;
            //calculates simulated motor speed
            
            M1_duty_cycle.put(100); //run motor full speed
            M2_duty_cycle.put(100);
            turn.put(false);
            turn.put(false);
        }
        else{
            //check IR
            if (ir_distance > 20){
                Serial << "Slow down!!" << endl;
                state = 0;
                M1_duty_cycle.put(50); //run motor ~1/3 speed
                M2_duty_cycle.put(50);
                turn.put(false);
                turn.put(false);
            }
            else{
                if(state){
                    Serial << "Turn!!" << endl;
                    M1_duty_cycle.put(50);
                    M2_duty_cycle.put(50);
                    turn.put(true);
                }
                else{
                    Serial << "Stop!!" << endl;
                    
                    state = 1;
                    M1_duty_cycle.put(0);
                    M2_duty_cycle.put(0);
                    turn.put(false);

                    delay(1000);    //pause 1s to allow motor/car to come to complete stop before turning
                
                } 
            }
        }
    }
}



/** @brief   Task which simulates a motor.
 *  @details This task runs at precise interfals using @c vTaskDelayUntil() and
 *           sort of simulates a motor whose duty cycle is controlled by a
 *           power level sent from the UI task. The simulation is just a very
 *           simple implementation of a first-order filter. 
 *  @param   p_params A pointer to function parameters which we don't use.
 */
void task_sensor_scan (void* p_params)
{
    (void)p_params;                             // Shuts up a compiler warning

    // Set up the variables of the simulation here
    const TickType_t sim_period = 60;         // RTOS ticks (ms) between runs

    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();


    for (;;)
    {

        //example code found at: https://cdn-learn.adafruit.com/downloads/pdf/distance-measurement-ultrasound-hcsr04.pdf
        pinMode (A1, INPUT);    //ultrasonic ECHO
        pinMode (A2, OUTPUT);   //ultrasonic TRIG
        pinMode (A5, INPUT);
        digitalWrite (A2, LOW);
        delayMicroseconds (2);
        digitalWrite (A2, HIGH);
        delayMicroseconds (10);
        us_duration = pulseIn(A1, HIGH, 30000);
        us_distance = us_duration / 58.2;
        //Serial << "Ultrasonic: " << us_distance << endl;  //test ultrasonic reading
        
        ir_reading = analogRead(A5);
        ir_distance = (5269.8/(ir_reading-136.88));
        //Serial << "Infrared: " << ir_distance << endl;    //test IR reading

        US_distance.put(us_distance);
        IR_distance.put(ir_distance);

        // This type of delay waits until it has been the given number of RTOS
        // ticks since the task previously began running. This prevents timing
        // inaccuracy due to not accounting for how long the task took to run
        vTaskDelayUntil (&xLastWakeTime, sim_period);
    }
}

/**@brief   Task which interacts with a user. 
 *  @details This task demonstrates how to use a FreeRTOS task for interacting
 *           with some user while other more important things are going on.
 *  @param   p_params A pointer to function parameters which we don't use.
 */
 
void task_RS_Motor1 (void* p_params)
{
    (void)p_params;            // Does nothing but shut up a compiler warning

    // Set up the variables of the simulation here
    const TickType_t sim_period = 60;         // RTOS ticks (ms) between runs

    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();

    
    int32_t duty_cycle_var;
    float sim_A = 0.9;
    float sim_B = 1 - sim_A;
    float sim_speed;

    MotorControl MotorA(D12, D7, D8, D10);
    bool turn_stored;


    for (;;)
    {
        // Gets duty_cycle_var from share
        M1_duty_cycle.get(duty_cycle_var);

        if(duty_cycle_var == 0){
            sim_speed = 0;  //if we want to stop the motor, just stop it
            turn_stored = true;
        }
        else{
            sim_speed = sim_speed * sim_A + duty_cycle_var * sim_B;    //otherwise, smoothly change speed
            turn.get(turn_stored);
        }
        
        //Serial << duty_cycle_var << endl;    //test duty_cycle_var
        //Serial << sim_speed << endl;    //test sim_speed
        
        MotorA.runMotor((uint32_t) sim_speed, turn_stored);
        
        vTaskDelayUntil (&xLastWakeTime, sim_period);
    }
}

/**@brief   Task which interacts with a user. 
 *  @details This task demonstrates how to use a FreeRTOS task for interacting
 *           with some user while other more important things are going on.
 *  @param   p_params A pointer to function parameters which we don't use.
 */
 
void task_RS_Motor2 (void* p_params)
{
    (void)p_params;            // Does nothing but shut up a compiler warning

    // Set up the variables of the simulation here
    const TickType_t sim_period = 60;         // RTOS ticks (ms) between runs

    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();

    
    int32_t duty_cycle_var;
    float sim_A = 0.9;
    float sim_B = 1 - sim_A;
    float sim_speed;

    MotorControl MotorB(D6, D2, D4, D9);


    for (;;)
    {
        // Gets duty_cycle_var from share
        M2_duty_cycle.get(duty_cycle_var);

        if(duty_cycle_var == 0){
            sim_speed = 0;  //if we want to stop the motor, just stop it
        }
        else{
            sim_speed = sim_speed * sim_A + duty_cycle_var * sim_B;    //otherwise, smoothly change speed
        }
        
        //Serial << duty_cycle_var << endl;    //test duty_cycle_var
        //Serial << sim_speed << endl;    //test sim_speed
        
        MotorB.runMotor((uint32_t) sim_speed, true);
        
        vTaskDelayUntil (&xLastWakeTime, sim_period);
    }
}


/** @brief   Arduino setup function which runs once at program startup.
 *  @details This function sets up a serial port for communication and creates
 *           the tasks which will be run.
 */
void setup () 
{
    // Start the serial port, wait a short time, then say hello. Use the
    // non-RTOS delay() function because the RTOS hasn't been started yet
    Serial.begin (115200);
    delay (2000);
    Serial << endl << endl << "ME507 UI Lab Starting Program" << endl;

    // Create a task which prints a more agreeable message
    xTaskCreate (task_sensor_scan,
                 "Scan",
                 1024,                            // Stack size
                 NULL,
                 4,                               // Priority
                 NULL);
    
    // Create a task which prints a more agreeable message
    xTaskCreate (task_process_sensor_data,
                 "Process",
                 1024,                            // Stack size
                 NULL,
                 2,                               // Priority
                 NULL);

    // Create a task which prints a more agreeable message
    xTaskCreate (task_RS_Motor1,
                 "RSMotor1",
                 1024,                            // Stack size
                 NULL,
                 3,                               // Priority
                 NULL);

    // Create a task which prints a more agreeable message
    xTaskCreate (task_RS_Motor2,
                 "RSMotor2",
                 1024,                            // Stack size
                 NULL,
                 3,                               // Priority
                 NULL);

    // If using an STM32, we need to call the scheduler startup function now;
    // if using an ESP32, it has already been called for us
    #if (defined STM32L4xx || defined STM32F4xx)
        vTaskStartScheduler ();
    #endif


}


/** @brief   Arduino's low-priority loop function, which we don't use.
 *  @details A non-RTOS Arduino program runs all of its continuously running
 *           code in this function after @c setup() has finished. When using
 *           FreeRTOS, @c loop() implements a low priority task on most
 *           microcontrollers, and crashes on some others, so we'll not use it.
 */
void loop () 
{
}
