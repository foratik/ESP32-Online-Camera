#include "esp_camera.h"
#include <WiFi.h>
#include "camera_pins.h"


const char* ssid = "XXXXXXXX";
const char* password = "xxxxxxxxxxxx";
const char* serverIP = "XXX.XXX.XXX.XXX";
const int serverPort = 5000;

extern uint16_t encode8b10b(uint8_t input, bool *rd);

WiFiServer server(80);

void sendPhotoToVPS(camera_fb_t *fb) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (!client.connect(serverIP, serverPort)) {
      Serial.println("Connection to server failed");
      return;
    }

    String boundary = "----ESP32Boundary";
    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"photo\"; filename=\"photo.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";

    int contentLength = bodyStart.length() + fb->len + bodyEnd.length();

    client.println("POST /send_photo HTTP/1.1");
    client.println("Host: " + String(serverIP));
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    client.println("Content-Length: " + String(contentLength));
    client.println();
    
    client.print(bodyStart);
    client.write(fb->buf, fb->len);
    client.print(bodyEnd);

    int timeout = millis() + 5000;
    while (client.connected() && millis() < timeout) {
      if (client.available()) {
        String response = client.readStringUntil('\r');
        Serial.println("Response: " + response);
        break;
      }
    }
    client.stop();
  } else {
    Serial.println("WiFi not connected");
  }
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
