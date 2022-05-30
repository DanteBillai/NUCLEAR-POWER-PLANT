// 2A: Shared drawing canvas (Server)

import processing.net.*;
import processing.serial.*; //import the Serial library
Serial myPort;  //the Serial port object
String val;
// since we're doing serial handshaking,
// we need to check if we've heard from the microcontroller
boolean firstContact = false;

Server s;
Client c;
String input;
int data[];

void setup() {
  //  initialize your serial port and set the baud rate to 9600
  myPort = new Serial(this, Serial.list()[4], 9600);
  myPort.bufferUntil('\n');
  s = new Server(this, 8000);  // Start a simple server on a port
}
void draw() {
  c = s.available();
  if (c != null) {
    input = c.readString();
    println(input);
    //input = trim(input);
    myPort.write(input);
  }
}

void serialEvent( Serial myPort)
{
  val = myPort.readStringUntil('\n');
  if (val != null) {

    val = trim(val);
    print(val);
    
    if (firstContact == false) {
      if (val.equals("A")) {
        myPort.clear();
        firstContact = true;
        myPort.write("A");
        //println("contact");
        val = null;
      }
    } else {
      s.write(val+"\n");
      c = s.available();

      if (c != null) {
        input = c.readString();
        myPort.write(input);
      }
    }
  }
}
