int leds[] = {19, 18, 5, 17, 16, 4, 0, 2, 15, 32, 33, 25, 26, 27, 12, 13};
int hall_sensor = 14;
int num_leds = 16;
int spin_count = 0;
int end_time = 0;
int start_time = 0;
float rotation_time = 0;
float degree = 0;

long unsigned int forty = 40;
long unsigned int thousand = 40;

volatile unsigned long lastMicros = 0;
volatile unsigned long intervalMicros = 0;
volatile bool newIntervalAvailable = false;

void IRAM_ATTR signalISR() {
  unsigned long now = micros();
  intervalMicros = now - lastMicros;
  lastMicros = now;
  newIntervalAvailable = true;
}

void processInterval(void *parameter) {
  while (true) {
    if (newIntervalAvailable) {
      // Copy volatile variable safely
      unsigned long interval;
      noInterrupts();
      interval = intervalMicros;
      newIntervalAvailable = false;
      interrupts();

      // Do something with the interval (e.g. print or calculate)
//      Serial.print("Interval (us): ");
//      Serial.println(interval);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Avoid hogging the CPU
  }
}

void setup() {
  // put your setup code here, to run once:
//  Serial.begin(115200);
  for (int i = 0; i < num_leds; i++){
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }
  pinMode(hall_sensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_sensor), signalISR, RISING);

  // Create a separate task for interval processing
  xTaskCreatePinnedToCore(
    processInterval,   // Task function
    "ProcessInterval", // Task name
    4096,              // Stack size
    NULL,              // Parameter
    1,                 // Priority
    NULL,              // Task handle
    1                  // Core (0 or 1)
  );

  lastMicros = micros();
}

void loop() {
   degree = min((float)thousand, max((float)forty, ((float)intervalMicros)/1000)/360);
//   Serial.println(" ");
//   Serial.println(intervalMicros);
//   Serial.println(intervalMicros/360);
//   Serial.println((float)intervalMicros/360);
//   Serial.println(degree);
   // Semi-Circle Code
   if (digitalRead(hall_sensor) == LOW && spin_count <= 49) {
//     Serial.println("Magnet Detected");
     spin_count = (spin_count + 1) % 200;
     for (int i = 0; i < num_leds/2; i++) {
       digitalWrite(leds[i], HIGH);
       digitalWrite(leds[num_leds-1-i], LOW);
     }
     delay(180*degree);
     for (int i = 0; i < num_leds/2; i++) {
       digitalWrite(leds[i], LOW);
       digitalWrite(leds[num_leds-1-i], HIGH);
     }
     //delay(180*degree);
   }

   // Cross Code
   else if (digitalRead(hall_sensor) == LOW && spin_count >= 50 && spin_count <= 149) {
//     Serial.println("Magnet Detected");
     spin_count = (spin_count + 1) % 200;
     for (int i = 0; i < num_leds; i++) {
       digitalWrite(leds[i], HIGH);
     }
     for (int i = 0; i < 2; i++) {
       delay(3*degree);
       for (int i = 0; i < num_leds; i++) {
         digitalWrite(leds[i], LOW);
       }
       delay(90*degree - 3*degree);
       for (int i = 0; i < num_leds; i++) {
         digitalWrite(leds[i], HIGH);
       }
       delay(3*degree);
       for (int i = 0; i < num_leds; i++) {
         digitalWrite(leds[i], LOW);
       }
       //delay(90*degree - degree);
     }
   }

  else if (digitalRead(hall_sensor) == LOW && spin_count >= 150) {
//    Serial.println("Magnet Detected");
    spin_count = (spin_count + 1) % 200;
    for (int i = 0; i < num_leds; i++) {
      digitalWrite(leds[i], HIGH);
    }
    for (int i = 0; i < 4; i++) {
      digitalWrite(leds[0], LOW);
      digitalWrite(leds[15], LOW);
      delay(degree*11.25);
      digitalWrite(leds[1], LOW);
      digitalWrite(leds[14], LOW);
      delay(degree*11.25);
      digitalWrite(leds[2], LOW);
      digitalWrite(leds[13], LOW);
      delay(degree*11.25);
      digitalWrite(leds[3], LOW);
      digitalWrite(leds[12], LOW);
      delay(degree*11.25);
      digitalWrite(leds[3], HIGH);
      digitalWrite(leds[12], HIGH);
      delay(degree*11.25);
      digitalWrite(leds[2], HIGH);
      digitalWrite(leds[13], HIGH);
      delay(degree*11.25);
      digitalWrite(leds[1], HIGH);
      digitalWrite(leds[14], HIGH);
      delay(degree*11.25);
      digitalWrite(leds[0], HIGH);
      digitalWrite(leds[15], HIGH);
      //delay(degree*11.25);
    }
  }
}
