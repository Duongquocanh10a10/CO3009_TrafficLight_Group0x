#include "button.h"

static GPIO_TypeDef* const BUTTON_PORT[NUM_BUTTONS] = {
    BTN_1_GPIO_Port,
    BTN_2_GPIO_Port,
    BTN_3_GPIO_Port,
    BTN_4_GPIO_Port,
    BTN_USER_GPIO_Port
};

static const uint16_t BUTTON_PIN[NUM_BUTTONS] = {
    BTN_1_Pin,
    BTN_2_Pin,
    BTN_3_Pin,
    BTN_4_Pin,
    BTN_USER_Pin
};

static GPIO_PinState KeyReg0[NUM_BUTTONS] = {
    NORMAL_STATE, NORMAL_STATE, NORMAL_STATE, NORMAL_STATE, NORMAL_STATE
};
static GPIO_PinState KeyReg1[NUM_BUTTONS] = {
    NORMAL_STATE, NORMAL_STATE, NORMAL_STATE, NORMAL_STATE, NORMAL_STATE
};
static GPIO_PinState KeyReg2[NUM_BUTTONS] = {
    NORMAL_STATE, NORMAL_STATE, NORMAL_STATE, NORMAL_STATE, NORMAL_STATE
};
static GPIO_PinState KeyReg3[NUM_BUTTONS] = {
    NORMAL_STATE, NORMAL_STATE, NORMAL_STATE, NORMAL_STATE, NORMAL_STATE
};

static uint16_t TimeOutForKeyPress[NUM_BUTTONS] = {
    LONG_PRESS_INITIAL_TICKS,
    LONG_PRESS_INITIAL_TICKS,
    LONG_PRESS_INITIAL_TICKS,
    LONG_PRESS_INITIAL_TICKS,
    LONG_PRESS_INITIAL_TICKS
};

static uint8_t button_flag[NUM_BUTTONS]      = {0};
static uint8_t button_long_flag[NUM_BUTTONS] = {0};

void button_reading(void)
{
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        KeyReg2[i] = KeyReg1[i];
        KeyReg1[i] = KeyReg0[i];
        KeyReg0[i] = HAL_GPIO_ReadPin(BUTTON_PORT[i], BUTTON_PIN[i]);

        if ((KeyReg0[i] == KeyReg1[i]) && (KeyReg1[i] == KeyReg2[i]))
        {
            if (KeyReg3[i] != KeyReg2[i])
            {
                KeyReg3[i] = KeyReg2[i];

                if (KeyReg2[i] == PRESSED_STATE)
                {
                    button_flag[i]        = 1;
                    TimeOutForKeyPress[i] = LONG_PRESS_INITIAL_TICKS;
                }
            }
            else
            {
                if (KeyReg2[i] == PRESSED_STATE)
                {
                    if (TimeOutForKeyPress[i] > 0)
                    {
                        TimeOutForKeyPress[i]--;
                        if (TimeOutForKeyPress[i] == 0)
                        {
                            button_flag[i]        = 1;
                            button_long_flag[i]   = 1;
                            TimeOutForKeyPress[i] = LONG_PRESS_REPEAT_TICKS;
                        }
                    }
                }
            }
        }
    }
}

uint8_t isButtonPressed(uint8_t index)
{
    if (index >= NUM_BUTTONS)
    {
        return 0;
    }

    if (button_flag[index] == 1)
    {
        button_flag[index] = 0;
        return 1;
    }

    return 0;
}

uint8_t isButtonLongPressed(uint8_t index)
{
    if (index >= NUM_BUTTONS)
    {
        return 0;
    }

    if (button_long_flag[index] == 1)
    {
        button_long_flag[index] = 0;
        return 1;
    }

    return 0;
}

void clearAllButtons(void)
{
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        button_flag[i]        = 0;
        button_long_flag[i]   = 0;
        TimeOutForKeyPress[i] = LONG_PRESS_INITIAL_TICKS;
    }
}
