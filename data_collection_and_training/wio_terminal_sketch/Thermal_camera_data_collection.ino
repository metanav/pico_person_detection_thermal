#include <Wire.h>
#include <SPI.h>
#include "TFT_eSPI.h"
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#define TA_SHIFT 8 //Default shift for MLX90640 in open air

paramsMLX90640 mlx90640;
TFT_eSPI tft;

const byte MLX90640_address = 0x33;
static float mlx90640To[768];
char filename[50];
int  file_count;

void readFile(fs::FS& fs, const char* path) {
  Serial.print("Reading file: ");
  Serial.println(path);
  File file = fs.open(path);

  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS& fs, const char* path, String data) {
  Serial.print("Writing file: ");
  Serial.println(path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  // write in chunks of 512 bytes
  for ( int i = 0; i < data.length(); i = i + 512) {
    if (!file.print(data.substring(i, i + 512))) {
      Serial.println("Write failed");
    }
  }


  file.close();
}

void deleteFile(fs::FS& fs, const char* path) {
  Serial.print("Deleting file: ");
  Serial.println(path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void setup()
{
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); 
  
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz

  Serial.begin(115200);
  delay(1000);
 // while (!Serial); //Wait for user to open terminal

  if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("MLX90640 online!");

  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");

  //Once params are extracted, we can release eeMLX90640 array
  tft.begin();
  tft.setRotation(3);

  while (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  pinMode(WIO_BUZZER, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  for (byte x = 0 ; x < 2 ; x++) //Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }

  uint32_t color;
  uint8_t label_id = 0;

  if (digitalRead(WIO_KEY_A) == LOW) {
    Serial.println("A Key pressed");
    label_id = 1;

  }
  else if (digitalRead(WIO_KEY_B) == LOW) {
    Serial.println("B Key pressed");
    label_id = 2;
  }
  else if (digitalRead(WIO_KEY_C) == LOW) {
    Serial.println("C Key pressed");
    label_id = 3;
  }

  String data;
  for (uint8_t x = 0; x < 32; x++) {
    for (uint8_t y = 0; y < 24; y++) {
      if (x == 0 && y == 0) {
        data = mlx90640To[24 * x + y];
      } else {
        data = data + "," +  mlx90640To[24 * x + y];
      }

      float val = mlx90640To[32 * (23 - y) + x];
      if (val > 99.99) val = 99.99;

      if (val > 32.0) {
        color = TFT_MAGENTA;
      }
      else if (val > 29.0) {
        color = TFT_RED;
      }
      else if (val > 26.0) {
        color = TFT_YELLOW;
      }
      else if ( val > 20.0 ) {
        color = TFT_GREENYELLOW;
      }
      else if (val > 17.0) {
        color = TFT_GREEN;
      }
      else if (val > 10.0) {
        color = TFT_CYAN;
      }
      else {
        color = TFT_BLUE;
      }

      tft.fillRect(x * 10, y * 10, 10, 10, color);
    }
  }

  if (label_id > 0) {
    analogWrite(WIO_BUZZER, 128);
    sprintf(filename, "/readings_%d_label_%d.csv", file_count, label_id);
    Serial.println(data);
    writeFile(SD, filename, data);
    file_count++;
  }
  delay(500);

  if (label_id > 0) {
    analogWrite(WIO_BUZZER, 0);
  }
}

//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}
