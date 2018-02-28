/**********************************************************************************************************************
 * Sketch created by Edoardo Viola - GitHub Account: @edovio - Email: edoardo@edovio.in - http://edovio.in            *
 *            This code is under Creative Commons - Attribution 4.0 International (CC BY 4.0)                         *
 *              For any information about the project or to contribute follow it on GitHub                            *                 
 * *******************************************************************************************************************/


//Librerie per Sensore DHT11
#include <Adafruit_Sensor.h>
#include "DHT.h"

//Libreria per ESP8266
#include <ESP8266WiFi.h>


//Conversione PIN per ESP8266
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

//Sensori Collegati
#define buzzer D2
#define led D3

//Stringhe per utilizzare Bootstrap
String bootstrap = "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css' integrity='sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm' crossorigin='anonymous'>";
String jQuery = "<script src='https://code.jquery.com/jquery-3.2.1.slim.min.js' integrity='sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN' crossorigin='anonymous'></script>";
String cdnjs = "<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js' integrity='sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q' crossorigin='anonymous'></script>";
String bootstrapJs = "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js' integrity='sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl' crossorigin='anonymous'></script>";

//Dati accesso WiFi
const char* ssid = "CASA_EDO_2";
const char* password = "Aless@ndr@_abis90";

//Variabili delle letture
int h_index = -100;
float t_temp = -100;
float h_temp = -100;
float t = -100;
float h = -100;

//Definizione tipo e PIN per il sensore Temperatura
#define DHTPIN D1
#define DHTTYPE DHT11

//Inizializzazione Sensore Temperatura
DHT dht(DHTPIN, DHTTYPE);

//Inizializzazione Server
WiFiServer server(80);

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(DHTPIN, INPUT);
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

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
  
//Inizializzazione Variabili con controllo sui valori letti dal Sensore
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

  //Calcolo indice Humidex
  h_index = t + (0.5555 * ( 0.06 * h * (pow(10, 0.03 * t)) - 10));

  //Stampo sulla seriale i valori attuali di t e h
  Serial.print("Temperature = ");
  Serial.print(t);
  Serial.println(" °C");
  Serial.print("Humidity = ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("conteggi: ");
  Serial.println(count);

  //Richiamo Funzione per gli Allarmi HW
  alarm(t, h, h_index);

  //Richiamo Funzione per l'invio dei dati
  createServer(t, h, h_index);
}

void alarm(float t, float h, float h_index) {
  //Se la temperatura è al di sotto di 16 °C, Segnale acustico e LED
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

  //Se la temperatura è al di sopra di 55 °C, Segnale acustico e LED
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

  //Se Humidex >= 40 e la temperatura è nel range allora Segnale acustico e LED
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

//Funzione che Genera pagina HTML per visualizzare in locale i dati del Dispositivo
void createServer(float t, float h, int h_index) {
  WiFiClient client = server.available();
  delay(1);
  if (!client) {
    Serial.println("Nessun Client è connesso");
    delay(1000);
  }
  else {
    Serial.println("E' connesso un Client");
    count++;
    //Esiste un Client e scrivo Pagina WEB
    //Write HTML
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  do not forget this one
    client.println("<!DOCTYPE HTML>");

    //Codice HTML

    //Change the Lang in base of your
    client.println("<html lang='it-IT'>");
    //HEAD HTML
    client.println("<head>");
    client.println("<meta http-equiv='refresh' content='5'>");
    client.println("<meta charset='utf-8'>");
    client.println("<title>Monitor Temperatura</title>");
    client.println(bootstrap);
    client.println(jQuery);
    client.println(cdnjs);
    client.println(bootstrapJs);
    client.println("</head>");

    //BODY HTML
      client.println("<body>");

    //NAVBAR
    client.println("<div class='navbar navbar-expand-lg navbar-dark bg-dark'>");
    client.println("<a class='navbar-brand' href='#'>Monitor Temperatura</a>");
    client.println("</div>");
    client.println("<hr>");

    //Blocco Info su Temperatura
    if (t <= 16.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Attenzione La temperatura e' bassa!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }
    if (t >= 55.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Pericolo! La temperatura e' troppo alta!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }

    //Blocco Info su Umidità
    if (h <= 40.00 && t > 25.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Attenzione alla Umidita'!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }
    else if (h > 65 && t > 25.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Attenzione alla Umidita'!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }

    //Blocco info su Humidex
    if (h_index >= 40 && t >= 20.00 && t <= 55.00) {
      client.println("<div class='alert alert-danger alert-dismissible fade show' role='alert'> Pericolo! Evitare sforzi, cercare un luogo fresco e asciutto nelle vicinanze!");
      client.println("<button type='button' class='close' data-dismiss='alert' aria-label='Close'>");
      client.println("<span aria-hidden='true'>&times;</span></button></div>");
    }

    //CONTENT
    client.println("<div class='container-fluid'>");

    //Blocco Temperatura (Colonna 1)
    client.println("<div class ='row justify-content-around'>");
    client.println("<div class='col-5 alert alert-info'>");
    client.println("<p class = 'text-centered'> Temperatura attuale: ");
    client.println(t);
    if (t <= 16.00) {
      client.println("Gradi <span class = 'badge badge-danger'>Pericolo</span> </p> ");
    }
    if (t > 16.00 && t < 20.00) {
      client.println(" Gradi <span class = 'badge badge-warning'>Attenzione</span> </p> ");
    }
    if ( t > 35.00) {
      client.println(" Gradi <span class = 'badge badge-danger'>Pericolo</span> </p> ");
    }
    if (t >= 20.00 && t < 25.00) {
      client.println("Gradi <span class = 'badge badge-success'>Ottimo</span> </p> ");
    }
    if (t >= 25.00 && t <= 35.00) {
      client.println(" Gradi <span class = 'badge badge-warning'>Attenzione</span> </p> ");
    }
    client.println("</div>");

    //Blocco Umidità (Colonna 2)
    client.println("<div class='col-5 alert alert-info'>");
    client.println("<p class = 'text-centered'> Umidita' attuale: ");
    client.println(h);
    if (h <= 40.00) {
      client.println(" % <span class = 'badge badge-warning'>Attenzione</span> </p> ");
    }
    if (h > 40.00 && h < 65.00) {
      client.println(" % <span class = 'badge badge-success'>Ottimo</span> </p> ");
    }
    else if ( h >= 65.00) {
      client.println(" % <span class = 'badge badge-warning'>Attenzione</span> </p> ");
    }
    client.println("</div>");
    client.println("</div>");

    client.println("<hr>");

    //Blocco Indice Humidex
    client.println("<div class ='row justify-content-around'>");
    client.println(" <div class ='col-11 alert alert-info'>");
    client.println("<p class = 'text-centered'> Indice Humidex: ");
    client.println(h_index);
    if (h_index < 27) {
      client.println("<span class = 'badge badge-success'>Ottimo</span> </p> ");
    }
    if (h_index >= 27 && h_index < 30) {
      client.println("<span class = 'badge badge-warning'>Attenzione</span></p> ");
    }
    if (h_index >= 30 && h_index < 40) {
      client.println("<span class = 'badge badge-warning'>Attenzione</span></p> ");
    }
    if (h_index >= 40 && h_index < 55) {
      client.println("<span class = 'badge badge-danger'>Pericolo</span> </p> ");
    }
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


