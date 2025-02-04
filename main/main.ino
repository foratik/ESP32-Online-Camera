#include "esp_camera.h"
#include <WiFi.h>
#include "camera_pins.h"


const char* ssid = "XXXXXXXX";
const char* password = "xxxxxxxxxxxx";
const char* serverIP = "XXX.XXX.XXX.XXX";
const int serverPort = 5000;

extern uint16_t encode8b10b(uint8_t input, bool *rd);

WiFiServer server(80);

void encodeData8b10b(const uint8_t* input, size_t len, uint8_t* output, size_t* outLen) {
  bool rd = false;  // Running disparity
  size_t outIdx = 0;
  uint16_t bitBuffer = 0;
  int bitCount = 0;

  for (size_t i = 0; i < len; i++) {
    uint16_t symbol = encode8b10b(input[i], &rd);

    // Pack 10-bit symbols into byte stream
    bitBuffer = (bitBuffer << 10) | symbol;
    bitCount += 10;

    while (bitCount >= 8) {
      output[outIdx++] = (bitBuffer >> (bitCount - 8)) & 0xFF;
      bitCount -= 8;
    }
  }

  // Handle remaining bits
  if (bitCount > 0) {
    output[outIdx++] = (bitBuffer << (8 - bitCount)) & 0xFF;
  }

  *outLen = outIdx;
}

// =============================================
// Modified Photo Sending Function
// =============================================
void sendPhotoToVPS(camera_fb_t *fb) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  WiFiClient client;
  if (!client.connect(serverIP, serverPort)) {
    Serial.println("Connection failed");
    return;
  }

  // Encode the image data
  size_t encodedSize = (fb->len * 10 + 7) / 8;  // Calculate output size
  uint8_t* encodedData = (uint8_t*)malloc(encodedSize);
  size_t actualEncodedSize;
  
  encodeData8b10b(fb->buf, fb->len, encodedData, &actualEncodedSize);

  // Send HTTP headers
  String boundary = "ESP32Boundary";
  String header = "POST /upload HTTP/1.1\r\n";
  header += "Host: " + String(serverIP) + "\r\n";
  header += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
  header += "Content-Length: " + String(actualEncodedSize + boundary.length() * 2 + 8) + "\r\n\r\n";
  
  client.print(header);
  client.print("--" + boundary + "\r\n");
  client.print("Content-Disposition: form-data; name=\"image\"; filename=\"encoded.img\"\r\n");
  client.print("Content-Type: application/octet-stream\r\n\r\n");

  // Send encoded data
  client.write(encodedData, actualEncodedSize);

  // Send footer
  client.print("\r\n--" + boundary + "--\r\n");
  
  free(encodedData);
  client.stop();
}

void startCameraServer() {
  server.begin();
  Serial.println("Camera Ready! Connect to http://your-esp32-cam-ip");
}

void handleClientStream(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println();

  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      client.stop();
      return;
    }

    client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.println("\r\n");

    esp_camera_fb_return(fb);

    if (!client.connected()) break;
  }
}

void handleSerialCommand(void *parameter) {
  while (true) {
    if (Serial.available()) {
      char command = Serial.read();
      if (command == 'p') {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
          sendPhotoToVPS(fb);
          esp_camera_fb_return(fb);
          Serial.println("Photo sent to VPS");
        } else {
          Serial.println("Failed to capture photo");
        }
      }
    }
    delay(10);
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  startCameraServer();
  xTaskCreate(handleSerialCommand, "SerialCommandTask", 4096, NULL, 1, NULL);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClientStream(client);
  }
}
