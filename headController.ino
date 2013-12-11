#include <Servo.h> 
 
// Servo
Servo headServo;         // Servo object from the servo library
const int head_servo_pin = 9;  // Pin to connect the head servo to
const int head_pot_pin   = 0;  // Pin to connect the pot that controls head direction
const int pot_noise      = 2;  // Two consecutive readings will be considered equal if they are within this value
int head_pot_val;        // Value of the head pot.

// Buttons - Note: buttons are connected to pull-down resistors thus are nominally 0;
// using intterupt pins
const int left_but_pin   = 2;  // Pin to connect the left hand button to
const int right_but_pin  = 3;  // Pin to connect the right hand button to

// Routine index
volatile int cur_routine_index = 0;            // Index to switch on in the main loop()
volatile boolean restart_routine_flag = true;  // A flag to know if the routine should start from its beginning or merely continue
// Routine labels to seitch on
const int NORMAL       = 0;
const int LEFT_SHAKE   = 1;
const int RIGHT_SHAKE  = 2;

int left_but_state;
int right_but_state;
const int debounce_milli = 50; // The amount of milliseconds to use for switch debounce

// LEDs - Eye leds are connected in parallel with wires for indevidual RGB values;
// Using PWM pins to allow pulsing
const int eyes_r_pin     = 3;
const int eyes_g_pin     = 5;
const int eyes_b_pin     = 6;

int eyes_r_val;
int eyes_g_val;
int eyes_b_val;

// Helper functions
void eyesWhite()
{
   eyes_r_val = 255;
   eyes_g_val = 255;
   eyes_b_val = 255; 
}

void eyesOff()
{
   eyes_r_val = 0;
   eyes_g_val = 0;
   eyes_b_val = 0; 
}

void leftShakeISR()
{
  restart_routine_flag = true;
  cur_routine_index = LEFT_SHAKE;
}

void rightShakeISR()
{
  restart_routine_flag = true;
  cur_routine_index = RIGHT_SHAKE;  
}

void setup() 
{ 
  headServo.attach(9);  // attaches the servo on pin 9 to the servo object 
  attachInterrupt(0, leftShakeISR, RISING);
  attachInterrupt(1, rightShakeISR, RISING);
} 

boolean potValueChanged()
{
  int new_head_pot_val = analogRead(head_pot_pin);
  // if current value is within pot_noise of last val return no change
  if (( new_head_pot_val <= (head_pot_val + pot_noise) ) || ( new_head_pot_val >= (head_pot_val - pot_noise) ))
  {
    return false;
  } else
  {
    head_pot_val = new_head_pot_val;
    return true;
  }
}

void readPot()
{
   int val = analogRead(head_pot_pin);            // reads the value of the potentiometer (value between 0 and 1023) 
  val = map(val, 0, 1023, 0, 179);     // scale it to use it with the servo (value between 0 and 180) 
  headServo.write(val);                  // sets the servo position according to the scaled value 
}

// Global variables for shake routine
int cur_shake_val;                    // The global variable used to remember current shake state
int shake_left_bound = 0;
int shake_right_bound = 180;
int shake_speed = 1;
const int shake_range = 20;
void shakeHead(int shake_index)
{ 
  
  if (restart_routine_flag == true)
  {
    if (shake_index == LEFT_SHAKE)
    {
      shake_left_bound = 0;
      shake_right_bound = 0 + shake_range;
      cur_shake_val = 0 + shake_range/2;
    } else if (shake_index == RIGHT_SHAKE)
    {
      shake_left_bound = 180 - shake_range;
      shake_right_bound = 180;
      cur_shake_val = 180 - shake_range/2;
    }
    headServo.write(cur_shake_val);    
    cur_shake_val += shake_speed;
    
    if ((cur_shake_val >= shake_right_bound) || (cur_shake_val <= shake_left_bound))
    {
     shake_speed = -shake_speed; 
     cur_shake_val += shake_speed;
    }    
  }    
}
 
void loop() 
{ 
  if ( potValueChanged() )
  {
    cur_routine_index = NORMAL;
  }
  
  switch (cur_routine_index) 
  {   
   case LEFT_SHAKE:
     shakeHead(LEFT_SHAKE);
   break;
   case RIGHT_SHAKE:
     shakeHead(RIGHT_SHAKE);
   break;
   case NORMAL: // Same as default
   default:
     readPot();
   
  };
  
 
  delay(15);                           // waits for the servo to get there 
} 
