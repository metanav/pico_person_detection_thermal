#include <stdio.h>
#include <tusb.h>
#include "pico/stdlib.h"
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#define I2C_SDA 8
#define I2C_SCL 9
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
#define INPUT_LENGTH 768

static float mlx90640To[INPUT_LENGTH];
paramsMLX90640 mlx90640;
static uint8_t MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

bool init_thermal_camera()
{
    if (! MLX90640_I2CInit(MLX90640_address)) {
        return false;
    }

    //Get device parameters - We only have to do this once
    int status;
    uint16_t eeMLX90640[832];

    status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);

    if (status != 0) {
        printf("Failed to load system parameters");
    }

    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);

    if (status != 0) {
        printf("Parameter extraction failed.\n");
    }
    

    return true; 
}

float get_thermal_image(float* input, int length) 
{ 
    //Read both subpages 
    for (uint8_t x = 0 ; x < 2 ; x++){
        uint16_t mlx90640Frame[834];
        int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
        if (status < 0) {
            printf("GetFrame Error: %d\n", status);
            return -100.0; // return big enough negative value to show error
        }

        float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
        float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

        float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
        float emissivity = 0.95;

        MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
    }

    int i = 0;
    float max_temp = 0.0;
    for (uint8_t x = 0; x < 32; x++) {
        for (uint8_t y = 0; y < 24; y++) {
            input[i] = mlx90640To[24 * x + y];
            if (input[i] > max_temp) {
                max_temp = input[i];
            }
            i++;
        }
    }

    return max_temp;
}

int main()
{
    stdio_init_all();
    while (!tud_cdc_connected()) { sleep_ms(100);  }

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    if (init_thermal_camera()) {
        printf("Thermal Camera Initialized.\n");
        float input[INPUT_LENGTH];

        while (1) {
            float max_reading = get_thermal_image(input, INPUT_LENGTH);
            printf("Max. Temp = %0.2f\n", max_reading);
            sleep_ms(500);
        }
    } else {
        printf("Thermal Camera Initialization failed!\n");
    }

    return 0;
}
