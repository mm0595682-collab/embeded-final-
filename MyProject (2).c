//====================================================================
// Project 2: Digital Combination Lock with Keypad + Buzzer
// MCU  : STM32F103C6
// Port : Keypad rows/cols + LED + Motor + Buzzer on GPIOB
// Passcode: 1 2 3 4
//====================================================================

char code_1 = '1';
char code_2 = '2';
char code_3 = '3';
char code_4 = '4';

char input_1 = 0;
char input_2 = 0;
char input_3 = 0;
char input_4 = 0;

unsigned char position = 0;
unsigned char fail_count = 0;

// Helper: set only rows (PB0..PB3) and keep PB7..PB9 state
void Set_Rows(unsigned char row_mask) {
    // row_mask uses bits 0..3 only
    // Clear PB0..PB3
    GPIOB_ODR &= ~(0x000F);
    // Set them according to mask
    GPIOB_ODR |= (row_mask & 0x0F);
}

void Init_All() {
    // PB0..PB3 rows, PB7 LED, PB8 motor, PB9 buzzer as outputs
    GPIO_Digital_Output(&GPIOB_BASE,
                        _GPIO_PINMASK_0 |
                        _GPIO_PINMASK_1 |
                        _GPIO_PINMASK_2 |
                        _GPIO_PINMASK_3 |
                        _GPIO_PINMASK_7 |
                        _GPIO_PINMASK_8 |
                        _GPIO_PINMASK_9);

    // PB4..PB6 columns as inputs
    GPIO_Digital_Input(&GPIOB_BASE,
                       _GPIO_PINMASK_4 |
                       _GPIO_PINMASK_5 |
                       _GPIO_PINMASK_6);

    // Internal pull-ups for PB4..PB6 (STM32F1 style: set ODR bits high)
    GPIOB_ODR |= (_GPIO_PINMASK_4 |
                  _GPIO_PINMASK_5 |
                  _GPIO_PINMASK_6);

    // Make sure outputs are initially low (LED, motor, buzzer off, rows low)
    GPIOB_ODR &= ~(_GPIO_PINMASK_0 |
                   _GPIO_PINMASK_1 |
                   _GPIO_PINMASK_2 |
                   _GPIO_PINMASK_3 |
                   _GPIO_PINMASK_7 |
                   _GPIO_PINMASK_8 |
                   _GPIO_PINMASK_9);

    Delay_ms(100);
}

char Keypad_Scan() {
    char key = 0;

    // Row 0 active (PB0 = 1)
    Set_Rows(0x01);
    Delay_us(30);
    if(GPIOB_IDR.B4 == 0) key = '1';
    else if(GPIOB_IDR.B5 == 0) key = '2';
    else if(GPIOB_IDR.B6 == 0) key = '3';

    // Row 1 active (PB1 = 1)
    if(key == 0) {
        Set_Rows(0x02);
        Delay_us(30);
        if(GPIOB_IDR.B4 == 0) key = '4';
        else if(GPIOB_IDR.B5 == 0) key = '5';
        else if(GPIOB_IDR.B6 == 0) key = '6';
    }

    // Row 2 active (PB2 = 1)
    if(key == 0) {
        Set_Rows(0x04);
        Delay_us(30);
        if(GPIOB_IDR.B4 == 0) key = '7';
        else if(GPIOB_IDR.B5 == 0) key = '8';
        else if(GPIOB_IDR.B6 == 0) key = '9';
    }

    // Row 3 active (PB3 = 1)
    if(key == 0) {
        Set_Rows(0x08);
        Delay_us(30);
        if(GPIOB_IDR.B4 == 0) key = '*';
        else if(GPIOB_IDR.B5 == 0) key = '0';
        else if(GPIOB_IDR.B6 == 0) key = '#';
    }

    // All rows off
    Set_Rows(0x00);

    return key;
}

void Open_Lock() {
    // Motor ON + LED ON (5 seconds)
    GPIOB_ODR.B8 = 1;   // Motor
    GPIOB_ODR.B7 = 1;   // LED
    Delay_ms(5000);
    GPIOB_ODR.B8 = 0;
    GPIOB_ODR.B7 = 0;
}

void Sound_Alarm() {
    GPIOB_ODR.B9 = 1;
    Delay_ms(3000);
    GPIOB_ODR.B9 = 0;
}

char Check_Passcode() {
    if(input_1 == code_1 &&
       input_2 == code_2 &&
       input_3 == code_3 &&
       input_4 == code_4) {
        return 1;
    }
    return 0;
}

void Clear_All() {
    input_1 = 0;
    input_2 = 0;
    input_3 = 0;
    input_4 = 0;
    position = 0;
}

void main() {
    Init_All();

    while(1) {
        char key = Keypad_Scan();

        // Store digits only when 0–9
        if(key >= '0' && key <= '9') {
            if(position == 0) input_1 = key;
            else if(position == 1) input_2 = key;
            else if(position == 2) input_3 = key;
            else if(position == 3) input_4 = key;

            if(position < 4) position++;
            Delay_ms(250);   // debounce
        }

        // # = check the 4-digit code
        if(key == '#') {
            if(position == 4) {     // only check if 4 digits entered
                if(Check_Passcode()) {
                    Open_Lock();
                    fail_count = 0;
                } else {
                    fail_count++;
                    if(fail_count >= 3) {
                        Sound_Alarm();
                        fail_count = 0;
                    }
                }
            }
            Clear_All();
        }

        // * = clear input any time
        if(key == '*') {
            Clear_All();
        }

        Delay_ms(30);
    }
}

