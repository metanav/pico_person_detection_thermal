/**
   @copyright (C) 2017 Melexis N.V.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include "hardware/i2c.h"
#include "MLX90640_I2C_Driver.h"

#define I2C_PORT i2c0

bool MLX90640_I2CInit(uint8_t _deviceAddress)
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    uint8_t rxdata;
    int ret = i2c_read_blocking(I2C_PORT, _deviceAddress, &rxdata, 1, false);

    if (ret < 0) {
        printf("MLX90640 not detected at provided I2C address. Please check wiring.\n");
        return false;
    }

    printf("MLX90640 detected!\n");

    return true;
}

//Read a number of words from startAddress. Store into Data array.
//Returns 0 if successful, -1 if error
int MLX90640_I2CRead(uint8_t _deviceAddress, unsigned int startAddress, unsigned int nWordsRead, uint16_t *data)
{
    //Caller passes number of 'unsigned ints to read', increase this to 'bytes to read'
    uint16_t len = nWordsRead * 2;
    int ret;
    uint8_t addr = (uint8_t) startAddress;
    ret = i2c_write_blocking(I2C_PORT, _deviceAddress, &addr, 2, true); // true to keep master control of bus

    if (ret == PICO_ERROR_GENERIC) {
        return -1;
    }

    ret = i2c_read_blocking(I2C_PORT, _deviceAddress, (uint8_t*)data, len, false);
   
    if (ret == PICO_ERROR_GENERIC) {
        return -1;
    }
    
    return 0; //Success
}

//Write two bytes to a two byte address
int MLX90640_I2CWrite(uint8_t _deviceAddress, unsigned int writeAddress, uint16_t data)
{
    uint8_t addr = (uint8_t) writeAddress;
    int ret;
    ret = i2c_write_blocking(I2C_PORT, _deviceAddress, &addr, 2, true);
    ret = i2c_write_blocking(I2C_PORT, _deviceAddress, (uint8_t*) &data, 2, true);
    if (ret == PICO_ERROR_GENERIC) {
        //Sensor did not ACK
        printf("Error: Sensor did not ack.\n");
        return -1;
    }

   uint16_t dataCheck;
   MLX90640_I2CRead(_deviceAddress, writeAddress, 1, &dataCheck);
   if (dataCheck != data) {
       //printf("The write request didn't stick.\n");
       return -2;
   }

    if (dataCheck != data) {
        //printf("The write request didn't stick.\n");
        return -2;
    }

    return 0; //Success
}
