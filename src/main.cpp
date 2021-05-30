#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

#include <Arduino.h>
#include <OneWire.h>
#include <WS2812FX.h>
#include <ETH.h>
#include <MQTT.h>

OneWire ds_1(13);                   
OneWire ds_2(14); 
OneWire ds_3(16); 
OneWire ds_4(15); 

#define LED_COUNT 4
#define LED_PIN 5

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ400);

MQTTClient mqttclient;
WiFiClient wificlient;

unsigned long lastMillis = 0;

static bool eth_connected = false;

void PrintTwoDigitHex (byte b, boolean newline)
{
  //Serial.print(b / 16, HEX);
  //Serial.print(b % 16, HEX);
  //if (newline) 
  //Serial.println();
}

    uint32_t Color(byte g, byte r, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}


void connect() {
  //Serial.print("checking wifi...");

  delay(5000);


  //Serial.print("\nconnecting...");
  while (!mqttclient.connect("schluessel", "Euer Benutzername", "Euer Passwort")) {
    //Serial.print(".");
    delay(1000);
  }

  //Serial.println("\nconnected!");

  mqttclient.subscribe("/schluesselbrett/LED1");
  mqttclient.subscribe("/schluesselbrett/LED2");
  mqttclient.subscribe("/schluesselbrett/LED3");
  mqttclient.subscribe("/schluesselbrett/LED4");

  mqttclient.subscribe("/schluesselbrett/helligkeit");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  //Serial.println("incoming: " + topic + " - " + payload);


  if (topic == "/schluesselbrett/LED1")
  {

    // Get rid of '#' and convert it to integer
    int number = (int) strtol( &payload[1], NULL, 16);

    // Split them up into r, g, b values
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

//ws2812fx.setPixelColor(0, r, g, b);
ws2812fx.setPixelColor(0, Color(r,g,b));
 ws2812fx.show();
   // ws2812fx.setSegment(0,  0, 0, FX_MODE_FADE, Color(r,g,b),  2000, false);

    //ws2812fx.setColor(g, r, b);

  }

  if (topic == "/schluesselbrett/LED2")
  {

    // Get rid of '#' and convert it to integer
    int number = (int) strtol( &payload[1], NULL, 16);

    // Split them up into r, g, b values
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

ws2812fx.setPixelColor(1,Color(r,g,b));
ws2812fx.show();

    //ws2812fx.setSegment(0,  1, 1, FX_MODE_FADE, Color(r,g,b),  2000, false);

  }

    if (topic == "/schluesselbrett/LED3")
  {

    // Get rid of '#' and convert it to integer
    int number = (int) strtol( &payload[1], NULL, 16);

    // Split them up into r, g, b values
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;


ws2812fx.setPixelColor(2, Color(r,g,b));
ws2812fx.show();


   //ws2812fx.setSegment(0,  2, 2, FX_MODE_FADE,  Color(r,g,b),  2000, false);

  }


    if (topic == "/schluesselbrett/LED4")
  {

    // Get rid of '#' and convert it to integer
    int number = (int) strtol( &payload[1], NULL, 16);

    // Split them up into r, g, b values
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

    ws2812fx.setPixelColor(3, Color(r,g,b));
    ws2812fx.show();


    //ws2812fx.setSegment(0,  3, 3, FX_MODE_FADE, Color(r,g,b),  2000, false);

  }

  if (topic == "/schluesselbrett/helligkeit")
  {
   int helligkeit = payload.toInt();
   ws2812fx.setBrightness(helligkeit);
    ws2812fx.show();

  }



}


void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      //Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      //Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      //Serial.print("ETH MAC: ");
      //Serial.print(ETH.macAddress());
      //Serial.print(", IPv4: ");
      //Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        //Serial.print(", FULL_DUPLEX");
      }
      //Serial.print(", ");
      //Serial.print(ETH.linkSpeed());
      //Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      //Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      //Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}


void setup()
{
  //Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  delay(250);

  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x000000);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  mqttclient.begin("192.168.178.X", wificlient);
  mqttclient.onMessage(messageReceived);

  connect();

}

void loop()
{
  byte i;           // This is for the for loops
  boolean present_1,present_2,present_3,present_4;  // device present var
  byte data[8];     // container for the data from device
  byte crc_calc;    //calculated CRC
  byte crc_byte;    //actual CRC as sent by DS2401

 //ws2812fx.service();
 mqttclient.loop();

  if (!mqttclient.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 3000) {
    lastMillis = millis();



    //1-Wire bus reset, needed to start operation on the bus,
    //returns a 1/TRUE if presence pulse detected
    present_1 = ds_1.reset();
    present_2 = ds_2.reset();
    present_3 = ds_3.reset();
    present_4 = ds_4.reset();

    if (present_1 == true)
    {
      //Serial.println("---------- Device present ----------");
      ds_1.write(0x33);  //Send Read data command

      data[0] = ds_1.read();
      //Serial.print("Family code: 0x");
      PrintTwoDigitHex (data[0], 1);

      //Serial.print("Hex ROM data: ");
      for (i = 1; i <= 6; i++)
      {
        data[i] = ds_1.read(); //store each byte in different position in array
        PrintTwoDigitHex (data[i], 0);
        //Serial.print(" ");
      }
      //Serial.println();

      crc_byte = ds_1.read(); //read CRC, this is the last byte
      crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

      //Serial.print("Calculated CRC: 0x");
      PrintTwoDigitHex (crc_calc, 1);
      //Serial.print("Actual CRC: 0x");
      PrintTwoDigitHex (crc_byte, 1);

      
       String byteString = String(crc_byte);

        mqttclient.publish("/schluesselbrett/1", byteString );
      

  

    }


    else //Nothing is connected in the bus
    {
      //Serial.println("xxxxx Nothing connected xxxxx");


      mqttclient.publish("/schluesselbrett/1", "NONE");
    }


      if (present_2 == true)
    {
      //Serial.println("---------- Device present ----------");
      ds_2.write(0x33);  //Send Read data command

      data[0] = ds_2.read();
      //Serial.print("Family code: 0x");
      PrintTwoDigitHex (data[0], 1);

      //Serial.print("Hex ROM data: ");
      for (i = 1; i <= 6; i++)
      {
        data[i] = ds_2.read(); //store each byte in different position in array
        PrintTwoDigitHex (data[i], 0);
        //Serial.print(" ");
      }
      //Serial.println();

      crc_byte = ds_2.read(); //read CRC, this is the last byte
      crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

      //Serial.print("Calculated CRC: 0x");
      PrintTwoDigitHex (crc_calc, 1);
      //Serial.print("Actual CRC: 0x");
      PrintTwoDigitHex (crc_byte, 1);

      
       String byteString = String(crc_byte);

        mqttclient.publish("/schluesselbrett/2", byteString );
      

  

    }


    else //Nothing is connected in the bus
    {
      //Serial.println("xxxxx Nothing connected xxxxx");


      mqttclient.publish("/schluesselbrett/2", "NONE");
    }


if (present_3 == true)
    {
      //Serial.println("---------- Device present ----------");
      ds_3.write(0x33);  //Send Read data command

      data[0] = ds_3.read();
      //Serial.print("Family code: 0x");
      PrintTwoDigitHex (data[0], 1);

      //Serial.print("Hex ROM data: ");
      for (i = 1; i <= 6; i++)
      {
        data[i] = ds_3.read(); //store each byte in different position in array
        PrintTwoDigitHex (data[i], 0);
        //Serial.print(" ");
      }
      //Serial.println();

      crc_byte = ds_3.read(); //read CRC, this is the last byte
      crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

      //Serial.print("Calculated CRC: 0x");
      PrintTwoDigitHex (crc_calc, 1);
      //Serial.print("Actual CRC: 0x");
      PrintTwoDigitHex (crc_byte, 1);

      
       String byteString = String(crc_byte);

        mqttclient.publish("/schluesselbrett/3", byteString );
      

  

    }

    else //Nothing is connected in the bus
    {
      //Serial.println("xxxxx Nothing connected xxxxx");


      mqttclient.publish("/schluesselbrett/3", "NONE");
    }


 if (present_4 == true)
    {
      //Serial.println("---------- Device present ----------");
      ds_4.write(0x33);  //Send Read data command

      data[0] = ds_4.read();
      //Serial.print("Family code: 0x");
      PrintTwoDigitHex (data[0], 1);

      //Serial.print("Hex ROM data: ");
      for (i = 1; i <= 6; i++)
      {
        data[i] = ds_4.read(); //store each byte in different position in array
        PrintTwoDigitHex (data[i], 0);
        //Serial.print(" ");
      }
      //Serial.println();

      crc_byte = ds_4.read(); //read CRC, this is the last byte
      crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

      //Serial.print("Calculated CRC: 0x");
      PrintTwoDigitHex (crc_calc, 1);
      //Serial.print("Actual CRC: 0x");
      PrintTwoDigitHex (crc_byte, 1);

      
       String byteString = String(crc_byte);

        mqttclient.publish("/schluesselbrett/4", byteString );
      

  

    }



    else //Nothing is connected in the bus
    {
      //Serial.println("xxxxx Nothing connected xxxxx");


      mqttclient.publish("/schluesselbrett/4", "NONE");
    }

    if (present_1 == false && present_2 == false && present_3 == false && present_4 == false)
    {
      mqttclient.publish("/schluesselbrett/nothing", "nothing" );
    }

    else
    {
      mqttclient.publish("/schluesselbrett/nothing", "atleastone" );
    }
    


  }


}

