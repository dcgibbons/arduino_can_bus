#include <Arduino.h>
#include <SoftwareSerial.h>

#include <Debug.h>
#include <Canbus.h>
#include <mcp2515.h>

// we need fundamental FILE definitions and printf declarations
#include <stdio.h>

// create a FILE structure to reference our UART output function
static FILE uartout = {
  0};

#define CANSPEED_500	1
#define CANSPEED_250    3
#define CANSPEED_125 	7

#define LED2 8
#define LED3 7

/* Define Joystick connection */
#define UP     A1
#define RIGHT  A2
#define DOWN   A3
#define CLICK  A4
#define LEFT   A5

#define SD_CS       9
#define MCP2515_CS  10

volatile uint32_t messages = 0;

void setup() 
{
  Debug.begin(115200);

  pinMode(UP,INPUT);
  pinMode(DOWN,INPUT);
  pinMode(LEFT,INPUT);
  pinMode(RIGHT,INPUT);
  pinMode(CLICK,INPUT);

  digitalWrite(UP, HIGH);       /* Enable internal pull-ups */
  digitalWrite(DOWN, HIGH);
  digitalWrite(LEFT, HIGH);
  digitalWrite(RIGHT, HIGH);
  digitalWrite(CLICK, HIGH);

  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(3, OUTPUT);

  pinMode(MCP2515_CS, OUTPUT);

  Serial.println("CAN-Bus Passive Listener");  /* For debug use */

  while(1)
  {
    if (digitalRead(DOWN) == 0)
    {
      Serial.println("Starting!");
      break;
    }
    delay(100);
  }

  for (int tries = 0; tries < 10; tries++)
  {  
    PRINT("Attempting #%d to initialize CAN-Bus\n", tries+1);

    char ret = mcp2515_init(CANSPEED_125);
    if (ret)
    {
      Serial.println("mcp2515 CAN-Bus controller successfully initialized\n");
      digitalWrite(LED2, HIGH);
      break;
    }
    else
    {
      Serial.println("mcp2515 CAN-Bus controller could not initialize\n");
    }
    delay(250);
  }
}

void print_can_message(tCAN *message)
{
  char buffer[80];

  uint8_t length = message->header.length;

  int n = sprintf(&buffer[0], "id:0x%3x len:%d rtr:%d ", message->id, length, message->header.rtr);

  if (!message->header.rtr) {
    n += sprintf(&buffer[n], "data:");

    for (uint8_t i = 0; i < length; i++) {
      n += sprintf(&buffer[n], "0x%02x ", message->data[i]);
    }

    for (uint8_t i = 0; i < length; i++) {
      n += sprintf(&buffer[n], "%c", isprint(message->data[i]) ? (const char)message->data[i] : '.');
    }
  }
  if ((messages % 1000) == 0)
  {
    PRINT("%lu messages received, %s\n", messages, buffer);
  }
  file.println(buffer);
  PRINT("%s\n", buffer);
}

static uint32_t timer = millis();

void loop() 
{  
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();
  
  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) 
  {
    timer = millis();
    PRINT("%d total messages received\n", messages);
  }
  
  while (mcp2515_check_message())
  {
    tCAN message;
    if (mcp2515_get_message(&message))
    {
      messages++;
      digitalWrite(LED3, HIGH);
      print_can_message(&message);
    }
  }
  digitalWrite(LED3, LOW);
}
