//Imports
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Servo.h>

//Defining our accelerometer
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

//Defining our servo
Servo myservo;
int servoloc = 90;

//Defining our connections
int north_pin = 2;
int east_pin = 3;
int west_pin = 4;
int south_pin = 5;

const int dirs[4] = {north_pin, east_pin, west_pin, south_pin};
int wrong_direction_time;

int green_upper_pin = 6;
int green_middle_pin = 7;
int red_barrel_pin = 8;
int yellow_barrel_pin = 9;
int green_lower_pin = 10;

const int barrel[5] = {green_lower_pin, green_middle_pin, green_upper_pin, yellow_barrel_pin, red_barrel_pin};
int barrel_time;
int barrel_index;
int barrel_emptied = 0;

const int pot_sensor = A0;

//Defining accel variables
float previousX = 0;
float currentX;
float previousY = 0;
float currentY;
float pot_value;

//Defining flex variables
float currentPot;
int barrel_countr;

void setup()
{
  Serial.begin(9600);
  accel.begin();

  //LEDs for the power director
  pinMode(north_pin, OUTPUT);
  pinMode(east_pin, OUTPUT);
  pinMode(west_pin, OUTPUT);
  pinMode(south_pin, OUTPUT);

  //LEDs for the barrel
  pinMode(red_barrel_pin, OUTPUT);
  pinMode(yellow_barrel_pin, OUTPUT);
  pinMode(green_upper_pin, OUTPUT);
  pinMode(green_middle_pin, OUTPUT);
  pinMode(green_lower_pin, OUTPUT);

  pinMode(pot_sensor, INPUT);
  myservo.attach(11);
  myservo.write(120);
  delay(300);
  myservo.write(90);
  establishContact();
}

void establishContact()
{
  while (Serial.available() <= 0) {
    Serial.println("A");   // send a capital A
    delay(300);
  }
}

//Filter for our accelerometer
float lowpass(float previous, float current)
{
  return (0.8 * previous + 0.2 * current);
}

void reading_accel()
{
  sensors_event_t accelEvent;
  accel.getEvent(&accelEvent);
  currentX = lowpass(previousX, accelEvent.acceleration.x);
  previousX = currentX;

  currentY = lowpass(previousY, accelEvent.acceleration.y);
  previousY = currentY;
  //Serial.print(currentX);
  //Serial.print("   ");
  //Serial.println(currentY);
  delay(50);
}

void reading_pot()
{
  pot_value = analogRead(pot_sensor);
  currentPot = map(pot_value, 950, 700, 0, 100);
}

void random_power_failure(unsigned long theTime)
{
  int index = random(4);
  dirs_nolight();
  digitalWrite(dirs[index], HIGH);
  wrong_direction_time = 1;
  while (true)
  {
    if (millis() > theTime + 5000) {
      Serial.println('c');
    } else {
      reading_accel();
      wrong_direction_time += 1;

      if (index == 0)
      {
        if (currentX > 2.0)
        {
          dirs_nolight();
          break;
        }
      } else if (index == 1)
      {
        if (currentY < 1.6)
        {
          dirs_nolight();
          break;
        }
      } else if (index == 2)
      {
        if (currentY > 3.3)
        {
          dirs_nolight();
          break;
        }
      } else {
        if (currentX < -1.0)
        {
          dirs_nolight();
          break;
        }
      }
    }
  }
}

void dirs_nolight()
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(dirs[i], LOW);
  }
}

void barrel_nolight()
{
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(barrel[i], LOW);
  }
}

void barrel_filled(int amount)
{
  barrel_nolight();
  for (int i = 0; i < amount; i++)
  {
    digitalWrite(barrel[i], HIGH);
  }
}

void fill_barrel(int value)
{
  barrel_time += 24;
  if (barrel_time < 10000)
  {
    barrel_index = map(barrel_time, 0, 10000, 1, 6);
  } else {
    barrel_time = 0;
  }
  barrel_filled(barrel_index);

  if (barrel_index == 5) {
    Serial.println('c');
  }

  if (barrel_index == 4) {
    reading_pot();

    if (currentPot > 70)
    {
      barrel_time = 0;
      barrel_nolight();
      barrel_emptied += 1;
      if (barrel_emptied == 4) {
        Serial.println('c');
      }
      barrel_index = 1;
    }
  }
}


void termo(int where)//servoloc 90
{
  int newservoloc = servoloc + where;

  if (where > 0) {
    for (int i = servoloc; i < newservoloc; i++) {
      myservo.write(i);
      delay(25);
    }
  }
  if (where < 0) {
    for (int i = servoloc; i > newservoloc; i--) {
      myservo.write(i);
      delay(25);
    }
  }
  servoloc = newservoloc;
  if ((servoloc < 40 and servoloc > 35) or (servoloc > 140 and servoloc < 145)) {
    Serial.println('d');
  }

  if (servoloc < 0 or servoloc > 180) {
    Serial.println('c');
  }
}
int gaugecheck(int value)
{
  if (value != 0) {
    value = value * 2 + 5;
    return (value);
  }
  return (value);
}


int gage = 0;
int gageval = 0;
int counter = 0;
int direction_counter = 0;
unsigned long myTime;
unsigned long timecount = 0;
int temp_dir;
char val;


void loop()
{
  int barrelval = 0;
  if (Serial.available() > 0) {
    val = Serial.read();

    if (val == 'a') {
      temp_dir = 1;
    } else if (val == 'b') {
      temp_dir = -1;
    }

    val = Serial.read();

    if (val != 'A' or val != 'a' or val != 'b' or val != '0' or val != 'c' or val != 'd') {
      gageval = val - '0';
      barrelval = gageval;
      gageval = gageval * temp_dir;

      if (counter > 3) {
        if (gageval <= 8 and gageval >= -8)
        {
          barrelval = abs(gageval);
          gageval = gaugecheck(gageval);
          termo(gageval);
          counter = 0;
          gageval = 0;
        }

      }
      while (Serial.available() > 0) {
        Serial.read();
      }
    }
  }

  if (millis() > 10000 + timecount)
  {
    random_power_failure(millis());
    timecount += 10000;
  }
  reading_pot();
  fill_barrel(barrelval);
  counter += 1;
  gageval = 0;
  delay(50);
}
