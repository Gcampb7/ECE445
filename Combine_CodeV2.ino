#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <FastLED.h>

// WiFi credentials - CHANGE THESE!
const char* ssid = "UIUC";
const char* password = "billybob";

// SD Card pins
#define SD_CS    5
#define SD_MOSI  23
#define SD_MISO  19
#define SD_SCK   18

// FastLED Info
#define LED_PIN     4        // Data pin connected to LED strip
#define NUM_LEDS    70       // Number of LEDs in your strip
#define COLOR_ORDER GRB
#define LED_TYPE    WS2812B
#define BRIGHTNESS  255      // 0 - 255
CRGB leds[NUM_LEDS];

// Image directory on SD card
#define IMAGE_DIR "/images"

// Create web server on port 80
WebServer server(80);

int hall_sensor = 14;
int num_leds = 16;
int spin_count = 0;
int end_time = 0;
int start_time = 0;
float rotation_time = 0;
float degree = 0;
int shifting = 0;
int shifting_leds = 0;
int anima_flag;
uint8_t pixels[256*64*3];
char image[1000];

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
//  Initialize second processor for synchronization

  pinMode(hall_sensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_sensor), signalISR, FALLING);

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

  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n================================");
  Serial.println("ESP32 WiFi Image Server");
  Serial.println("================================\n");
  
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
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n✗ WiFi connection failed!");
    Serial.println("Check your SSID and password");
    while (1) delay(1000);
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
  server.onNotFound(handleNotFound);
  
  // Start server
  server.begin();
  Serial.println("✓ Web server started");
  
  Serial.println("\n================================");
  Serial.println("Ready! Open this URL in your browser:");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("================================\n");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
   Serial.println("Start handleClient");
   server.handleClient();
   Serial.println("End handleClient");
   degree = (min((float)thousand, max((float)forty, ((float)intervalMicros)/1000)/360))*1.03;

   // Image Display Code
   if (digitalRead(hall_sensor) == LOW && spin_count <= 49) {
     spin_count = (spin_count + 1) % 200;

     strcpy(image, "/images/test2.bmp");   // Some sort of name retrieval is needed here
     readImage(image, pixels);
     leds[0] = CRGB(255, 255, 255);
     leds[1] = CRGB(0, 0, 0);
     FastLED.show();
     delay(1000);
     leds[0] = CRGB(0, 0, 0);
     leds[1] = CRGB(255, 0, 255);
     delay(1000);
//     for (int i=0; i<256; i++) {
//     // Write the first three rgb values of pixel to the three color channel pins. Our drivers will decide which LEDS are to be written too at any given time.
//       for (int j=0; j<64; j++)
         // Serial.println(255-pixels[(i*64*3)+(j*3)+0]);
  //     analogWrite(27, 255-pixels[(i*64*3)+(j*3)+0]);
  //     analogWrite(32, 255-pixels[(i*64*3)+(j*3)+1]);
  //     analogWrite(33, 255-pixels[(i*64*3)+(j*3)+2]);
         // I assume here we are controlling all 64 LEDS individually so we need a 6 to 64 decoder.
//         digitalWrite(Decodepin0, countinbinary); // Count in binary to account for all LEDS that need to be adressed individually
//         digitalWrite(Decodepin0, countinbinary);
//         digitalWrite(Decodepin0, countinbinary);
//         digitalWrite(Decodepin0, countinbinary);
//         digitalWrite(Decodepin0, countinbinary);
//         digitalWrite(Decodepin0, countinbinary);
         // The image should be 256 wide so we split our circle into 256 parts and 64 parts per led turnig on
         // If we want to control 8 LEDS at a time equation would look like (256/8) as opposed to controlling all 64 individually.
//            delay((360*degree)/256/64);
   }
}

// Function call to read pixel data from image
void readImage(char imageA[], uint8_t pixelsA[]) {
  int pixelCount = 0;
  // File bmpFile = SD.open("/images/test2.bmp");
  File bmpFile = SD.open(imageA);
  if (!bmpFile) {
    // Serial.println("Failed to open test.bmp");
    return;
  }

  // Skip BMP header (first 54 bytes)
  bmpFile.seek(54);

  // Read pixel data (B, G, R)
  while (bmpFile.available()) {
  //  Write image data to array.
      pixelsA[pixelCount+0] = bmpFile.read();
      pixelsA[pixelCount+1] = bmpFile.read();
      pixelsA[pixelCount+2] = bmpFile.read();
      pixelCount = pixelCount+3;
  }

  bmpFile.close();
}

// List all images
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

// Handle file upload
File uploadFile;

void handleUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    
    String filepath = String(IMAGE_DIR) + filename;
    Serial.print("Upload Start: ");
    Serial.println(filepath);
    
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
    }
  }
}

void handleUploadResponse() {
  server.send(200, "text/plain", "Upload successful");
}

// Delete an image
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

// Get an image
void handleGetImage() {
  if (server.hasArg("file")) {
    String filename = server.arg("file");
    String filepath = String(IMAGE_DIR) + "/" + filename;
    
    if (SD.exists(filepath)) {
      File file = SD.open(filepath);
      if (file) {
        // Determine content type based on extension
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

// Get status
void handleStatus() {
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  uint64_t usedSpace = SD.usedBytes() / (1024 * 1024);
  uint64_t freeSpace = (SD.totalBytes() - SD.usedBytes()) / (1024 * 1024);
  
  String json = "{";
  json += "\"cardSize\":" + String((unsigned long)cardSize) + ",";
  json += "\"usedSpace\":" + String((unsigned long)usedSpace) + ",";
  json += "\"freeSpace\":" + String((unsigned long)freeSpace) + ",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

// Handle 404
void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Image Manager</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        h1 {
            color: #333;
            margin-bottom: 10px;
            text-align: center;
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
        }
        .status {
            padding: 15px;
            border-radius: 10px;
            margin-bottom: 20px;
            text-align: center;
            font-weight: 600;
        }
        .status.connected {
            background: #e8f5e9;
            color: #2e7d32;
        }
        .upload-section {
            background: #f5f5f5;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 30px;
        }
        .file-input-wrapper {
            position: relative;
            margin-bottom: 15px;
        }
        input[type="file"] {
            width: 100%;
            padding: 12px;
            border: 2px dashed #667eea;
            border-radius: 8px;
            cursor: pointer;
        }
        button {
            padding: 12px 24px;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
        }
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        }
        .btn-danger {
            background: #f44336;
            color: white;
            padding: 8px 16px;
            font-size: 14px;
        }
        .btn-view {
            background: #2196f3;
            color: white;
            padding: 8px 16px;
            font-size: 14px;
            margin-right: 10px;
        }
        .progress-bar {
            width: 100%;
            height: 30px;
            background: #e0e0e0;
            border-radius: 15px;
            overflow: hidden;
            margin-top: 15px;
            display: none;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
            width: 0%;
            transition: width 0.3s;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: 600;
        }
        .gallery {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        .image-card {
            background: #f5f5f5;
            border-radius: 10px;
            overflow: hidden;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        .image-card img {
            width: 100%;
            height: 200px;
            object-fit: cover;
            cursor: pointer;
        }
        .image-info {
            padding: 15px;
        }
        .image-name {
            font-weight: 600;
            margin-bottom: 5px;
            word-break: break-all;
        }
        .image-size {
            color: #666;
            font-size: 14px;
            margin-bottom: 10px;
        }
        .image-actions {
            display: flex;
            gap: 10px;
        }
        .empty-state {
            text-align: center;
            padding: 60px 20px;
            color: #999;
        }
        .empty-state svg {
            width: 100px;
            height: 100px;
            margin-bottom: 20px;
            opacity: 0.3;
        }
        .modal {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background: rgba(0,0,0,0.9);
            justify-content: center;
            align-items: center;
        }
        .modal.active {
            display: flex;
        }
        .modal img {
            max-width: 90%;
            max-height: 90%;
            border-radius: 10px;
        }
        .modal-close {
            position: absolute;
            top: 20px;
            right: 40px;
            color: white;
            font-size: 40px;
            cursor: pointer;
            font-weight: 300;
        }
        @media (max-width: 768px) {
            .gallery {
                grid-template-columns: repeat(auto-fill, minmax(150px, 1fr));
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📷 ESP32 Image Manager</h1>
        <p class="subtitle">Upload, view, and manage images on your ESP32</p>
        
        <div class="status connected">
            Connected to ESP32
        </div>
        
        <div class="upload-section">
            <h3>Upload Image</h3>
            <div class="file-input-wrapper">
                <input type="file" id="fileInput" accept="image/*" multiple>
            </div>
            <button class="btn-primary" onclick="uploadFiles()">Upload Selected Images</button>
            <div class="progress-bar" id="progressBar">
                <div class="progress-fill" id="progressFill">0%</div>
            </div>
        </div>
        
        <h3>Image Gallery</h3>
        <div class="gallery" id="gallery">
            <div class="empty-state">
                <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z" />
                </svg>
                <p>No images yet. Upload some images to get started!</p>
            </div>
        </div>
    </div>
    
    <div class="modal" id="imageModal" onclick="closeModal()">
        <span class="modal-close">&times;</span>
        <img id="modalImage" src="" alt="Full size image">
    </div>
    
    <script>
        // Load images on page load
        window.onload = () => {
            loadImages();
        };
        
        async function loadImages() {
            try {
                const response = await fetch('/list');
                const images = await response.json();
                
                const gallery = document.getElementById('gallery');
                gallery.innerHTML = '';
                
                if (images.length === 0) {
                    gallery.innerHTML = `
                        <div class="empty-state">
                            <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z" />
                            </svg>
                            <p>No images yet. Upload some images to get started!</p>
                        </div>
                    `;
                    return;
                }
                
                images.forEach(image => {
                    const card = document.createElement('div');
                    card.className = 'image-card';
                    card.innerHTML = `
                        <img src="/image?file=${encodeURIComponent(image.name)}" alt="${image.name}" onclick="viewImage('${image.name}')">
                        <div class="image-info">
                            <div class="image-name">${image.name}</div>
                            <div class="image-size">${formatBytes(image.size)}</div>
                            <div class="image-actions">
                                <button class="btn-view" onclick="viewImage('${image.name}')">View</button>
                                <button class="btn-danger" onclick="deleteImage('${image.name}')">Delete</button>
                            </div>
                        </div>
                    `;
                    gallery.appendChild(card);
                });
            } catch (error) {
                console.error('Error loading images:', error);
                alert('Failed to load images');
            }
        }
        
        async function uploadFiles() {
            const fileInput = document.getElementById('fileInput');
            const files = fileInput.files;
            
            if (files.length === 0) {
                alert('Please select at least one image');
                return;
            }
            
            const progressBar = document.getElementById('progressBar');
            const progressFill = document.getElementById('progressFill');
            progressBar.style.display = 'block';
            
            for (let i = 0; i < files.length; i++) {
                const file = files[i];
                const formData = new FormData();
                formData.append('file', file);
                
                try {
                    progressFill.textContent = `Uploading ${i + 1} of ${files.length}...`;
                    progressFill.style.width = ((i / files.length) * 100) + '%';
                    
                    const response = await fetch('/upload', {
                        method: 'POST',
                        body: formData
                    });
                    
                    if (!response.ok) {
                        throw new Error('Upload failed');
                    }
                } catch (error) {
                    console.error('Upload error:', error);
                    alert(`Failed to upload ${file.name}`);
                }
            }
            
            progressFill.textContent = 'Complete!';
            progressFill.style.width = '100%';
            
            setTimeout(() => {
                progressBar.style.display = 'none';
                progressFill.style.width = '0%';
                fileInput.value = '';
                loadImages();
            }, 1500);
        }
        
        async function deleteImage(filename) {
            if (!confirm(`Delete ${filename}?`)) {
                return;
            }
            
            try {
                const response = await fetch('/delete', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: 'filename=' + encodeURIComponent(filename)
                });
                
                if (response.ok) {
                    loadImages();
                } else {
                    alert('Failed to delete image');
                }
            } catch (error) {
                console.error('Delete error:', error);
                alert('Failed to delete image');
            }
        }
        
        function viewImage(filename) {
            const modal = document.getElementById('imageModal');
            const modalImage = document.getElementById('modalImage');
            modalImage.src = '/image?file=' + encodeURIComponent(filename);
            modal.classList.add('active');
        }
        
        function closeModal() {
            const modal = document.getElementById('imageModal');
            modal.classList.remove('active');
        }
        
        function formatBytes(bytes) {
            if (bytes === 0) return '0 Bytes';
            const k = 1024;
            const sizes = ['Bytes', 'KB', 'MB'];
            const i = Math.floor(Math.log(bytes) / Math.log(k));
            return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
        }
    </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}
