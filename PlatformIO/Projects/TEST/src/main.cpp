#include <Arduino.h> 
#include <StarterKitNB.h> 
#include <SparkFun_SHTC3.h> 
#include <Wire.h>   

//Definiciones 

//Definicion sensor movimiento
#define SENSOR_PIN WB_IO6 
StarterKitNB sk; 
//Definicion sensor temperatura y humedad
SHTC3 mySHTC3; 

//Definiciones bateria
#define PIN_VBAT WB_A0
#define VBAT_MV_PER_LSB (0.73242188F) // 3.0V ADC range and 12 - bit ADC resolution = 3000mV / 4096
#define VBAT_DIVIDER_COMP (1.73)      // Compensation factor for the VBAT divider, depend on the board
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

//Mensaje 
String Temperature; 
String Humidity; 
String Movimiento;


//APN 
String apn = "m2m.entel.cl"; 
String user = "entelpcs"; 
String pw = "entelpcs"; 

//ThingsBoard 
String clientId = "grupo2"; 
String userName = "22222"; 
String password = "22222"; 

int gCurrentStatus = 0;         // variable for reading the pin current status 
int gLastStatus = 0;            // variable for reading the pin last status 

void errorDecoder(SHTC3_Status_TypeDef message)                            
{   
  switch(message)   
  {    
    case SHTC3_Status_Nominal : Serial.print("Nominal"); break;     
    case SHTC3_Status_Error : Serial.print("Error"); break;     
    case SHTC3_Status_CRC_Fail : Serial.print("CRC Fail"); break;     
    default : Serial.print("Unknown return code"); break;   
  }   
} 
void shtc3_get(void) 
{   
  float Temperature,Humidity;   
  mySHTC3.update();   
  if(mySHTC3.lastStatus == SHTC3_Status_Nominal)                 
  {     
    /* Packing LoRa data */     
    Temperature = mySHTC3.toDegC();     
    Humidity = mySHTC3.toPercent(); 


    Serial.print("RH = ");     
    Serial.print(mySHTC3.toPercent());                       
    Serial.print("% (checksum: ");     
    if(mySHTC3.passRHcrc)                                         
    {       
      Serial.print("pass");     
    }     
    else    
    {       
      Serial.print("fail");     
    }     
    Serial.print("), T = ");     
    Serial.print(mySHTC3.toDegC());                             
    Serial.print(" deg C (checksum: ");     
    if(mySHTC3.passTcrc)     
    {      
      Serial.print("pass");     
    }     
    else     
    {       
      Serial.print("fail");     
    }     
    Serial.println(")");   
  }   
  else   
  {     
    Serial.print("Update failed, error: ");     
    errorDecoder(mySHTC3.lastStatus);     
    Serial.println();   
  } 
} 


void setup() 
{ 
  sk.Setup(true);   
  delay(500);   
  Wire.begin();   
  
  /* shtc3 init */   
  Serial.println("shtc3 init");   
  Serial.print("Beginning sensor. Result = ");             
  errorDecoder(mySHTC3.begin());                                 
  Wire.setClock(400000);                                         
  Serial.println(); 

  sk.UserAPN ( apn , user , pw);   
  delay(500);   
  sk.Connect(apn); 
  /* SENSOR_PIN */  
  pinMode(SENSOR_PIN, INPUT);   // The Water Sensor is an Input 
} 


void loop() 
{     
  if (!sk.ConnectionStatus())   
  {     
    sk.Reconnect(apn);   
  } 
  if (!sk.LastMessageStatus)   
  {     
    sk.ConnectBroker(clientId , userName , password);   
  } 
  shtc3_get();   
  delay(1000); 
  //sk.SendMessage(msg);     
  //using namespace std;   
  //Temperature =  String(mySHTC3.toDegC());   
  //Humidity =  String(mySHTC3.toPercent()); 
  //String msg2 = "{\"Temperatura\" : "+Temperature+" , \"Humedad\" : "+Humidity+" ,  \"Bateria\": "+String(round(analogRead(PIN_VBAT) * REAL_VBAT_MV_PER_LSB)/37)+"  }" ; 
 
  //sk.SendMessage(msg2);   
  gCurrentStatus = digitalRead(SENSOR_PIN);   
  if(gLastStatus != gCurrentStatus)   
  {     
    if(gCurrentStatus == 0)     
    { //0: detected   1: not detected      
      Serial.println("IR detected ..."); 
      digitalWrite(LED_GREEN,HIGH);   //turn on       
      digitalWrite(LED_BLUE,HIGH);     
    } 
      else     
      {       
       digitalWrite(LED_GREEN,LOW);       
       digitalWrite(LED_BLUE,LOW);   // turn LED OF 
      }     
    gLastStatus = gCurrentStatus;   
  }   
    else 
    {     
      delay(100);   
    } 


  using namespace std;   
  Temperature =  String(mySHTC3.toDegC());   
  Humidity =  String(mySHTC3.toPercent()); 
  Movimiento = String(digitalRead(SENSOR_PIN));
  String msg2 = "{\"Temperatura\" : "+Temperature+" , \"Humedad\" : "+Humidity+" ,  \"Bateria\": "+String(round(analogRead(PIN_VBAT) * REAL_VBAT_MV_PER_LSB)/37)+" , \"Movimiento\" : "+Movimiento+" }" ; 
 
  sk.SendMessage(msg2);  


}
