#include <Servo.h> 
const boolean DEBUG = 0; 
// Servo
Servo headServo;         // Servo object from the servo library
const int head_servo_pin = 9;  // Pin to connect the head servo to
const int head_pot_pin   = 0;  // Pin to connect the pot that controls head direction
const int pot_noise      = 10;  // Two consecutive readings will be considered equal if they are within this value
int head_pot_val;        // Value of the head pot.

// Buttons - Note: buttons are connected to pull-down resistors thus are nominally 0;
// using intterupt pins
const int left_but_pin   = 2;  // Pin to connect the left hand button to
const int right_but_pin  = 3;  // Pin to connect the right hand button to
int last_left_time;
int last_right_time;
const int debounce_milli = 200; // The amount of milliseconds to use for switch debounce

// Routine index
volatile int cur_routine_index = 0;            // Index to switch on in the main loop()
volatile boolean restart_routine_flag = true;  // A flag to know if the routine should start from its beginning or merely continue
// Routine labels to seitch on
const int NORMAL       = 0;
const int LEFT_SHAKE   = 1;
const int RIGHT_SHAKE  = 2;

// LEDs - Eye leds are connected in parallel with wires for indevidual RGB values;
// Using PWM pins to allow pulsing
const int LEFT_EYE_PIN   = 5;
const int RIGHT_EYE_PIN  = 11;

int left_eye_val;
int right_eye_val;


void leftButtonISR()
{
  if (millis() >= (last_left_time + debounce_milli))
  {
    
     restart_routine_flag = true;
     cur_routine_index = LEFT_SHAKE;
     last_left_time = millis();
  }
}

void rightButtonISR()
{
  if (millis() >= (last_right_time + debounce_milli))
  {
    
     restart_routine_flag = true;
     cur_routine_index = RIGHT_SHAKE;
     last_right_time = millis();
  } 
}

boolean potValueChanged()
{
  int new_head_pot_val = analogRead(head_pot_pin);
  if (true) 
  {
    Serial.print(new_head_pot_val);
    Serial.print(" ");
  }
  // if current value is within pot_noise of last val return no change
  if (( new_head_pot_val <= (head_pot_val + pot_noise) ) && ( new_head_pot_val >= (head_pot_val - pot_noise) ))
  {
    return false;
  } else
  {
    head_pot_val = new_head_pot_val;
    if (DEBUG) { Serial.println("Pot Changed"); }
    return true;
  }
}

void setHeadFromPot()
{
   int val = analogRead(head_pot_pin);            // reads the value of the potentiometer (value between 0 and 1023) 
  val = map(val, 0, 1023, 10, 170);     // scale it to use it with the servo (value between 0 and 180) 
  Serial.println("====");
  Serial.println(val);
  headServo.write(val);                  // sets the servo position according to the scaled value 
}

const int LOW_BRIGHTNESS = 10;
const int HIGH_BRIGHTNESS = 100;
const int NORMAL_FADE_AMOUNT = 1.0;
const int FAST_FADE_AMOUNT = 10.0;
volatile double fade_amount = 1.0;
int fade_dir = 1;
int brightness;

void pulseEyes()
{
  if (DEBUG)
  {
  Serial.print(fade_amount);
  Serial.print(" ");
  Serial.print(fade_dir);
  Serial.print(" ");
  Serial.println(brightness);
  }
  // reverse the direction of the fading at the ends of the fade: 
  if (( brightness <= LOW_BRIGHTNESS )  || ( brightness >= HIGH_BRIGHTNESS ))
  {
    fade_dir = -fade_dir;
  }
  
  brightness = brightness + (fade_amount * fade_dir);
  
  analogWrite(LEFT_EYE_PIN, brightness);
  analogWrite(RIGHT_EYE_PIN, brightness);
}

// Global variables for shake routine
int cur_shake_val;                    // The global variable used to remember current shake state
int shake_left_bound = 10;
int shake_right_bound = 170;
int shake_speed = 1;
int num_shakes = 0;
const int MAX_LEFT_SHAKE = 10;
const int MAX_RIGHT_SHAKE = 170;
const int total_shakes = 5;
const int shake_range = 20;

void shakeHead(int shake_index)
{ 
  // check for restart
  if (restart_routine_flag == true)
  {
    fade_amount = FAST_FADE_AMOUNT;
    num_shakes = 0;
    
    if (shake_index == LEFT_SHAKE)
    {
      shake_left_bound = MAX_LEFT_SHAKE;
      shake_right_bound = MAX_LEFT_SHAKE + shake_range;
      cur_shake_val = MAX_LEFT_SHAKE + shake_range/2;
    } else if (shake_index == RIGHT_SHAKE)
    {
      shake_left_bound = MAX_RIGHT_SHAKE - shake_range;
      shake_right_bound = MAX_RIGHT_SHAKE;
      cur_shake_val = MAX_RIGHT_SHAKE - shake_range/2;
    }
    restart_routine_flag = false;
  }
  //write value and increment
  headServo.write(cur_shake_val);    
  cur_shake_val += shake_speed;
    
  // Check for limits and reverse direction if needed      
  if (num_shakes >= total_shakes)
  {
    cur_routine_index = NORMAL;
    fade_amount = NORMAL_FADE_AMOUNT;
  }
  if ((cur_shake_val >= shake_right_bound) || (cur_shake_val <= shake_left_bound))
  {
   shake_speed = -shake_speed; 
   cur_shake_val += shake_speed;
   num_shakes += 1;
  }    
}

void setup() 
{ 
  if (DEBUG) { Serial.begin(115200); }
  
  headServo.attach(9);  // attaches the servo on pin 9 to the servo object 
  attachInterrupt(0, leftButtonISR, RISING);
  attachInterrupt(1, rightButtonISR, RISING);
  pinMode(LEFT_EYE_PIN, OUTPUT);
  pinMode(RIGHT_EYE_PIN, OUTPUT);
  head_pot_val = analogRead(head_pot_pin);
  brightness = LOW_BRIGHTNESS+1;
  fade_dir = 1; // positive direction starting from low bightness + 1
  fade_amount = NORMAL_FADE_AMOUNT;
} 
 
void normalBehaviour()
{  
  fade_amount = NORMAL_FADE_AMOUNT;   
  setHeadFromPot(); 
}

void loop() 
{ 
  
  if ( potValueChanged() )
  {
    cur_routine_index = NORMAL;    
    normalBehaviour();
  }
  if (cur_routine_index != NORMAL)
  { 
    shakeHead(cur_routine_index); // ASSUMES only 3 index values this will need to change if more are added
  } 
  
  pulseEyes();
  
  delay(15);                           // waits for the servo to get there 
} 
