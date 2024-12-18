#include "WifiService.h"
#include "MySocketService.h"
#include "helper.h"

// Create a SocketService instance
SocketService socketService;
WiFiService wiFiService;
Helper helper;

bool internet_status = false;
int battery_percentage = 0;

// Push data to server timing
unsigned long device_update_time = millis();
const unsigned long device_update_duration = 1000 * 45;

bool sensor_status = false;
bool sensor_previous_value = false;



// Sensor status detection related
unsigned long sensor_low_time = 0;
const unsigned long sensor_low_duration = 1000 * 60;

// train detection variables
unsigned long detection_sensor_low_time = 0;
const unsigned long detection_sensor_low_duration = 1000 * 10;
unsigned int wheel_count = 0;

// train detection led related variables
unsigned long train_detection_led_on_time = 0;
const unsigned long train_detection_led_duration = 1000 * 30;

// train detection cooldown related
unsigned long traind_detected_at = 0;
const unsigned long train_detection_cooldown_duration = 1000 * 60 * 3;
bool socket_status = false;

SemaphoreHandle_t mutex;


void socketIOTask(void *pvParameters)
{

  while (true)
  {

    if (xSemaphoreTake(mutex, portMAX_DELAY))
    {
      // Run the Socket.IO loop
      socketService.socketIO.loop();
      socket_status = socketService.isConnected(); 
      xSemaphoreGive(mutex);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Yield to other tasks
  }
}

void setup()
{
  // Pin definitions
  pinMode(BATTERY, INPUT);
  pinMode(MODEM_RELAY, OUTPUT);
  pinMode(INTERNET_LED, OUTPUT);
  pinMode(SENSOR_LED, OUTPUT);   // Added for SENSOR_LED
  pinMode(SENSOR, INPUT_PULLUP); // Pin for sensor input
  pinMode(TRAIN_DETECTION_LED, OUTPUT);
  pinMode(TEST_BUTTON, INPUT_PULLUP);

  // Initial pin states
  digitalWrite(MODEM_RELAY, HIGH);
  digitalWrite(INTERNET_LED, LOW);
  digitalWrite(SENSOR_LED, LOW); // Turn off SENSOR_LED initially
  digitalWrite(TRAIN_DETECTION_LED, LOW);

  Serial.begin(115200);

  wiFiService.begin();
  socketService.connect(HOST_ID, HOST_PORT);

  mutex = xSemaphoreCreateMutex();

  // Create the Socket.IO task on Core 1
  xTaskCreatePinnedToCore(socketIOTask, "SocketIOTask", 8192, NULL, 1, NULL, 1);
}

// DONE :  test button 
void testButton(bool test_button_value,unsigned long current_time){
 
  if (test_button_value == LOW && train_detection_led_on_time == 0)
  {
    digitalWrite(TRAIN_DETECTION_LED, HIGH);
    socketService.send_log(battery_percentage, sensor_status, true);
    train_detection_led_on_time = current_time;
    Serial.println("Test Button is pressed");
  }
}

void loop()
{
  unsigned long current_time = millis();
  internet_status = wiFiService.isConnected();
  
  if (!internet_status)
  {
    wiFiService.begin();
  }

  // Check if Wi-Fi is connected
  digitalWrite(INTERNET_LED, socket_status);
  battery_percentage = helper.batteryPercentage();

  // Sensor status code
  int sensor_value = digitalRead(SENSOR);
  int test_button_value = digitalRead(TEST_BUTTON);

  // DONE : Test system
  testButton(test_button_value,current_time);

  // Check the sensor value
  if (sensor_value == HIGH)
  {
    // Turn on SENSOR_LED if sensor value is high
    digitalWrite(SENSOR_LED, HIGH);
    sensor_low_time = 0;  // Reset sensor low time since it's high
    sensor_status = true; // Update LED state
  }
  else if (sensor_value == LOW)
  {
    // If sensor is low, start or continue the low timer
    if (sensor_low_time == 0)
    {
      sensor_low_time = current_time; // Start timing when sensor goes low
    }

    // If sensor has been low for more than 1 minute, turn off SENSOR_LED
    if (current_time - sensor_low_time > sensor_low_duration)
    {
      digitalWrite(SENSOR_LED, LOW);
      sensor_status = false; // Update LED state
    }
  }

  if (sensor_value != sensor_previous_value)
  {

    if (detection_sensor_low_time == 0)
    {
      detection_sensor_low_time = current_time;
    }

    sensor_previous_value = sensor_value;

    Serial.println(wheel_count);
    if (current_time - detection_sensor_low_time < detection_sensor_low_duration && detection_sensor_low_time != 0 && traind_detected_at == 0)
    {

      wheel_count++;

      if (wheel_count >= 8)
      {
        socketService.send_log(battery_percentage, sensor_status, true);
        digitalWrite(TRAIN_DETECTION_LED, HIGH);
        wheel_count = 0;
        detection_sensor_low_time = 0;
        traind_detected_at = current_time;
        train_detection_led_on_time = current_time;
      }
    }
  }

  // resest detection sensor low time
  if (current_time - detection_sensor_low_time > detection_sensor_low_duration && detection_sensor_low_time != 0)
  {
    wheel_count = 0;
    detection_sensor_low_time = 0;
    Serial.println("Train detectecion time reseted");
  }

  // turn off the led
  if (current_time - train_detection_led_on_time > train_detection_led_duration && train_detection_led_on_time != 0)
  {
    digitalWrite(TRAIN_DETECTION_LED, LOW);
    train_detection_led_on_time = 0;

    Serial.println("Turning off train detection led");
  }

  // reset train detection cooldown time

  if (current_time - traind_detected_at > train_detection_cooldown_duration && traind_detected_at != 0)
  {
    traind_detected_at = 0;
    Serial.println("Now! Your can detect train");
  }

  delay(2);
}
