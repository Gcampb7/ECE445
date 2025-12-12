#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <FastLED.h>
#include <SPIFFS.h>
#include <vector>

#include "webpage.h"

// WiFi credentials - CHANGE THESE!
const char* ssid = "UIUC";
const char* password = "billybob";

// SD Card pins
#define SD_CS    5
#define SD_MOSI  23
#define SD_MISO  19
#define SD_SCK   18

// FastLED Info
#define LED_PIN     15        // Data pin connected to LED strip
#define NUM_LEDS    62       // Number of LEDs in your strip
#define COLOR_ORDER GRB
#define LED_TYPE    WS2812B
#define BRIGHTNESS  125      // 0 - 255
CRGB leds[NUM_LEDS];

void handleRoot();
void handleListImages();
void handleUpload();
void handleUploadResponse();
void handleDelete();
void handleGetImage();
void handleStatus();
void handleSelectImage();
bool readImage(char imageA[], uint8_t pixelsA[]);

// Image directory on SD card
#define IMAGE_DIR "/images"

// Create web server on port 80
WebServer server(80);

int hall_sensor = 14;
uint8_t pixels[256*64*3];
char image[1000];
std::vector<String> imageFiles;
int currentImageNum = 0;

// Hall sensor timing variables
volatile unsigned long lastMicros = 0;
volatile unsigned long intervalMicros = 0;
volatile bool newIntervalAvailable = false;
volatile unsigned long currentRotationTime = 0;

// Display control
char currentImage[64];
volatile bool newImageAvailable = false;
volatile bool displayPaused = false;
portMUX_TYPE pixelMux = portMUX_INITIALIZER_UNLOCKED;

// Minimum rotation time for safety (in microseconds)
// This prevents division by zero and too-fast refresh rates
#define MIN_ROTATION_TIME 10000  // 10ms = 6000 RPM max

void IRAM_ATTR signalISR() {
  unsigned long now = micros();
  unsigned long interval = now - lastMicros;
  
  // Only update if interval is reasonable
  if (interval > MIN_ROTATION_TIME) {
    intervalMicros = interval;
    lastMicros = now;
    newIntervalAvailable = true;
  }
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
      
      // Update rotation time for display task
      taskENTER_CRITICAL(&pixelMux);
      currentRotationTime = interval;
      taskEXIT_CRITICAL(&pixelMux);
      
      Serial.print("Rotation time: ");
      Serial.print(interval / 1000.0);
      Serial.print(" ms, RPM: ");
      Serial.println(60000000.0 / interval);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void displayTask(void *parameter) {
  const int COLUMNS = 256;  // Number of columns in the image
  unsigned long localRotationTime = 0;
  unsigned long columnDelay = 1000; // Default delay in microseconds
  
  while (true) {
    if (!displayPaused) {
      // Check if new image is available
      if (newImageAvailable) {
        taskENTER_CRITICAL(&pixelMux);
        newImageAvailable = false;
        taskEXIT_CRITICAL(&pixelMux);
        Serial.println("New image loaded in display task");
      }
      
      // Get current rotation time
      taskENTER_CRITICAL(&pixelMux);
      localRotationTime = currentRotationTime;
      taskEXIT_CRITICAL(&pixelMux);
      
      // Calculate delay per column based on rotation speed
      if (localRotationTime > MIN_ROTATION_TIME) {
        // Divide rotation time by number of columns to get delay per column
        columnDelay = localRotationTime / COLUMNS;
        
        // Ensure minimum delay for LED refresh
        if (columnDelay < 100) columnDelay = 100; // Minimum 100us per column
      } else {
        // If no valid rotation time, use default slow speed
        columnDelay = 1000; // 1ms per column = ~4 seconds per rotation
      }
      
      // Display one full rotation
      for(int i = 0; i < COLUMNS; i++) {
        unsigned long startTime = micros();
        
        // Update LEDs for this column
        for(int j = 0; j < 31; j++) {
          // First strip (0-30)
          leds[j].red = pixels[(i*32*3)+(j*3)+2];
          leds[j].green = pixels[(i*32*3)+(j*3)+1];
          leds[j].blue = pixels[(i*32*3)+(j*3)+0];
          
          // Second strip (31-61) - offset by 127 columns for opposite side
          leds[61-j].red = pixels[(((i+127)%256)*32*3)+(j*3)+2];
          leds[61-j].green = pixels[(((i+127)%256)*32*3)+(j*3)+1];
          leds[61-j].blue = pixels[(((i+127)%256)*32*3)+(j*3)+0];
          
          // Black out dim pixels
          if (leds[j].red + leds[j].green + leds[j].blue < 40) {
            leds[j] = CRGB::Black;
          }
          if (leds[61-j].red + leds[61-j].green + leds[61-j].blue < 40) {
            leds[61-j] = CRGB::Black;
          }
        }
        
        FastLED.show();
        
        // Precise timing: wait for the calculated column delay
        unsigned long elapsed = micros() - startTime;
        if (elapsed < columnDelay) {
          delayMicroseconds(columnDelay - elapsed);
        }
      }
    } else {
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
}

void swapImage(const char* filename) {
  displayPaused = true;
  vTaskDelay(100 / portTICK_PERIOD_MS);

  if (readImage((char*)filename, pixels)) {
    taskENTER_CRITICAL(&pixelMux);
    strncpy(currentImage, filename, sizeof(currentImage) - 1);
    currentImage[sizeof(currentImage) - 1] = '\0';
    newImageAvailable = true;
    taskEXIT_CRITICAL(&pixelMux);
    Serial.print("Switched to image: ");
    Serial.println(filename);
  } else {
    Serial.println("Failed to load image.");
  }

  displayPaused = false;
}

void setup() {
  pinMode(hall_sensor, INPUT_PULLUP);  // Enable pullup for hall sensor
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n================================");
  Serial.println("ESP32 POV Display with Hall Sensor");
  Serial.println("================================\n");
  
  // Initialize FastLED first
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  
  // Initialize SD card
  Serial.println("Initializing SD card...");
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("✗ SD Card initialization failed!");
    Serial.println("Check wiring and SD card");
    while (1) delay(1000);
  }
  
  Serial.println("✓ SD Card initialized");
  
  // Create images directory if it doesn't exist
  if (!SD.exists(IMAGE_DIR)) {
    SD.mkdir(IMAGE_DIR);
    Serial.println("✓ Created /images directory");
  }
  
  // Load image file list
  imageFiles.clear();
  File dir = SD.open("/images");
  if (!dir) {
    Serial.println("Failed to open /images directory");
  } else {
    File file = dir.openNextFile();
    while (file) {
      String name = file.name();
      if (!file.isDirectory()) {
        if (name.endsWith(".bmp") || name.endsWith(".BMP")) {
          imageFiles.push_back(name);
          Serial.print("Found: ");
          Serial.println(name);
        }
      }
      file = dir.openNextFile();
    }
    Serial.print("Loaded ");
    Serial.print(imageFiles.size());
    Serial.println(" BMP files.");
  }
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts = (attempts+1)%30;
    if (WiFi.status() != WL_CONNECTED && attempts == 29) {
      Serial.println("\n✗ WiFi connection failed!");
      Serial.println("Check your SSID and password");
    }
  }
  
  Serial.println("\n✓ WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/list", HTTP_GET, handleListImages);
  server.on("/upload", HTTP_POST, handleUploadResponse, handleUpload);
  server.on("/delete", HTTP_POST, handleDelete);
  server.on("/image", HTTP_GET, handleGetImage);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/select", HTTP_POST, handleSelectImage);
  
  server.begin();
  Serial.println("✓ Web server started");
  
  Serial.println("\n================================");
  Serial.println("Ready! Open this URL in your browser:");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("================================\n");

  // Load the first image if available
  if (imageFiles.size() > 0) {
    String firstImage = imageFiles[0];
    int lastSlash = firstImage.lastIndexOf('/');
    if (lastSlash >= 0) {
      firstImage = firstImage.substring(lastSlash + 1);
    }
    swapImage(firstImage.c_str());
  }
  
  // Initialize timing variables
  lastMicros = micros();
  currentRotationTime = 1000000; // Default to 1 second rotation
  
  // Start interval processing task on Core 0
  xTaskCreatePinnedToCore(
    processInterval,
    "ProcessInterval",
    4096,
    NULL,
    2,  // Higher priority for timing accuracy
    NULL,
    0
  );

  // Start display task on Core 1
  xTaskCreatePinnedToCore(
    displayTask,
    "DisplayTask",
    4096,
    NULL,
    1,
    NULL,
    1
  );
  
  // Attach hall sensor interrupt AFTER tasks are started
  attachInterrupt(digitalPinToInterrupt(hall_sensor), signalISR, FALLING);
  Serial.println("✓ Hall sensor interrupt attached");
}

void loop() {
  server.handleClient();
  
  // Refresh the image file list periodically (every 10 seconds)
  static unsigned long lastRefresh = 0;
  if (millis() - lastRefresh > 10000) {
    imageFiles.clear();
    File dir = SD.open("/images");
    if (dir) {
      File file = dir.openNextFile();
      while (file) {
        String name = file.name();
        if (!file.isDirectory()) {
          if (name.endsWith(".bmp") || name.endsWith(".BMP")) {
            imageFiles.push_back(name);
          }
        }
        file = dir.openNextFile();
      }
    }
    lastRefresh = millis();
  }
  
  delay(10);
}

bool readImage(char imageA[], uint8_t pixelsA[]) {
  char path[64];
  snprintf(path, sizeof(path), "/images/%s", imageA);

  File bmpFile = SD.open(path);
  if (!bmpFile) {
    Serial.print("Failed to open: ");
    Serial.println(path);
    return false;
  }

  // Skip BMP header (138 bytes for your format)
  bmpFile.seek(138);

  // Read pixel data (B, G, R)
  int pixelCount = 0;
  while (bmpFile.available() && pixelCount < 256*64*3) {
    pixelsA[pixelCount+0] = bmpFile.read();
    pixelsA[pixelCount+1] = bmpFile.read();
    pixelsA[pixelCount+2] = bmpFile.read();
    pixelCount += 3;
  }

  bmpFile.close();
  Serial.print("Loaded ");
  Serial.print(pixelCount / 3);
  Serial.println(" pixels");
  return true;
}

void handleSelectImage() {
  if (server.hasArg("filename")) {
    String filename = server.arg("filename");
    String filepath = String(IMAGE_DIR) + "/" + filename;
    if (SD.exists(filepath)) {
      swapImage(filename.c_str());
      server.send(200, "text/plain", "Image selected: " + filename);
    } else {
      server.send(404, "text/plain", "File not found");
    }
  } else {
    server.send(400, "text/plain", "Missing filename parameter");
  }
}

void handleListImages() {
  String json = "[";
  File root = SD.open(IMAGE_DIR);
  if (root && root.isDirectory()) {
    File file = root.openNextFile();
    bool first = true;
    while (file) {
      if (!file.isDirectory()) {
        String filename = String(file.name());
        int lastSlash = filename.lastIndexOf('/');
        if (lastSlash >= 0) {
          filename = filename.substring(lastSlash + 1);
        }
        if (!first) json += ",";
        json += "{\"name\":\"" + filename + "\",\"size\":" + String(file.size()) + "}";
        first = false;
      }
      file.close();
      file = root.openNextFile();
    }
    root.close();
  }
  json += "]";
  server.send(200, "application/json", json);
}

File uploadFile;

void handleUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    String filepath = String(IMAGE_DIR) + filename;
    Serial.print("Upload Start: ");
    Serial.println(filepath);
    displayPaused = true;
    uploadFile = SD.open(filepath, FILE_WRITE);
    if (!uploadFile) {
      Serial.println("Failed to open file for writing");
    }
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } 
  else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      Serial.print("Upload End: ");
      Serial.print(upload.totalSize);
      Serial.println(" bytes");
      displayPaused = false;
    }
  }
}

void handleUploadResponse() {
  server.send(200, "text/plain", "Upload successful");
}

void handleDelete() {
  if (server.hasArg("filename")) {
    String filename = server.arg("filename");
    String filepath = String(IMAGE_DIR) + "/" + filename;
    if (SD.exists(filepath)) {
      SD.remove(filepath);
      Serial.println("Deleted: " + filepath);
      server.send(200, "text/plain", "Deleted successfully");
    } else {
      server.send(404, "text/plain", "File not found");
    }
  } else {
    server.send(400, "text/plain", "Missing filename parameter");
  }
}

void handleGetImage() {
  if (server.hasArg("file")) {
    String filename = server.arg("file");
    String filepath = String(IMAGE_DIR) + "/" + filename;
    if (SD.exists(filepath)) {
      File file = SD.open(filepath);
      if (file) {
        String contentType = "image/jpeg";
        if (filename.endsWith(".png")) contentType = "image/png";
        else if (filename.endsWith(".gif")) contentType = "image/gif";
        else if (filename.endsWith(".bmp")) contentType = "image/bmp";
        server.streamFile(file, contentType);
        file.close();
        return;
      }
    }
  }
  server.send(404, "text/plain", "Image not found");
}

void handleStatus() {
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  uint64_t usedSpace = SD.usedBytes() / (1024 * 1024);
  uint64_t freeSpace = (SD.totalBytes() - SD.usedBytes()) / (1024 * 1024);
  
  // Get current RPM
  float rpm = 0;
  taskENTER_CRITICAL(&pixelMux);
  if (currentRotationTime > MIN_ROTATION_TIME) {
    rpm = 60000000.0 / currentRotationTime;
  }
  taskEXIT_CRITICAL(&pixelMux);
  
  String json = "{";
  json += "\"cardSize\":" + String((unsigned long)cardSize) + ",";
  json += "\"usedSpace\":" + String((unsigned long)usedSpace) + ",";
  json += "\"freeSpace\":" + String((unsigned long)freeSpace) + ",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"currentImage\":\"" + String(currentImage) + "\",";
  json += "\"rpm\":" + String(rpm, 1);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}


