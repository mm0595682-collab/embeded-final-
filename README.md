// ================= LCD Pin Configuration =================
sbit LCD_RS at GPIOA_ODR.B0;
sbit LCD_EN at GPIOA_ODR.B1;
sbit LCD_D4 at GPIOA_ODR.B2;
sbit LCD_D5 at GPIOA_ODR.B3;
sbit LCD_D6 at GPIOA_ODR.B4;
sbit LCD_D7 at GPIOA_ODR.B5;

sbit LCD_RS_Direction at GPIOA_CRL.B0;
sbit LCD_EN_Direction at GPIOA_CRL.B1;
sbit LCD_D4_Direction at GPIOA_CRL.B2;
sbit LCD_D5_Direction at GPIOA_CRL.B3;
sbit LCD_D6_Direction at GPIOA_CRL.B4;
sbit LCD_D7_Direction at GPIOA_CRL.B5;

// ================= Global Variables =================
char cmd[10];            // buffer for UART command
unsigned char count = 0; // command character count
char c;                  // incoming UART character

unsigned short light = 0; // Light status
unsigned short fan   = 0; // Fan status
unsigned short ac    = 0; // AC status

// ================= LCD Update Function =================
void Update_LCD() {
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Out(1,1,"L:F:A:");
    Lcd_Chr(1,3, light ? '1' : '0');
    Lcd_Chr(1,5, fan   ? '1' : '0');
    Lcd_Chr(1,7, ac    ? '1' : '0');
}

// ================= MAIN FUNCTION =================
void main() {

    // Initialize GPIO for LEDs (simulate devices)
    // PB0 = Light, PB1 = Fan, PB2 = AC
    GPIO_Digital_Output(&GPIOB_BASE,
        _GPIO_PINMASK_0 | _GPIO_PINMASK_1 | _GPIO_PINMASK_2);

    // Initialize UART1 (PA9 TX, PA10 RX)
    UART1_Init(19200);
    Delay_ms(50);
    UART1_Write_Text("Smart Room Ready\r\n");

    // Initialize LCD
    Lcd_Init();
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Update_LCD();

    // ================= MAIN LOOP =================
    while(1) {

        if (UART1_Data_Ready()) {
            c = UART1_Read();

            // Accept only uppercase letters for command
            if ((c >= 'A' && c <= 'Z')) {
                UART1_Write(c);   // echo typed character

                if (count < 9) {
                    cmd[count] = c;
                    count++;
                }
            }

            // Command end (ENTER key)
            if (c == '\r') {
                cmd[count] = 0;   // null-terminate command
                UART1_Write_Text("\r\n");

                // ===== Command Processing =====
                if (strcmp(cmd, "LIGHTON") == 0) {
                    light = 1;
                    GPIOB_ODR.B0 = 1;
                    UART1_Write_Text("Light ON\r\n");
                }
                else if (strcmp(cmd, "LIGHTOFF") == 0) {
                    light = 0;
                    GPIOB_ODR.B0 = 0;
                    UART1_Write_Text("Light OFF\r\n");
                }
                else if (strcmp(cmd, "FANON") == 0) {
                    fan = 1;
                    GPIOB_ODR.B1 = 1;
                    UART1_Write_Text("Fan ON\r\n");
                }
                else if (strcmp(cmd, "FANOFF") == 0) {
                    fan = 0;
                    GPIOB_ODR.B1 = 0;
                    UART1_Write_Text("Fan OFF\r\n");
                }
                else if (strcmp(cmd, "ACON") == 0) {
                    ac = 1;
                    GPIOB_ODR.B2 = 1;
                    UART1_Write_Text("AC ON\r\n");
                }
                else if (strcmp(cmd, "ACOFF") == 0) {
                    ac = 0;
                    GPIOB_ODR.B2 = 0;
                    UART1_Write_Text("AC OFF\r\n");
                }
                else {
                    UART1_Write_Text("ERROR\r\n");
                    Lcd_Cmd(_LCD_CLEAR);
                    Lcd_Out(1,1,"Error");
                    Delay_ms(1000);
                }

                // Update LCD after processing
                Update_LCD();

                // Reset buffer for next command
                count = 0;
                UART1_Write_Text("Ready\r\n");
            }
        }

        Delay_ms(10); // small delay to avoid CPU hog
    }
}
