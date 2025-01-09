/*
In this code:
- The GPIO pins for the stepper motor and encoder are defined.
- The PWM properties are set up using the LEDC (LED Control) driver.
- The `app_main` function configures the GPIO pins, PWM timer, and channel, and initializes the encoder.
- The motor direction is controlled by setting the `DIR_PIN` high or low.
- The PWM signal is generated on the `STEP_PIN` to control the stepper motor steps.
- The encoder value is read and printed to the console.

Make sure to adjust the GPIO pin numbers and PWM properties according to your specific setup and stepper motor requirements.

Let me know if you need any further assistance!
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/pulse_cnt.h"
#include "rom/gpio.h"
// Define the GPIO pins for the stepper motors
#define STEP_PIN_1 GPIO_NUM_18
#define DIR_PIN_1 GPIO_NUM_19
#define STEP_PIN_2 GPIO_NUM_21
#define DIR_PIN_2 GPIO_NUM_22
#define STEP_PIN_3 GPIO_NUM_23
#define DIR_PIN_3 GPIO_NUM_25

// Define the GPIO pins for the encoders
#define ENCODER_A_PIN_1 GPIO_NUM_32
#define ENCODER_B_PIN_1 GPIO_NUM_33
#define ENCODER_A_PIN_2 GPIO_NUM_26
#define ENCODER_B_PIN_2 GPIO_NUM_27
#define ENCODER_A_PIN_3 GPIO_NUM_14
#define ENCODER_B_PIN_3 GPIO_NUM_12

// Define the GPIO pins for the input multipliers
#define MULTIPLIER_PIN_1 GPIO_NUM_4
#define MULTIPLIER_PIN_2 GPIO_NUM_5
#define MULTIPLIER_PIN_3 GPIO_NUM_16

// Define the PWM properties
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_1      LEDC_CHANNEL_0
#define LEDC_CHANNEL_2      LEDC_CHANNEL_1
#define LEDC_CHANNEL_3      LEDC_CHANNEL_2
#define LEDC_DUTY_RES       LEDC_TIMER_10_BIT
#define LEDC_FREQUENCY      5000 // Frequency in Hertz. Set frequency at 5 kHz

void init_input_pins() {
    gpio_pad_select_gpio(MULTIPLIER_PIN_1);
    gpio_pad_select_gpio(MULTIPLIER_PIN_2);
    gpio_pad_select_gpio(MULTIPLIER_PIN_3);
    gpio_set_direction(MULTIPLIER_PIN_1, GPIO_MODE_INPUT);
    gpio_set_direction(MULTIPLIER_PIN_2, GPIO_MODE_INPUT);
    gpio_set_direction(MULTIPLIER_PIN_3, GPIO_MODE_INPUT);
}

int get_multiplier() {
    int multiplier = 10; // Default multiplier
    if (gpio_get_level(MULTIPLIER_PIN_1) == 1) {
        multiplier = 10;
    } else if (gpio_get_level(MULTIPLIER_PIN_2) == 1) {
        multiplier = 100;
    } else if (gpio_get_level(MULTIPLIER_PIN_3) == 1) {
        multiplier = 1000;
    }
    return multiplier;
}

void app_main(void) {
    // Configure the GPIO pins for the stepper motors
    gpio_pad_select_gpio(STEP_PIN_1);
    gpio_pad_select_gpio(DIR_PIN_1);
    gpio_set_direction(STEP_PIN_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIR_PIN_1, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(STEP_PIN_2);
    gpio_pad_select_gpio(DIR_PIN_2);
    gpio_set_direction(STEP_PIN_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIR_PIN_2, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(STEP_PIN_3);
    gpio_pad_select_gpio(DIR_PIN_3);
    gpio_set_direction(STEP_PIN_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIR_PIN_3, GPIO_MODE_OUTPUT);

    // Configure the PWM timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configure the PWM channels
    ledc_channel_config_t ledc_channel_1 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = STEP_PIN_1,
        .duty           = 512, // 50% duty cycle
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_1);

    ledc_channel_config_t ledc_channel_2 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_2,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = STEP_PIN_2,
        .duty           = 512, // 50% duty cycle
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_2);

    ledc_channel_config_t ledc_channel_3 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_3,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = STEP_PIN_3,
        .duty           = 512, // 50% duty cycle
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_3);

    // Initialize the encoders
    pcnt_unit_config_t unit_config = {
        .high_limit = 10000,
        .low_limit = -10000,
    };

    pcnt_unit_handle_t pcnt_unit_1;
    pcnt_unit_handle_t pcnt_unit_2;
    pcnt_unit_handle_t pcnt_unit_3;

    pcnt_new_unit(&unit_config, &pcnt_unit_1);
    pcnt_new_unit(&unit_config, &pcnt_unit_2);
    pcnt_new_unit(&unit_config, &pcnt_unit_3);

    pcnt_chan_config_t chan_config = {
        .edge_gpio_num = ENCODER_A_PIN_1,
        .level_gpio_num = ENCODER_B_PIN_1,
        .flags = {
            .invert_edge_input = false,
            .invert_level_input = false,
        },
    };

    pcnt_channel_handle_t pcnt_chan_1;
    pcnt_channel_handle_t pcnt_chan_2;
    pcnt_channel_handle_t pcnt_chan_3;

    pcnt_new_channel(pcnt_unit_1, &chan_config, &pcnt_chan_1);
    chan_config.edge_gpio_num = ENCODER_A_PIN_2;
    chan_config.level_gpio_num = ENCODER_B_PIN_2;
    pcnt_new_channel(pcnt_unit_2, &chan_config, &pcnt_chan_2);
    chan_config.edge_gpio_num = ENCODER_A_PIN_3;
    chan_config.level_gpio_num = ENCODER_B_PIN_3;
    pcnt_new_channel(pcnt_unit_3, &chan_config, &pcnt_chan_3);

    // Initialize the input pins for multipliers
    init_input_pins();

    int16_t last_count_1 = 0;
    int16_t last_count_2 = 0;
    int16_t last_count_3 = 0;

    while (1) {
        // Read encoder values
        int16_t count_1, count_2, count_3;
        pcnt_unit_get_count(pcnt_unit_1, &count_1);
        pcnt_unit_get_count(pcnt_unit_2, &count_2);
        pcnt_unit_get_count(pcnt_unit_3, &count_3);

        // Calculate the speeds based on the change in encoder counts
        int16_t speed_1 = count_1 - last_count_1;
        int16_t speed_2 = count_2 - last_count_2;
        int16_t speed_3 = count_3 - last_count_3;
        last_count_1 = count_1;
        last_count_2 = count_2;
        last_count_3 = count_3;

        // Get the multiplier from the input pins
        int multiplier = get_multiplier();

        // Set direction based on the sign of the speeds
        if (speed_1 > 0) {
            gpio_set_level(DIR_PIN_1, 1);
        } else {
            gpio_set_level(DIR_PIN_1, 0);
            speed_1 = -speed_1; // Make speed positive
        }

        if (speed_2 > 0) {
            gpio_set_level(DIR_PIN_2, 1);
        } else {
            gpio_set_level(DIR_PIN_2, 0);
            speed_2 = -speed_2; // Make speed positive
        }

        if (speed_3 > 0) {
            gpio_set_level(DIR_PIN_3, 1);
        } else {
            gpio_set_level(DIR_PIN_3, 0);
            speed_3 = -speed_3; // Make speed positive
        }

        // Set PWM duty cycles based on the speeds and multiplier
        uint32_t duty_1 = speed_1 * multiplier;
        uint32_t duty_2 = speed_2 * multiplier;
        uint32_t duty_3 = speed_3 * multiplier;

        if (duty_1 > 1023) {
            duty_1 = 1023; // Limit the duty cycle to the maximum value
        }
        if (duty_2 > 1023) {
            duty_2 = 1023; // Limit the duty cycle to the maximum value
        }
        if (duty_3 > 1023) {
            duty_3 = 1023; // Limit the duty cycle to the maximum value
        }
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, duty_1);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, duty_2);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_3, duty_3);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_3);

        vTaskDelay(100 / portTICK_PERIOD_MS); // Adjust the delay as needed
    }
}