#include <WiFiLink.h>
#include <WiFiUdp.h>

int status = WL_IDLE_STATUS;
char ssid[] = "EEERover";                       // Network details
char pass[] = "exhibition"; 

IPAddress ip(192, 168, 0, 12);                  // Arduino's IP
IPAddress remoteIp(192, 168,0, 229);            // iPhone's IP
IPAddress testIp(172, 20, 10, 2);

unsigned int recvPort = 2390;                   // Arduino's receiving port
unsigned int sendPort = 2525;                   // iPhone's receiving port

#define packetLen 16                            // Receiving packet length   

char packetbuffer[packetLen];                   // Receiving packet buffer

int sideways;                                   // Movement values
int forwardBackward;

WiFiUDP udpR;                                   // UDP
WiFiUDP udpT;

int leftDir = 3;                                // Motor control direction pins
int rightDir = 2;

int leftPWM = 9;                                // Motor control speed pins
int rightPWM = 10;

int sensorVals[8];                              // Sensor value array

int ultraPin = 7;
int radioPin = 6;;
int magneticPin1 = 12;
int magneticPin2 = 11;

unsigned long startMillis;                      // New data millis() timer
unsigned long currentMillis;



void setup() {
  Serial.begin(115200);                         // Serial out
  pinMode(13, OUTPUT);                          // Pin 13 acts as a status LED for WiFi
  pinMode(leftDir, OUTPUT);                     // Set Pin modes
  pinMode(rightDir, OUTPUT);
  pinMode(leftPWM, OUTPUT);
  pinMode(rightPWM, OUTPUT);
  
  pinMode(A5, INPUT);
  pinMode(ultraPin, INPUT);
  pinMode(radioPin, INPUT);
  pinMode(magneticPin1, INPUT);
  pinMode(magneticPin2, INPUT);
  
  if(WiFi.status() == WL_NO_WIFI_MODULE_COMM){  // Check for WiFi module
    Serial.println("No WiFi Module");
    while(true);
  }

  while(status != WL_CONNECTED){                // Connect to EEERover
    Serial.println("Connecting to EEERover");
    WiFi.config(ip);                           //IP address to change
    status = WiFi.begin(ssid, pass);
    delay(3000);
  }
  Serial.println("Connected");                 // Connection established
  digitalWrite(13, HIGH);
  udpR.begin(recvPort);                       // Open UDP streams
  udpT.begin(sendPort);
  Serial.println(WiFi.localIP());             
  startMillis = millis();                      // Millis timer setup
}

void loop() {
  int packetSize = udpR.parsePacket();         // Check for UDP packet
  if(packetSize){
    int len = udpR.read(packetbuffer, packetLen); // Read UDP packet buffer
    String upDown;
    String leftRight;
    bool colon;
    for(int i = 0; i < len; i++){             // Split packet buffer to correct string depending on where the colon is
      if(packetbuffer[i] == ':'){
        colon = true;
          colon = i;
      }
      else{
        if(colon){
          upDown += packetbuffer[i];         // Create a string for the direction values
        }
        else{
          leftRight += packetbuffer[i]; 
        }
      }
    }
    forwardBackward = upDown.toInt();       // Change string variable to integer
    sideways = leftRight.toInt();
//    Serial.print(forwardBackward);        // Debugging output
//    Serial.print("\t");
//    Serial.print(sideways);
//    Serial.print("\n");
    udpR.endPacket();                       // End UDP packet
    driveMotors(forwardBackward, sideways); // Send data to motors                   
  }                                         


  if(newData()){                            // If data is available
    udpT.beginPacket(remoteIp, sendPort);   // Start a packet to send data to iPhone's receiving port
    udpT.print(sensorVals[0]);              // Send sensor data split by colons
    udpT.print(":");
    udpT.print(sensorVals[1]);
    udpT.print(":");
    udpT.print(sensorVals[2]);
    udpT.print(":");
    udpT.print(sensorVals[3]);
    udpT.endPacket();                       // End UDP packet
  }
}

void driveMotors(int FB, int side){
  int pwmL;                                 // Integers for motor PWM values
  int pwmR;
  if(FB < 0){                               // If FB is negative, rover needs to move backwards, set direction pins as necessary
    digitalWrite(leftDir, LOW);
    digitalWrite(rightDir, LOW);
    FB = -FB;                               // Make FB positive again for the PWM maths
  }
  else{
    digitalWrite(leftDir, HIGH);
    digitalWrite(rightDir, HIGH);
  }
  pwmL = FB - side;                         // Set Right and left PWM values
  pwmR = FB + side;
  if(pwmL < 0){                             // Check values to make sure they aren't negative to prevent errors, set any negative values to 0
    pwmL = -pwmL;
    digitalWrite(leftDir, HIGH);
    digitalWrite(rightDir, LOW);
  }
  if(pwmR < 0){
    pwmR = -pwmR;
    digitalWrite(leftDir, LOW);
    digitalWrite(rightDir, HIGH);
  }
  pwmL = map(pwmL, 0, 70, 0, 380);          // Scale the output
  pwmR = map(pwmR, 0, 70, 0, 380);
  if(pwmL > 255){                           // Check values to make sure they aren't above 255 to prevent errors, set any larger values to 255
    pwmL = 255;
  }
  if(pwmR > 255){
    pwmR = 255;
  }
  analogWrite(rightPWM, pwmL);             // Output PWM values
  analogWrite(leftPWM, pwmR);
}

bool newData(){
  if(checkTime(1000)){                    // Check for new data every 1 second
    sensorVals[0] = checkFreqMod(radioPin);  
    sensorVals[1] = checkIR(A5);
    sensorVals[2] = checkUltrasound(ultraPin);;
    sensorVals[3] = checkMagnet(magneticPin1, magneticPin2);
//    Serial.println(sensorVals[0]);
    return true;                         // New data is available so return true otherwise return false
  }
  else{
    return false;
  }
}

bool checkTime(unsigned long wait){     // Non-blocking way to check if enough time has passed before reading data
  currentMillis = millis();
  if(currentMillis - startMillis >= wait){  // If time elapsed since start subtracted from the most recent read time is greater than the read time, return true
    startMillis = millis();
    return true;
  }
  return false;
}

bool checkUltrasound(int ultrasoundPin){
  int x = 0;
  int count;
  while(x < 1000){
    if(digitalRead(ultrasoundPin)){
      count++;
    }
    delayMicroseconds(10);
    x++;
  }
  if((count > 300)){
    return true;
  }else{
    return false;
  }
}

int checkFreqMod(int radioPin){
  int count = pulseIn(radioPin, LOW, 50000);
  if((count > 1500) && (count < 2500)){
    return 239;
  }else if((count > 2600) && (count < 3500)){
    return 151;
  }
  return 0;
}

bool checkIR(int IRPin){
  int val = analogRead(A0);
  if(val >= 95){
    return true;
  }
  return false;
}

bool checkMagnet(int magneticPin1, int magneticPin2){
  if(digitalRead(magneticPin1) || digitalRead(magneticPin2)){
    return true;
  }
  return false;
}

