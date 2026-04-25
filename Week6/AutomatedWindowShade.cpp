// NOTE: much of the original sensor portion was adapted from the 
// temp_sensor_DC_motor.png and the Ultrasonic Distance Sensor 
// instructional video.

//I referenced https://www.tinkercad.com/things/a6vEBDWxlwn/editel
//for information about how to program the motor encoder.

#include <Encoder.h>

int motorpin_negative = 6; //Vars to hold pin numbers
int motorpin_positive = 5;

//pins for the distance sensors
const int trig_1 = 8;  //trigger_1
const int trig_2 = 9;  //trigger_2
const int trig_3 = 10; //trigger_3
const int echo_1 = 11; //Sensor_1
const int echo_2 = 12; //Sensor_2
const int echo_3 = 13; //Sensor_3

//Input pins for controlling the shade manually.
const int button = 4;  //pushbutton
const int IR_sensor = 7; //IR sensor

const int allowed_variance = 5;
const long auto_shutdown_time = 1000 * 60 * 4;
const long auto_start_time = 0;

unsigned long stored_flip_flop_time = 0;
unsigned long stored_sensor_time = 0;
bool gate_open = false;
bool flip_flop = false;

float time = 0; 


Encoder encode(2,3);
long rotator_position;
int rotator_rotations;
int high_position = 15;
int low_position = 0;
int goal_position = 10;
int goal_position_stored = -1;
int i = 0;


void setup()
{
  pinMode(trig_1, OUTPUT);
  pinMode(trig_2, OUTPUT);
  pinMode(trig_3, OUTPUT);
  pinMode(motorpin_negative, OUTPUT);
  pinMode(motorpin_positive, OUTPUT);
  pinMode(echo_1, INPUT);
  pinMode(echo_2, INPUT);
  pinMode(echo_3, INPUT);
  pinMode(IR_sensor, INPUT);
  pinMode(button, INPUT);
  Serial.begin(9600);
  goal_position = 0;
}

float sensor_distance(int sensor, int echo){
  
  float distance = -1;
  digitalWrite(sensor, LOW);
  delayMicroseconds(2);
  digitalWrite(sensor, HIGH);
  delayMicroseconds(10);
  digitalWrite(sensor, LOW);
  time = pulseIn(echo, HIGH);
  // NOTE: 148.1 from Ultrasonic Distance Sensor video.
  distance = time/148.1;
  return distance;
}

float distance_percentage(float distance, float max=132){
  float distance_percentage_value;
  distance_percentage_value = (distance / max)*100;
  return distance_percentage_value;
}

bool check_distance_sensors()
{
  	bool distances_within_expected = false;
    float distance_1 = -1;
    float distance_2 = -1;
    float distance_3 = -1;
    float distance_percentage_1 = -1;
    float distance_percentage_2 = -1;
    float distance_percentage_3 = -1;

    distance_1 = sensor_distance(trig_1, echo_1);
    distance_percentage_1 = distance_percentage(distance_1);
    
    distance_2 = sensor_distance(trig_2, echo_2);
    distance_percentage_2 = distance_percentage(distance_2);
    
    distance_3 = sensor_distance(trig_3, echo_3);
    distance_percentage_3 = distance_percentage(distance_3);
    
  	distances_within_expected = ((abs(distance_percentage_1 - distance_percentage_2) < allowed_variance) &&
      (abs(distance_percentage_1 - distance_percentage_3) < allowed_variance) &&
      (abs(distance_percentage_3 - distance_percentage_2) < allowed_variance));
  	if(!distances_within_expected)
    {
      
    	Serial.println("*Sensor variance detected!*");
    }
    return distances_within_expected;

}


void loop()
{
  
  
  // CONTROLS:
  
  Serial.print(millis());
  Serial.print("GP: ");
  Serial.println(goal_position);
  // if a set time, set goal position to high position.
  if((millis() > auto_start_time) && (millis() < auto_start_time + 30))
  {
   	goal_position = high_position;
  }
  
  // if 4 minutes have passed, set goal position to low.
  else if(millis() > auto_shutdown_time && millis() < auto_shutdown_time + 30)
  {
    goal_position = low_position;
  }
  
  
  
  // If pushed and at high position, change the goal position to low position.
  if((digitalRead(button) || !digitalRead(IR_sensor)) && flip_flop)
  {
	Serial.println("Push/IR Toggle");
    if(goal_position == high_position)
    {
      goal_position = low_position;
    }
    else
    {
      goal_position = high_position;
    }
    flip_flop = false;
    stored_flip_flop_time = millis();
  }
  
  if(millis() > stored_flip_flop_time + 1000)
  {
   	flip_flop = true; 
  }
  
  
  
  //Check to see if ~2 seconds has passed. If it has, check distances.
  if (stored_sensor_time == 0 || (millis() - stored_sensor_time > 1900))
  {
    // gate_open allows or disallows motor control below
    gate_open = check_distance_sensors();
    stored_sensor_time = millis();
  }
  

  // If all sensors read within allowed_variance, continue
  if(gate_open)
  {
    rotator_position = encode.read() / 10;
    rotator_rotations = rotator_position / 10;
    if(goal_position == rotator_rotations)
    {
      analogWrite(motorpin_positive, 0);
      analogWrite(motorpin_negative, 0);
    }
    else
    {
      if(goal_position > rotator_rotations)
      {
        Serial.println("goal > rot pos");
        analogWrite(motorpin_positive, 30);
        analogWrite(motorpin_negative, 0);
      }
      else
      {
        Serial.println("goal < rot pos");
        analogWrite(motorpin_positive, 0);
        analogWrite(motorpin_negative, 30);
      }  
    }
  }
  goal_position_stored = goal_position;
  delay(1000);
}