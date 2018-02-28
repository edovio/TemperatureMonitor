/**********************************************************************************************************************
 * Sketch created by Edoardo Viola - GitHub Account: @edovio - Email: edoardo@edovio.in - http://edovio.in            *
 *            This code is under Creative Commons - Attribution 4.0 International (CC BY 4.0)                         *
 *              For any information about the project or to contribute follow it on GitHub                            *                 
 * *******************************************************************************************************************/


//Library for the DHT11 sensor
#include <Adafruit_Sensor.h>
#include "DHT.h"

//Library for ESP8266 connection via WiFi
#include <ESP8266WiFi.h>


//PIN Conversion per ESP8266
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1

//Sensors connected
#define buzzer D2
#define led D3

//Strings to use Bootstrap via Internet
String bootstrap = "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css' integrity='sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm' crossorigin='anonymous'>";
String jQuery = "<script src='https://code.jquery.com/jquery-3.2.1.slim.min.js' integrity='sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN' crossorigin='anonymous'></script>";
String cdnjs = "<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js' integrity='sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q' crossorigin='anonymous'></script>";
String bootstrapJs = "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js' integrity='sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl' crossorigin='anonymous'></script>";

//Information to make the WiFi Access in your WLAN
const char* ssid = "YOUR WiFi SSID";
const char* password = "YOUR WiFi PASSWORD";

//Variable for the Temperature sensor
int h_index = -100;
float t_temp = -100;
float h_temp = -100;
float t = -100;
float h = -100;

//Define about the type and PIN Connection for the Temperature Sensor
#define DHTPIN D1
#define DHTTYPE DHT11

//Temperature Sensor Inizialization
DHT dht(DHTPIN, DHTTYPE);

//Server Inizialization
WiFiServer server(80);

void setup() {
  //Inform the Board about the Input and Output PIN
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(DHTPIN, INPUT);
  Serial.begin(115200);
  delay(10);

  //Print information about the WLAN connection
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  //Wait to have a WLAN connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  
//Inizialization of the variable with the data sent from the Sensor with a check of value
  //If the value is not good the control don't update the value shown in the Web GUI
  if((!isnan(dht.readTemperature()))&& (!isnan(dht.readHumidity()))){
    t = dht.readTemperature();
    h = dht.readHumidity();
    t_temp = t;
    h_temp = h;
  }
  else{
    t = t_temp;
    h = h_temp;
  }

  //Evaluation of the Humidex Index
  h_index = t + (0.5555 * ( 0.06 * h * (pow(10, 0.03 * t)) - 10));

  //Print of the Value on to the Serial Port 
  Serial.print("Temperature = ");
  Serial.print(t);
  Serial.println(" °C");
  Serial.print("Humidity = ");
  Serial.print(h);
  Serial.println(" %");

  //Recall at the function Alarm
  alarm(t, h, h_index);

  //Recall at the function to create the WebPage
  createServer(t, h, h_index);
}

/**********************************************************************************************************
*   This function enable the buzzer and the LED with an intermittent sound and light at 440 Hz (A Tone)   *
*                                     If are true some condition                                          *
**********************************************************************************************************/

void alarm(float t, float h, float h_index) {
  //If the temperature is under 16 °C then Turn on the Led and buzzer intermittents
  if (t <= 16.00) {
    digitalWrite(led, HIGH);
    tone(buzzer, 440, 100);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
    digitalWrite(led, HIGH);
  }
  else {
    digitalWrite(led, LOW);
    noTone(buzzer);
  }

  //If the temperature is above of 55 °C then Turn on the Led and buzzer intermittents
  if (t >= 55.00) {
    digitalWrite(led, HIGH);
    tone(buzzer, 440, 100);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
    digitalWrite(led, HIGH);
  }
  else {
    digitalWrite(led, LOW);
    noTone(buzzer);
  }

  /* If Humidex Index is >= 40 and the temperature are inside the Range usefull for the Index 
    then Turn on the Led and buzzer intermittents */
  if (h_index >= 40 && t >= 20.00 && t <= 55.00) {
    digitalWrite(led, HIGH);
    tone(buzzer, 440, 100);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
    digitalWrite(led, HIGH);
  }
  else {
    digitalWrite(led, LOW);
    noTone(buzzer);
  }
}

/**********************************************************************************************************
*                       This function produce an HTML page that use Bootstrap 4.0                         *
*                                      when a Client is connected                                         *
*            moreover create dynamically alert in function of the Temperature read from the sensor        *
**********************************************************************************************************/

void createServer(float t, float h, int h_index) {
  WiFiClient client = server.available();
  delay(1);
  //if don't exist a client then print a messagge in the Serial Port
  if (!client) {
    Serial.println("No Client connected");
    delay(1000);
  }
  //else print a messagge in the Serial Port, build the HTTP Request and the HTML page
  else {
    Serial.println("E' connesso un Client");

    //Write HTTP Request
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  do not forget this one
    
    //Start to Write the HTML
    client.println("<!DOCTYPE HTML>");
    //Change the Lang in base of your
    client.println("<html lang='it-IT'>");
    
    //HEAD  of HTML Page
    client.println("<head>");
    client.println("<meta http-equiv='refresh' content='5'>");
    client.println("<meta charset='utf-8'>");
    client.println("<title>Monitor Temperatura</title>");
    client.println(bootstrap);
    client.println(jQuery);
    client.println(cdnjs);
    client.println(bootstrapJs);
    client.println("</head>");

    //BODY of HTML Page
      client.println("<body>");

    //NAVBAR 
    client.println("<div class='navbar navbar-expand-lg navbar-dark bg-dark'>");
    client.println("<a class='navbar-brand' href='#'>Monitor Temperatura</a>");
    client.println("</div>");
    client.println("<hr>");

    //Info Block Temperature
    //if the temperature is under 16 °C than send an Alert info
    if (t <= 16.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Attention the Temperature is Low!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }
    //if the temperature is above of 55 °C than send an Alert info
    if (t >= 55.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Danger! the Temperature is to High!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }

    //Info Block Humidity
    //if the Humidity is less than, or equal at 40% and the Temperature more than 25 °C than send an Alert info
    if (h <= 40.00 && t > 25.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Attention at the Humidity!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }
    //else if the Humidity is more than 65% and the Temperature more than 25 °C than send an Alert info
    else if (h > 65 && t > 25.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Attention at the Humidity!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }

    //Info Block Humidex
    //if the Humidex is more than 40 and the temperature between 20 °C and 55 °C than send a Danger info
    if (h_index >= 40 && t >= 20.00 && t <= 55.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Danger! Avoid efforts, look for a cool, dry place nearby!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }

    //CONTENT
    client.println("<div class='container-fluid'>");

    //Temperature Block (Column 1)
    client.println("<div class ='row justify-content-around'>");
    client.println("<div class='col-5 alert alert-info'>");
    client.println("<p class = 'text-centered'> Actual Temperature: ");
    client.println(t);
    //Badge for Temperature
    //if the Temperature is under 16 °C  than a Danger Badge
    if (t <= 16.00) {
      client.println("Gradi <span class = 'badge badge-danger'>Danger</span> </p> ");
    }
    //if the Temperature is between 16 °C  and 20 °C than an Attention Badge
    if (t > 16.00 && t < 20.00) {
      client.println(" Gradi <span class = 'badge badge-warning'>Attention</span> </p> ");
    }
    //if the Temperature is above of 35 °C  than a Danger Badge
    if ( t > 35.00) {
      client.println(" Gradi <span class = 'badge badge-danger'>Danger</span> </p> ");
    }
    //if the Temperature is between 20 °C  and 25 °C than an Good Badge
    if (t >= 20.00 && t < 25.00) {
      client.println("Gradi <span class = 'badge badge-success'>Good</span> </p> ");
    }
    //if the Temperature is between 25 °C  and 35 °C than an Attention Badge
    if (t >= 25.00 && t <= 35.00) {
      client.println(" Gradi <span class = 'badge badge-warning'>Attention</span> </p> ");
    }
    client.println("</div>");

    //Humidity Block (Column 2)
    client.println("<div class='col-5 alert alert-info'>");
    client.println("<p class = 'text-centered'> Actual Humidity: ");
    client.println(h);
    //if the Humidity is equal or under 40 % than an Attention Badge
    if (h <= 40.00) {
      client.println(" % <span class = 'badge badge-warning'>Attention</span> </p> ");
    }
    //if the Humidity is between 40% and 65% than a Good Badge
    if (h > 40.00 && h < 65.00) {
      client.println(" % <span class = 'badge badge-success'>Good</span> </p> ");
    }
    //if the Humidity is above of 65%  than an Attention Badge
    else if ( h >= 65.00) {
      client.println(" % <span class = 'badge badge-warning'>Attention</span> </p> ");
    }
    client.println("</div>");
    client.println("</div>");

    client.println("<hr>");

    //Humidex Index Block
    client.println("<div class ='row justify-content-around'>");
    client.println(" <div class ='col-11 alert alert-info'>");
    client.println("<p class = 'text-centered'> Humidex Index: ");
    client.println(h_index);
    //If Humidex is under 27 than a Good Badge
    if (h_index < 27) {
      client.println("<span class = 'badge badge-success'>Good</span> </p> ");
    }
    //if the Humidex is between 27 and 30 than an Attention Badge
    if (h_index >= 27 && h_index < 30) {
      client.println("<span class = 'badge badge-warning'>Attention</span></p> ");
    }
    //if the Humidex is between 30 and 40 than an Attention Badge
    if (h_index >= 30 && h_index < 40) {
      client.println("<span class = 'badge badge-warning'>Attenzione</span></p> ");
    }
    //if the Humidex is between 40 and 55 than a Danger Badge
    if (h_index >= 40 && h_index < 55) {
      client.println("<span class = 'badge badge-danger'>Danger</span> </p> ");
    }
    //if the Humidex is above or equal 55 than a Danger Badge
    if (h_index >= 55) {
      client.println("<span class = 'badge badge-danger'>Pericolo</span> </p> ");
    }
    client.println("</div>");
    client.println("</div>");

    client.println("</div>");
    client.println("<hr>");

    //FOOTER
    client.println("<div class='container-fluid'>");
    client.println("<footer> <h5 class='text-centered'>Proudly made by <a href='mailto:edovio@gmail.com'>Edoardo Viola</a></h5></footer>");
    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
    delay(1);
    delay(800);
  }
}


