#define DEBUG 0    // SET TO 0 OUT TO REMOVE TRACES

#if DEBUG
#define D_SerialBegin(...) Serial.begin(__VA_ARGS__);
#define D_print(...)       Serial.print(__VA_ARGS__)
#define D_write(...)       Serial.write(__VA_ARGS__)
#define D_println(...)     Serial.println(__VA_ARGS__)
#else
#define D_SerialBegin(bauds)
#define D_print(...)
#define D_write(...)
#define D_println(...)
#endif

#include <I2S.h>
#include <WiFi.h>
#include <Arduino.h>
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems

#define LED_BUILT_IN 21 
WiFiClient client;
const int buff_size = 256;
int availablefrommic, bytessampled, bytessent, bytesread, packetssent, availablefromclient;
uint8_t micbuffer[buff_size];
uint8_t clientbuffer[buff_size];
//uint8_t receivebuffer[buff_size];

//bool ledStatus = false;

//Replace with your wifi network credentials
const char* ssid = "put your SSID here";
const char* password = "put your wifi password here";
// whisper server
IPAddress targethost(192,168,0,173); // change to your server IP
int targetport = 43007; // change to your server port

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  // set up serial for debugging
  D_SerialBegin(115200);

  if(DEBUG){
    // When the serial monitor is turned on, 
    // the program starts to execute
    // while(!Serial){}
  };

  // for LED blinky blinky
  pinMode(LED_BUILT_IN, OUTPUT);
  digitalWrite(LED_BUILT_IN, HIGH); //Turn off led

  // connect wifi to access point
  D_print("Attempting connect to wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    D_print(".");
  }

  D_println("");
  D_println("WiFi connected");
  D_print("SSID: ");

  D_println(WiFi.SSID());
  D_print("IP address: ");
  D_println(WiFi.localIP());

  // connect TCP client to host and TCP port
  // client.stop();

  // while (!client.connect(targethost, targetport)) {
  //     D_println("Failed to connect to Whisper server.  Retrying...");
  //     digitalWrite(LED_BUILT_IN, LOW); //Turn on
  //     delay (500); //Wait 1 sec
  //     digitalWrite(LED_BUILT_IN, HIGH); //Turn off
  //     delay (500); //Wait 1 sec
  // }

  // D_println("TCP client connected");

  D_println("Initializing microphone audio");

  // start I2S at 16 kHz with 16-bits per sample
  I2S.setAllPins(-1, 42, 41, -1, -1);

  if (!I2S.begin(PDM_MONO_MODE, 16000, 16)) {
    D_println("Failed to initialize I2S!");
    while (1); // do nothing
  }

  D_println("Initialized microphone audio");

}

void loop() {

  // Check if the client is connected
  if (!client.connected()) {
    D_println("Connecting...");

    client.stop();

    // Attempt to reconnect
    if (client.connect(targethost, targetport)) {
      D_println("Reconnected to the server.");
    } else {
      D_println("Reconnection failed.");
      // triple fast blink followed by half second off
      digitalWrite(LED_BUILT_IN, LOW); //Turn on
      delay (100); //Wait 1 sec
      digitalWrite(LED_BUILT_IN, HIGH); //Turn off
      delay (100); //Wait 1 sec
      digitalWrite(LED_BUILT_IN, LOW); //Turn on
      delay (100); //Wait 1 sec
      digitalWrite(LED_BUILT_IN, HIGH); //Turn off
      delay (100); //Wait 1 sec
      digitalWrite(LED_BUILT_IN, LOW); //Turn on
      delay (100); //Wait 1 sec
      digitalWrite(LED_BUILT_IN, HIGH); //Turn off
      delay (500); //Wait 1 sec
      return; // Exit the loop if reconnection fails
    }
  }

  digitalWrite(LED_BUILT_IN, HIGH); // Turn OFF led

  // availablefrommic = I2S.available();

  // if (availablefrommic < buff_size) {
  //   bytessampled = I2S.read(micbuffer, availablefrommic);
  // } else {
  //   bytessampled = I2S.read(micbuffer, buff_size);
  // }

  // if(I2S.available() > 0){

  //   bytessampled = I2S.read(b,1);
    
  //   if(client.connected()){

  //     client.write(b, 1);
      
  //   }
  // }

  bytessampled = 0;
  availablefrommic = I2S.available();

  if(availablefrommic > 0 ){

    if(availablefrommic < buff_size){
      bytessampled = I2S.read(micbuffer, availablefrommic);
    }else{
      bytessampled = I2S.read(micbuffer, buff_size);
    }

    if(bytessampled > 0){
      client.write(micbuffer, bytessampled);
    }
  }

  //bytessent = bytessent + bytessampled;
  //if(bytessent % 4096 == 0){
    //D_print("Sent ");
    //D_println(String(bytessent) + " bytes");
  //}
  // packetssent++;

  digitalWrite(LED_BUILT_IN, LOW); // Turn ON led

  availablefromclient = client.available();

  if (availablefromclient > 0) {

    // // Read the available data
    // char data = client.read();

    // if(data != NULL){

    //   D_print(data);

    // }

    if(availablefromclient < buff_size){
      bytesread = client.read(clientbuffer, availablefromclient);
    }else{
      bytesread = client.read(clientbuffer, buff_size);
    }

    if(bytesread > 0){
      
      //String s(clientbuffer, sizeof(clientbuffer));

      String s="";
      for(int i=0; i<bytesread; i++){
        if(clientbuffer[i] != NULL){
          s += (char)clientbuffer[i];
        }
        else{
          break;
        }
      }

      if(s != NULL){

        D_print(s);

      }
    }
  }
}
