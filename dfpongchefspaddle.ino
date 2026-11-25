/*
  DF Pong: Sandwich Recipe Book Color Controller (Smoothed + Multi-Channel Diff)
  Runs on Adafruit Feather ESP32 V2 with an APDS-9999 RGB/IR sensor to map
  flipped recipe-book pages to paddle movement in DF Pong.
*/

#include <DFPongController.h>
#include <Adafruit_APDS9999.h>

DFPongController controller;
Adafruit_APDS9999 apds;

int controllerNumber = 6;

// Latest raw RGB and IR readings from the sensor
uint32_t r, g, b, ir;

// Tracks the current paddle target position (0 = UP, 50 = CENTER, 100 = DOWN)
int currentTarget = 50;

void setup()
{

  if (!apds.begin())
  {
    Serial.println("No sensor found: check the STEMMA QT cable");
  }
  
  // Enable ambient light sensing and switch to RGB measurement mode
  apds.enableLightSensor(true);
  apds.setRGBMode(true);

  controller.setControllerNumber(controllerNumber);
  controller.setStatusLED(LED_BUILTIN);

  controller.setControllerName("Chef's Paddle");
  controller.setPlayerColor("#ffcc00");
  controller.setMoveSpeed(10);
  controller.setPaddleSize(110);

  //sound & audio from game CookingMama
  controller.setHitSound("glass");       
  controller.setScoreSound("https://raw.githubusercontent.com/elisachen0725/dfpongscoresound/main/scoresound.mp3");     
  controller.setWinSound("https://raw.githubusercontent.com/elisachen0725/dfpongwinsound/main/winsound.mp3");   
  controller.setMusic("https://raw.githubusercontent.com/elisachen0725/dfpongbackgroundmusic/main/dfpongbgmusic.mp3");

  controller.begin();
}

void loop()
{
  controller.update();

  // 1. Take 5 rapid samples and average them to reduce single-read sensor noise
  long sumR = 0, sumG = 0, sumB = 0;
  for (int i = 0; i < 5; i++)
  {
    apds.getRGBIRData(&r, &g, &b, &ir);
    sumR += r;
    sumG += g;
    sumB += b;
    delay(4);
  }
  int avgR = sumR / 5;
  int avgG = sumG / 5;
  int avgB = sumB / 5;

  // 2. Compute channel differences to separate color hues more reliably than raw brightness
  int diff_gr = avgG - avgR;
  int diff_gb = avgG - avgB; // Computed for separating similar green/yellow tones
  int diff_rb = avgR - avgB; // Computed for comparing red against blue

  // Print averaged values and differences to Serial Monitor for calibration
  Serial.print("R: "); Serial.print(avgR);
  Serial.print(" | G: "); Serial.print(avgG);
  Serial.print(" | B: "); Serial.print(avgB);
  Serial.print(" | G-R: "); Serial.print(diff_gr);
  Serial.print(" | G-B: "); Serial.print(diff_gb);
  Serial.print(" | R-B: "); Serial.println(diff_rb);

  // 3. Map G - R difference thresholds to paddle targets:
  //    Red (tomato) -> DOWN (100), Green (lettuce) -> UP (0), Yellow (cheese) -> CENTER (50)
  if (diff_gr <= -18) {
    currentTarget = 100;
    Serial.println("Detected: RED -> Moving DOWN");
  } else if (diff_gr >= -3 && diff_gr <= 2) {
    currentTarget = 0;
    Serial.println("Detected: GREEN -> Moving UP");
  } else {
    currentTarget = 50;
    Serial.println("Detected: YELLOW -> Center");
  }

  // Send the target paddle position normalized to [0, 100]
  controller.sendPosition(currentTarget, 0, 100);

  // Short pacing delay between loop iterations
  delay(20);
}
