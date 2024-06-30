#include <ESP8266WiFi.h>
#include <espnow.h>

/**
 *
 * User configuration starts here
 *
**/

// Max of 250 bytes as per the ESP-Now documentation, reduce this if you're having performance issues.
#define BUFFER_SIZE 250

// Probably can go higher, but do keep in mind that the maximum theorethical speed of ESP-Now itself is around 2Mbps
#define BAUD_RATE   115200

/**
 *
 * Controls how many times the packet will try to be resent before it gets dropped.
 * This will incur a performance penealty as new packets will need to be resent BEFORE
 * the ones that came after it through a "blocking mechanism", delaying the communication chain. 
 * Change this value with care.
 * 
 * Commenting this line will disable resending and blocking of packets which should DRASTICALLY improve performance/latency.
 * This however, means your serial communication is significantly more likely to drop and lose information, resulting in
 * potentially undefined behaviour.
 *
**/
#define MAX_CONNECTION_ATTEMPTS 5

// Enable debug printing, this is in order to debug what messages are coming in and out.
// WARNING: For testing only!!
//#define DEBUG

// Quickly change which boards MAC address is used.
//#define BOARD1

#ifdef BOARD1
  // MAC address of your other board 
  uint8_t broadcastAddress[] = {0xA4, 0xE5, 0x7C, 0x1E, 0x89, 0xBC}; 
#else
  // BOARD1s MAC address
  uint8_t broadcastAddress[] = {0xA4, 0xE5, 0x7C, 0x1E, 0xA4, 0xEF}; 
#endif


/**
 *
 * User configuration stops here
 *
**/

const uint32_t timeout_micros = (int)(1.0 / BAUD_RATE * 1E6) * 20;

uint8_t buf_recv[BUFFER_SIZE];
uint8_t buf_send[BUFFER_SIZE];
uint8_t buf_size = 0;
uint32_t send_timeout = 0;
volatile uint8_t connection_failures = 0;
volatile bool    block_send = 0;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (sendStatus == 0){
    #ifdef DEBUG   
      Serial.print("Delivery success, attempt #");
      Serial.println(connection_failures + 1);
    #endif

    connection_failures = 0;
    buf_size = 0;
  } else {
    #ifdef DEBUG
      Serial.print("Delivery fail, attempt #");
      Serial.println(connection_failures + 1);
    #endif

    connection_failures++;
  }

  block_send = false;
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  digitalWrite(LED_BUILTIN, LOW);
  
  #ifdef DEBUG
  Serial.print("Bytes received: ");
  Serial.println(len);
  #endif

  ESP.wdtFeed();
  
  memcpy(&buf_recv, incomingData, sizeof(buf_recv));
  Serial.write(buf_recv, len);

  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  // init LED output, inverted logic for some reason
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Init Serial Monitor
  Serial.begin(BAUD_RATE);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  // Unblock reading input
  connection_failures = 0;
}

void send_message() {
  #ifdef MAX_CONNECTION_ATTEMPTS 
  if(connection_failures < MAX_CONNECTION_ATTEMPTS && !block_send) {
    block_send = true;
    esp_now_send(broadcastAddress, (uint8_t *) &buf_send, buf_size);
  } else if(connection_failures == MAX_CONNECTION_ATTEMPTS) {
    #ifdef DEBUG
      Serial.println("Dropping packet as it has exceeded the maximum number of failures");
    #endif
    connection_failures = 0;
    buf_size = 0;
  }
  #else
    esp_now_send(broadcastAddress, (uint8_t *) &buf_send, buf_size);
    buf_size = 0;
  #endif
}
 
void loop() {
  // read up to BUFFER_SIZE from serial port
  if (Serial.available() 
#ifdef MAX_CONNECTION_ATTEMPTS
  // block modifying the input buffer UNTIL last buffer had been sent or dropped
  && (connection_failures == 0)
#endif
  ) {
    while (Serial.available() && buf_size < BUFFER_SIZE) {
      buf_send[buf_size] = Serial.read();
      send_timeout = micros() + timeout_micros;
      buf_size++;
    }
  }

  // send buffer contents when full or timeout has elapsed
  if (buf_size == BUFFER_SIZE || (buf_size > 0 && micros() >= send_timeout)) {
    send_message();
  }
}
