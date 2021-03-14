#include <stdio.h>
#include <tusb.h>
#include "pico/stdlib.h"
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include "ILI9341.h"
#include "ei_run_classifier.h"

#define I2C_SDA 8
#define I2C_SCL 9
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
#define INPUT_LENGTH 768

static float mlx90640To[INPUT_LENGTH];
paramsMLX90640 mlx90640;
static uint8_t MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640
float features[INPUT_LENGTH];

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

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
        printf("Parameter extraction failed. status=%d\n", status);
    }

    return true; 
}

bool get_thermal_readings(float* input, int length) 
{ 
    //Read both subpages 
    for (uint8_t x = 0 ; x < 2 ; x++){
        uint16_t mlx90640Frame[834];
        int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
        if (status < 0) {
            printf("GetFrame Error: %d\n", status);
            return false;
        }

        float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
        float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

        float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
        float emissivity = 0.95;

        MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
    }

    int i = 0;
    float val;
    uint16_t color;

    clear_buffer();

    for (uint8_t x = 0; x < 32; x++) {
        for (uint8_t y = 0; y < 24; y++) {
            input[i] = mlx90640To[24 * x + y];
            i++;
            val = mlx90640To[32 * (23 - y) + x];
            if (val > 99.99) val = 99.99;

            if (val > 40.0) {
              color = ILI9341_WHITE;
            } else if (val > 32.0) {
              color = ILI9341_RED;
            } else if (val > 29.0) {
              color = ILI9341_ORANGE;
            } else if ( val > 26.0 ) {
              color = ILI9341_YELLOW;
            } else if ( val > 23.0 ) {
              color = ILI9341_GREEN;
            } else if ( val > 20.0 ) {
              color = ILI9341_BLUE;
            } else {
              color = ILI9341_BLACK;
            }

            draw_rectangle(y * 10, x * 10, 10, 10, color);
        }
    }

    write_buffer();

    return true;
}

int main()
{
    stdio_init_all();
    //while (!tud_cdc_connected()) { sleep_ms(100);  }
    sleep_ms(1000);

    const uint LED_PIN = 3;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    if (! init_thermal_camera()) {
        printf("Thermal Camera Initialization failed!\n");
        return 1;
    }

    init_SPI();
    init_display();
    init_drawing();

    ei_impulse_result_t result = { 0 };
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    while (1) {
        if (! get_thermal_readings(features, INPUT_LENGTH)) {
            sleep_ms(100);
            continue;
        }


        if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            printf("The size of your 'features' array is not correct. Expected %d items, but had %u\n",
               EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
            printf("Exited.");
            return 1;
        }
        // invoke the impulse
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
        printf("run_classifier returned: %d\n", res);

        if (res != 0) return 1;

        printf("Predictions (DSP: %d ms., Classification: %d ms.): \n",
            result.timing.dsp, result.timing.classification);

        // print the predictions
        printf("[");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf_float(result.classification[ix].value);
            if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
                printf(", ");
            }
        }
        printf("]\n");

        // On person detected, turn LED on
        if (result.classification[2].value >= 0.90f) {
            printf("Person Detected!");
            gpio_put(LED_PIN, 1);
        } else {
            gpio_put(LED_PIN, 0);
        }

        sleep_ms(50);
    }
    return 0;
}
