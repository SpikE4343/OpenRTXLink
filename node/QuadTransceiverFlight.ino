#include <SoftwareSerial.h>


/** RF24Mesh_Example.ino by TMRh20
 *
 * This example sketch shows how to manually configure a node via RF24Mesh, and send data to the
 * master node.
 * The nodes will refresh their network address as soon as a single write fails. This allows the
 * nodes to change position in relation to each other and the master node.
 */


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
#include <printf.h>


/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(7, 8);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
 * User Configuration: nodeID - A unique identifier for each radio. Allows addressing
 * to change dynamically with physical changes to the mesh.
 *
 * In this example, configuration takes place below, prior to uploading the sketch to the device
 * A unique value from 1-255 must be configured for each node.
 * This will be stored in EEPROM on AVR devices, so remains persistent between further uploads, loss of power, etc.
 *
 **/
#define nodeID 1


uint32_t displayTimer = 0;

struct payload_t {
  int data;
  unsigned long counter;
};

void setup() {

  Serial.begin(115200);
  
  printf_begin();

  while(!Serial);
  
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  
  Serial.println(mesh.getNodeID());
  
  // Connect to the mesh
  Serial.println("mesh.begin() - running");
  mesh.begin();
  Serial.println("mesh.begin() - complete");
}

void displayStatus()
{
  //Serial.println("displayStatus");
  uint32_t now = millis();
  if( displayTimer > now )
  {
    return;
  }

    
  displayTimer = now + 5000;

  Serial.println("===========================" );
  Serial.print("n:");
  Serial.print(mesh.getNodeID());
  Serial.print(", t:");
  Serial.println(now);
  
  radio.printDetails();
}



void networkRecv()
{
  //Serial.println("networkRecv");
  // Check for incoming data from the sensors
  if(!network.available())
    return;
    
  RF24NetworkHeader header;
  network.peek(header);
  
  payload_t payload;

  // handle messages
  switch(header.type){
    case 'M': 
      network.read(header,&payload,sizeof(payload)); 
      Serial.write(payload.data);
      //++stats.bytesRead;
      break;
      
    default: 
      network.read(header,NULL,0); 
      break;
  }
}

void networkSend()
{
  //Serial.println("networkSend");
  // TODO: buffer read/write
  if( !Serial.available() )
    return;
  
  RF24NetworkHeader header(mesh.addrList[0].address, OCT);
  payload_t payload;
  payload.data = Serial.read();
  
  if( !network.write(header, &payload, sizeof(payload)) )
  {
    Serial.println("Failed to send to nodes" );
    return;  
  }
  
  //++stats.bytesWritten;
}

void networkUpdate()
{
  // Call mesh.update to keep the network updated
  mesh.update();
  
  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
 
  networkRecv();
  networkSend();
}

void loop() {
  networkUpdate();
  displayStatus();
}






