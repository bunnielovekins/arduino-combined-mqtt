#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Servo.h>

byte mac[] = { 0x8E, 0x8D, 0xBE, 0x8F, 0xFE, 0xEE };
//char server[] = "ec2-54-213-70-96.us-west-2.compute.amazonaws.com";
char server[] = "10.32.25.133";
int myId = -1;
char myName[] = "PetersDesk";
int sensorPin = A0;
int sensorValue = 0;
int lastValue = 0;
char messageBuffer[10];
char meta[] = "sens/meta";
char myTopic[] = "sens/ ";
char tempTopic[] = "sens/0";
char myClientNum[] = "Ardy2";

EthernetClient client;
PubSubClient mqclient(server, 1883, callback, client);

//Servos
Servo myservos[8];
int servoConnected[] = {false,false,true,true,false,false,false};//First two must remain false, they're used for serial stuff
int inValue = -1;
int outValue = -1;

void setup() {
  Serial.begin(9600);
  Serial.println(".");
  if(Ethernet.begin(mac)==0){
    Serial.println("Failed to configure Ethernet with DHCP");
    
  }
  else 
    Serial.println("Phew");
  delay(1000);
  
  while(!mqclient.connect(myClientNum)){
    Serial.println("Connection failed, trying again in 1s");
    delay(1000);
  }
  
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  int i = 0;
  for(i;i<7;i++){
    if(servoConnected[i]){
      myservos[i].attach(i);
      tempTopic[5] = i+46;
      Serial.print("Watching: ");
      Serial.println(tempTopic);
      mqclient.subscribe(tempTopic);
    }
  }
  
  mqclient.subscribe(meta);
  mqclient.publish(meta,"add PetersDesk2");

  delay(500);
  mqclient.loop();


}



void loop()
{
    mqclient.loop();
    //Serial.print(',');
    if(myId!=-1){
      lastValue = sensorValue;
      sensorValue = analogRead(sensorPin);
      int diff = lastValue-sensorValue;
      if(diff>2 || diff<-2){
        if(!mqclient.connected()){
          while(!mqclient.connect(myClientNum)){
            Serial.println("Connection failed, trying again in 1s");
            delay(1000);
          }
        }
        intToStr(sensorValue,messageBuffer);
        mqclient.publish(myTopic,messageBuffer);
        Serial.println(messageBuffer);
      }
    }
  delay(50);
}

void callback(char* topic, byte* payload, unsigned int length) {
  if(myId==-1 && topic[5] == 'm'){
    //Serial.print("Meta reply: ");
    //Serial.println((char*)payload);
    if(payload[0]>='0' && payload[0]<='9'){
      myTopic[5] = payload[0];
      myId=(int)(payload[0]-48);
      digitalWrite(13,HIGH);
      //myClientNum[5] = payload[0];
      Serial.print("Id:");
      Serial.println(myId);
    }

    //Serial.print("Topic:");
    //Serial.println(myTopic);
  }
  else if(myId==-1){
    
    
  }
  else  {
    //Serial.println((char*)payload);
    if(topic[5]>='0' && topic[5]<='9' && payload[0]>='0' && payload[0]<='9'){
      int number = topic[5] - 48;
      number+=2;
      //Serial.print("Sensor Number:");
      //Serial.println(number);
      int i = 0;
      int result = 0;
      while(i<length){
        result*=10;
        result+=payload[i]-48;
        i++;
      }
      //Serial.print("Value:");
      //Serial.println(result);
      if(result>=0 && result<=1024 && servoConnected[number]){
        result = map(result,0,1023,0,179);
        myservos[number].write(result);
      }
    }
  }
}

char *intToStr(int num, char *buffer){
  String str = "";
  str+=num;
  str.toCharArray(buffer,10);
  return buffer;
}
