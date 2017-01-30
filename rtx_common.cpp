 
 
 /** RF24Mesh_Example_Master.ino by TMRh20
  * 
  *
  * This example sketch shows how to manually configure a node via RF24Mesh as a master node, which
  * will receive all data from sensor nodes.
  *
  * The nodes can change physical or logical position in the network, and reconnect through different
  * routing nodes as required. The master node manages the address assignments for the individual nodes
  * in a manner similar to DHCP.
  *
  */
  
  
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
//Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>

#include <printf.h>

/***** Configure the chosen CE,CS pins *****/
RF24 radio(7,8);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

uint32_t displayTimer = 0;

struct stats_t {
  int bytesRead;
  int bytesWritten;
  uint32_t lastEvent;
};



struct payload_t 
{
  int data;
  unsigned long counter;
};

#define MAX_NODES 8;
stats_t stats[MAX_NODES-1];

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
  Serial.println("-----------");
  printf("addr:%u\n", mesh.addrListTop );
  
  for( int i=0; i < mesh.addrListTop; ++i)
  {
    auto addr = mesh.addrList[i];
    auto stat = stats[i];
    
    Serial.print(addr.nodeID);
    Serial.print(":");
    Serial.print(addr.address);
    Serial.print(",rx:");
    Serial.print(stat.bytesRead);
    Serial.print(",tx:");
    Serial.print(stat.bytesWritten);
    Serial.print(",t:");
    Serial.println(now - stat.lastEvent);
  }
}

void setup() 
{
  displayTimer = millis() + 5000;
  
  Serial.begin(9600);
  printf_begin();
  while(!Serial);
  
  // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  Serial.println(mesh.getNodeID());
  
  // Connect to the mesh
  Serial.println("mesh.begin() - running");
  mesh.begin();
  Serial.println("mesh.begin() - complete");
}

void networkUpdate()
{
  // Call mesh.update to keep the network updated
  mesh.update();
  
  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
  
  networkRecv();
  networkSend();
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
      ++stats.bytesRead;
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
  
  ++stats.bytesWritten;
}

void loop() {    
  //Serial.println("loop");
  networkUpdate();  

  displayStatus();
}
