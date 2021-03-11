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
    uint8_t cmd[2];
    cmd[0] = startAddress >> 8;
    cmd[1] = startAddress & 0x00FF;

    int ret;
    ret = i2c_write_blocking(I2C_PORT, _deviceAddress, cmd, 2, true); // true to keep master control of bus

    if (ret == PICO_ERROR_GENERIC) {
        return -1;
    }

    uint16_t len = nWordsRead * 2;
    uint8_t buffer[len];
    ret = i2c_read_blocking(I2C_PORT, _deviceAddress, buffer, len, false);
   
    if (ret == PICO_ERROR_GENERIC) {
        return -1;
    }

    for (int i = 0; i < nWordsRead; i++) {
        data[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }

    return 0; //Success
}

//Write two bytes to a two byte address
int MLX90640_I2CWrite(uint8_t _deviceAddress, unsigned int writeAddress, uint16_t data)
{
    uint8_t cmd[4];
    cmd[0] = writeAddress >> 8;
    cmd[1] = writeAddress & 0x00FF;
    cmd[2] = data >> 8;
    cmd[3] = data & 0x00FF;

    int ret = i2c_write_blocking(I2C_PORT, _deviceAddress, cmd, 4, false);
    if (ret == PICO_ERROR_GENERIC) {
        //Sensor did not ACK
        printf("Error: Sensor did not ack.\n");
        return -1;
    }

    static uint16_t dataCheck;

    MLX90640_I2CRead(_deviceAddress, writeAddress, 1, &dataCheck);
    if (dataCheck != data) {
       //printf("The write request didn't stick.\n");
       return -2;
    }

    return 0; //Success
}
