// Include application, user and local libraries

#ifndef __CC3200R1M1RGC__
#include <SPI.h>                // Do not include SPI for CC3200 LaunchPad
#endif
#include <WiFi.h>
#include "HX711.h"
#include "OneMsTaskTimer.h"

HX711 cell(3, 2);

int laserPin = 7;
int ldr=6;
int buzz=10;
int count=-1
int countpmin=-1;
int cn=0;
float volpmin;


void refresh();
OneMsTaskTimer_t myTask1 ={10000,refresh, 0, 0};
// Define variables and constants
char wifi_name[] = "SMART_DRIPS";
char wifi_password[] = "launchpad";

WiFiServer myServer(80);
uint8_t oldCountClients = 0;
uint8_t countClients = 0;

int calib;
long caliberation=0;
long initialWeight=0;

// Add setup code
void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("*** LaunchPad CC3200 WiFi Web-Server in AP Mode");

    // Start WiFi and create a network with wifi_name as the network name
    // with wifi_password as the password.
    Serial.print("Starting AP... ");
    while (WiFi.localIP() == INADDR_NONE)
    {
        // print dots while we wait for the AP config to complete
        Serial.print('.');
        delay(300);
    }
    Serial.println("DONE");

    Serial.print("LAN name = ");
    Serial.println(wifi_name);

    Serial.print("WPA password = ");
    Serial.println(wifi_password);



    pinMode(8,OUTPUT);
    pinMode(4,OUTPUT);
    pinMode(5,OUTPUT);
    digitalWrite(8,LOW);
    pinMode (laserPin, OUTPUT);
    pinMode(ldr, INPUT);
    pinMode(buzz,OUTPUT);
    digitalWrite(buzz,LOW);
    OneMsTaskTimer::add(&myTask1);

    IPAddress ip = WiFi.localIP();
    Serial.print("Webserver IP address = ");
    Serial.println(ip);

    Serial.print("Web-server port = ");
    myServer.begin();                           // start the web server on port 80
    Serial.println("80");
    Serial.println("Dont keep any weights . please wait for initialization . . . . .");

    for(int i=0;i<25;i++){
        calib=cell.read();
        // Serial.println("wait for intialization . . . . .");
        // Serial.println((calib-8151200)/8345.0f*31);
        caliberation=((calib-8178386)/7930.0f*35);
    }

    Serial.println("Initialization successful. ");
    Serial.println("Keep glucose bottle and wait for some time ........");
    delay(1000);

    for(int i=0;i<25;i++)
    {
        delay(500);
        calib=cell.read();
        initialWeight=(((calib-8178386)/7930.0f*35)-caliberation);
        // Serial.println(initialWeight);
    }

    if(initialWeight==0) //to get ridoff divide by 0 error
        initialWeight=1;

    Serial.print("Initial weight = ");
    Serial.println(initialWeight);
    OneMsTaskTimer::start();
    Serial.println();
}


long val = 0;
long reading=0;
float percentage=0;
String Status;
// Add loop code
void loop()
{
int sen=0;
//analogReadResolution(10);
digitalWrite (laserPin, HIGH);  
val = cell.read();
int sensorValue = analogRead(ldr);
if(sensorValue>4000)
sen=1;
else 
sen=0;
if(sen==1)
{
    count++;
    countpmin++;
    delay(50);
}
Serial.println(sen);
// delay(100);
// Serial.println(count);
delay(100);
countClients = WiFi.getTotalDevices();

// Did a client connect/disconnect since the last time we checked?
if (countClients != oldCountClients)
{
    if (countClients > oldCountClients)
    {  
        //            digitalWrite(RED_LED, !digitalRead(RED_LED));
        Serial.println("Client connected to AP");
        for (uint8_t k = 0; k < countClients; k++)
        {
            Serial.print("Client #");
            Serial.print(k);
            Serial.print(" at IP address = ");
            Serial.print(WiFi.deviceIpAddress(k));
            Serial.print(", MAC = ");
            Serial.println(WiFi.deviceMacAddress(k));
            Serial.println("CC3200 in AP mode only accepts one client.");
        }
    }
    else
    {  // Client disconnect
    //            digitalWrite(RED_LED, !digitalRead(RED_LED));
    Serial.println("Client disconnected from AP.");
    Serial.println();
    }
    oldCountClients = countClients;
}

WiFiClient myClient = myServer.available();

if (myClient)
{                             // if you get a client,
    Serial.println(". Client connected to server");           // print a message out the serial port
    char buffer[150] = {0};                 // make a buffer to hold incoming data
    int8_t i = 0;
    while (myClient.connected())
    {            // loop while the client's connected
        if (myClient.available())
        {             // if there's bytes to read from the client,
        char c = myClient.read();             // read a byte, then
        //   Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

        // if the current line is blank, you got two newline characters in a row.
        // that's the end of the client HTTP request, so send a response:
            if (strlen(buffer) == 0)
            {
                myClient.println("HTTP/1.1 200 OK");
                myClient.println("Content-type:text/html");
                myClient.println("refresh : 30;"); //refresh after every 15 second 
                myClient.println();
                myClient.print("<br/><b> Glucose left in the bottle = ");
                myClient.println(reading);
                myClient.println(" gms </b></br>");
                myClient.println("");
                myClient.println("<br/><b> Glucose left in the bottle =");
                myClient.println(percentage);
                myClient.println(" % </b></br>");
                myClient.print("<br/><b> Status =");
                myClient.print(Status);
                myClient.print("</b></br>");
                myClient.println("<br/><b> Glucose count per minute =");
                myClient.println(cn);
                myClient.println("  </b></br>");
                myClient.println("<br/><b> Volume per minute =");
                myClient.println(volpmin);
                myClient.println(" ml/min </b></br>");


                myClient.println();


                // the content of the HTTP response follows the header:


                // The HTTP response ends with another blank line:
                myClient.println();
                // break out of the while loop:
                break;
            }
            else
            {      // if you got a newline, then clear the buffer:
                memset(buffer, 0, 150);
                i = 0;
            }
        }
        else if (c != '\r')
        {    // if you got anything else but a carriage return character,
            buffer[i++] = c;      // add it to the end of the currentLine
        }
        Serial.println();
        String text = buffer;
        // Check to see if the client request was "GET /H" or "GET /L":
        if (text.endsWith("GET /UP"))
        {
            digitalWrite(8,HIGH); //1-->+ 2--> -ve
            digitalWrite(4,HIGH); //4-> 1n1
            digitalWrite(5,LOW);//5->in2
            delay(1000); //anticlockwise 1 sec
            digitalWrite(8,LOW);
        }
        if (text.endsWith("GET /DOWN"))
        {
            digitalWrite(8,HIGH); //1-->+ 2--> -ve
            digitalWrite(4,LOW); //4-> 1n1
            digitalWrite(5,HIGH);//5->in2
            delay(1000); //anticlockwise 1 sec
            digitalWrite(8,LOW);
        }

    }
    }

        // close the connection:
    myClient.stop();
    Serial.println(". Client disconnected from server");
    Serial.println();
    }
}


void refresh()
{
    reading=(((val-8178386)/7930.0f*35)-caliberation); //caliberation 
    percentage=((float)reading/initialWeight)*100;    //calculate %
    delay(10);
    //percentage=10;
    Serial.print("weight :  ");
    Serial.print( reading); 
    Serial.println(" gms");
    Serial.print("percentage left :  ");
    Serial.print( percentage); 
    Serial.println(" %");
    if(percentage<20.0)
    {  
        Status = "CRITICAL";
    // buzzer();
    } else 
    {
        Status="SAFE";
    }
    Serial.print("Count per minute : ");
    Serial.println(countpmin*6);
    cn=countpmin*6;
    Serial.print("volume per minute : ");
    volpmin=countpmin*6*0.05;
    Serial.println(volpmin);

    countpmin=0;
}

void buzzer()
{
    for(int m=0;m<50;m++)
    {
        digitalWrite(10,HIGH);
        delay(50);
        digitalWrite(10,LOW);
        delay(50);
    }
}