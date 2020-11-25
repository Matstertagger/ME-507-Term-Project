/** @file motorcontrol.cpp
 *      This file contains a class that 
 *  @author Peyton Ulrich
 *  @date 2020-Oct-25    Original File
*/

#include <Arduino.h>
#include <PrintStream.h>
#include "motorcontrol.h"

/** @brief  
 * @details 
 * @param   
 */
MotorControl::MotorControl (uint32_t EN, uint32_t IN_A, uint32_t IN_B, uint32_t PWM){
    pinMode (EN, OUTPUT);
    pinMode (IN_A, OUTPUT);
    pinMode (IN_B, OUTPUT);
    pinMode (PWM, OUTPUT);

    EN_stored = EN;
    IN_A_stored = IN_A;
    IN_B_stored = IN_B;
    PWM_stored = PWM;
}

void MotorControl::runMotor(uint32_t duty_cycle, bool clockwise){

    // EN SET HIGH
    digitalWrite (EN_stored, HIGH);

    if(duty_cycle == 0){
        //INA SET HIGH, INB SET HIGH
        digitalWrite (IN_A_stored, HIGH);
        digitalWrite (IN_B_stored, HIGH);
        Serial << "Stop Motor" << endl;
    }
    else{
        if(clockwise){
            //INA SET HIGH, INB SET LOW
            digitalWrite (IN_A_stored, HIGH);
            digitalWrite (IN_B_stored, LOW);
        }
        else{
            //INA SET LOW, INB SET HIGH
            digitalWrite (IN_A_stored, LOW);
            digitalWrite (IN_B_stored, HIGH);
        }
        analogWrite (PWM_stored, duty_cycle);
    }
    
}