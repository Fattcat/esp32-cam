#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"

// ===== PINOUT AI THINKER =====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ===== FLASH LED =====
#define FLASH_LED_PIN 4
bool FlashLight = true;

// ===== DIRECTORY =====
#define IMAGE_DIR "/esp32s-cam"

int photoNumber = 1;

// ============================
// 📁 CREATE DIRECTORY
// ============================
void ensureDirExists() {
  if (!SD_MMC.exists(IMAGE_DIR)) {
    Serial.println("Creating directory...");
    if (SD_MMC.mkdir(IMAGE_DIR)) {
      Serial.println("Directory created");
    } else {
      Serial.println("Failed to create directory");
    }
  } else {
    Serial.println("Directory exists");
  }
}

// ============================
// 🔢 FILE NAME GENERATOR
// ============================
String getFileName() {
  char fileName[50];

  while (true) {
    sprintf(fileName, IMAGE_DIR "/img%06d.jpg", photoNumber);

    if (!SD_MMC.exists(fileName)) {
      return String(fileName);
    }

    photoNumber++;
  }
}

// ============================
// 📸 CAMERA INIT
// ============================
void initCamera() {

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;

  // ===== DETEKCIA PSRAM =====
  if (psramFound()) {
    config.frame_size   = FRAMESIZE_UXGA; // 1600x1200
    config.jpeg_quality = 10;
    config.fb_count     = 2;
  } else {
    config.frame_size   = FRAMESIZE_SVGA; // fallback
    config.jpeg_quality = 12;
    config.fb_count     = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("Camera init failed");
    while (true);
  }

  // ===== OTOČENIE OBRAZU =====
  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
}

// ============================
// 📷 CAPTURE PHOTO
// ============================
void capturePhoto() {

  if (FlashLight) {
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(150);
  }

  camera_fb_t * fb = esp_camera_fb_get();

  if (FlashLight) {
    digitalWrite(FLASH_LED_PIN, LOW);
  }

  if (!fb) {
    Serial.println("Capture failed");
    return;
  }

  String path = getFileName();
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);

  if (!file) {
    Serial.println("File open failed");
  } else {
    file.write(fb->buf, fb->len);
    Serial.println("Saved: " + path);
  }

  file.close();
  esp_camera_fb_return(fb);
}

// ============================
// ⚙️ SETUP
// ============================
void setup() {
  Serial.begin(115200);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  initCamera();

  if (!SD_MMC.begin()) {
    Serial.println("SD Mount Failed");
    while (true);
  }

  ensureDirExists();

  Serial.println("System ready");
}

// ============================
// 🔁 LOOP
// ============================
void loop() {
  capturePhoto();
  delay(5000);
}
