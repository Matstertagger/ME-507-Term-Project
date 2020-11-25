/** @file motorcontrol.h
 *      This file contains 
 *  @author Peyton Ulrich
 *  @date 2020-Oct-25    Original File
 */

#include "Arduino.h"

/** @brief  
 * @details 
 * @param   
 */
class MotorControl
{
    protected:
        uint32_t EN_stored;
        uint32_t IN_A_stored;
        uint32_t IN_B_stored;
        uint32_t PWM_stored;


    public:
        MotorControl (uint32_t EN, uint32_t IN_A, uint32_t IN_B, uint32_t PWM);
        void runMotor (uint32_t duty_cycle, bool clockwise);
};