#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/gpio.h"

// Configuration for AI Thinker Camera board
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22


const char* ssid = "daddy";
const char* password = "babytuapochoni";
const char* WS_HOST = "wss://iot-project-production-5bcd.up.railway.app:443/video/ws/stream";

// Performance tuning variables
#define TARGET_FPS 15              // Target frames per second
#define FRAME_INTERVAL (1000/TARGET_FPS) // Milliseconds between frames
unsigned long lastFrameTime = 0;
unsigned long frameCount = 0;
unsigned long totalLatency = 0;

using namespace websockets;
WebsocketsClient client;


const char* lets_encrypt_root_ca = R"(-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)";


esp_err_t init_camera() {
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
  
  // OPTIMIZATIONS FOR LATENCY:
  config.xclk_freq_hz = 20000000;     // Higher frequency for faster capture
  config.pixel_format = PIXFORMAT_JPEG;
  
  // REDUCED RESOLUTION FOR SPEED:
  config.frame_size = FRAMESIZE_QVGA;  // 320x240 (was VGA 640x480)
  
  // OPTIMIZED QUALITY vs SIZE:
  config.jpeg_quality = 25;            // Increased from 15 for smaller files
  config.fb_count = 2;                 // Double buffering for smoother capture
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init FAIL: 0x%x", err);
    return err;
  }
  
  sensor_t * s = esp_camera_sensor_get();
  
  // ADDITIONAL OPTIMIZATIONS:
  s->set_framesize(s, FRAMESIZE_QVGA);   // Confirm resolution
  s->set_quality(s, 25);                 // Set quality
  
  // OPTIONAL: Enable grayscale for smaller files
  // s->set_special_effect(s, 2);        // Grayscale effect
  
  // Performance tuning
  s->set_brightness(s, 0);               // -2 to 2
  s->set_contrast(s, 0);                 // -2 to 2
  s->set_saturation(s, 0);               // -2 to 2
  s->set_gainceiling(s, (gainceiling_t)0); // Limit gain for consistent timing
  
  Serial.println("Camera init OK - Optimized for low latency");
  return ESP_OK;
}

esp_err_t init_wifi() {
  // WiFi performance optimizations
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.println("WiFi init...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // WiFi power management for better performance
  WiFi.setSleep(false);  // Disable WiFi sleep for lower latency
  
  Serial.println("");
  Serial.println("WiFi OK");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("Connecting to WebSocket...");
  
  bool connected = client.connect(WS_HOST);
  if (!connected) {
    Serial.println("WS connect failed!");
    return ESP_FAIL;
  }
  
  Serial.println("WebSocket OK");
  return ESP_OK;
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  Serial.setDebugOutput(false);  // Disable debug for better performance
  
  // Set CPU frequency to maximum for better performance
  setCpuFrequencyMhz(240);
  
  client.setCACert(lets_encrypt_root_ca);
  
  init_camera();
  init_wifi();
  
  Serial.println("Setup complete - Starting optimized streaming");
}

void loop() {
  client.poll();
  
  if (!client.available()) {
    delay(1); // Small delay to prevent busy waiting
    return;
  }
  
  // Frame rate control
  unsigned long currentTime = millis();
  if (currentTime - lastFrameTime < FRAME_INTERVAL) {
    return; // Skip this frame to maintain target FPS
  }
  
  unsigned long captureStart = micros();
  
  // Capture frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Capture failed");
    esp_camera_fb_return(fb);
    return; // Don't restart, just continue
  }
  
  unsigned long captureTime = micros() - captureStart;
  unsigned long sendStart = micros();
  
  // Send frame
  client.sendBinary((const char*)fb->buf, fb->len);
  
  unsigned long sendTime = micros() - sendStart;
  unsigned long totalTime = captureTime + sendTime;
  
  // Performance monitoring
  frameCount++;
  totalLatency += totalTime;
  
  if (frameCount % 100 == 0) {
    float avgLatency = totalLatency / 100.0 / 1000.0; // Convert to milliseconds
    Serial.printf("Frame: %lu, Size: %u bytes, Avg latency: %.1f ms\n", 
                  frameCount, fb->len, avgLatency);
    totalLatency = 0;
  }
  
  esp_camera_fb_return(fb);
  lastFrameTime = currentTime;
}