int leds[] = {19, 18, 5, 17, 16, 4, 0, 2, 15, 32, 33, 25, 26, 27, 12, 13};
int hall_sensor = 14;
int num_leds = 16;
int spin_count = 0;
int end_time = 0;
int start_time = 0;
int rotation_time = 0;
float degree = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (int i = 0; i < num_leds; i++){
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }
}

void loop() {
   // Semi-Circle Code
   if (digitalRead(hall_sensor) == LOW && spin_count <= 49) {
     Serial.println("Magnet Detected");
     spin_count = (spin_count + 1) % 200;
     for (int i = 0; i < num_leds/2; i++) {
       digitalWrite(leds[i], HIGH);
       digitalWrite(leds[num_leds-1-i], LOW);
     }
     delay(50);
     for (int i = 0; i < num_leds/2; i++) {
       digitalWrite(leds[i], LOW);
       digitalWrite(leds[num_leds-1-i], HIGH);
     }
     delay(50);
   }

   // Cross Code
   else if (digitalRead(hall_sensor) == LOW && spin_count >= 50 && spin_count <= 149) {
     Serial.println("Magnet Detected");
     spin_count = (spin_count + 1) % 200;
     for (int i = 0; i < num_leds; i++) {
       digitalWrite(leds[i], HIGH);
     }
     delay(1);
     for (int i = 0; i < num_leds; i++) {
       digitalWrite(leds[i], LOW);
     }
     delay(25 - 1);
     for (int i = 0; i < num_leds; i++) {
       digitalWrite(leds[i], HIGH);
     }
     delay(1);
     for (int i = 0; i < num_leds; i++) {
       digitalWrite(leds[i], LOW);
     }
     delay(25 - 1);
   }

  else if (digitalRead(hall_sensor) == LOW && spin_count >= 150) {
    Serial.println("Magnet Detected");
    spin_count = (spin_count + 1) % 200;
    for (int i = 0; i < num_leds; i++) {
      digitalWrite(leds[i], HIGH);
    }
    for (int i = 0; i < 4; i++) {
      digitalWrite(leds[0], LOW);
      digitalWrite(leds[15], LOW);
      delay(3.125);
      digitalWrite(leds[1], LOW);
      digitalWrite(leds[14], LOW);
      delay(3.125);
      digitalWrite(leds[2], LOW);
      digitalWrite(leds[13], LOW);
      delay(3.125);
      digitalWrite(leds[3], LOW);
      digitalWrite(leds[12], LOW);
      delay(3.125);
      digitalWrite(leds[3], HIGH);
      digitalWrite(leds[12], HIGH);
      delay(3.125);
      digitalWrite(leds[2], HIGH);
      digitalWrite(leds[13], HIGH);
      delay(3.125);
      digitalWrite(leds[1], HIGH);
      digitalWrite(leds[14], HIGH);
      delay(3.125);
      digitalWrite(leds[0], HIGH);
      digitalWrite(leds[15], HIGH);
      delay(3.125);
    }
  }
}
