#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
}; //MAC address found on the back of your ethernet shield.
byte ip[] = { 192, 168, 137, 20 };
byte serverName[] = { 192, 168, 137, 1 };
EthernetClient client;
uint8_t id=1;
String st="";

// pin #2 is IN from sensor (YELLOW wire)
// pin #3 is OUT from arduino  (BLUE wire)
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()  
{  
  Serial.begin(9600);
  Serial.println("Adafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) 
  {
    Serial.println("Found fingerprint sensor!");
  } 
  else 
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }
  Ethernet.begin(mac, ip);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop()                     // run over and over again
{  
 while (! getFingerprintEnroll() );
}

uint8_t getFingerprintEnroll() 
{

  int p = -1;
  Serial.println("Waiting for valid finger..................."); 
  while (p != FINGERPRINT_OK) 
  {
    p = finger.getImage();
    switch (p) 
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      break;
    default:
      Serial.println("Error while taking image");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) 
  {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    default:
      Serial.println("Error while making conversion");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(1000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) 
  {
    p = finger.getImage();
  }
  
  p = -1;
  Serial.println("Place same finger again..................");
  while (p != FINGERPRINT_OK) 
  {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      break;
    default:
      Serial.println("Error while taking image");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    default:
      Serial.println("Error while making conversion");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model");  
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Error while matching");
    return p;
  }   
  
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) 
  {
    Serial.println("Success!");
    uploadFingerpintTemplate(id);    
    return;
  }
  else 
  {
    Serial.println("Error while storing image");
    return p;
  }   
}

uint8_t uploadFingerpintTemplate(uint16_t id)
{
 uint8_t p = finger.loadModel(id);
  
  if(p!=FINGERPRINT_OK)
  {
      Serial.print("Error while retrieving image"); 
      return p;
  }

  // OK success!

  p = finger.getModel();
  if(p!=FINGERPRINT_OK)
  {
      Serial.print("Error while modeling "); 
      return p;
  }
  
  uint8_t templateBuffer[256];
  memset(templateBuffer, 0xff, 256);  //zero out template buffer
  int index=0;
  uint32_t starttime = millis();
  st="";
  while (index < 256)
  {
    if (mySerial.available())
    {
      templateBuffer[index] = mySerial.read();
      st+=templateBuffer[index];
      index++;
    }
  }
  Serial.print("\nYour ID:");
  Serial.println(st);
  Serial.println();
  ether();
}

void ether()
{

  if (client.connect(serverName, 80))
  {
    Serial.println("connected...");
    Serial.println("ARDUINO: forming HTTP request message");
    client.print("GET /smartcampus1/admin/staff_login.php?faculty_id=");
    client.println(st);
    client.println();
    client.println("HTTP/1.1");
    client.println("Host: localhost");
    client.println();

    Serial.println("ARDUINO: HTTP message sent");
    delay(1000);

    client.stop();
  }
  else
  {
    Serial.println("connection failure");
  }
}
