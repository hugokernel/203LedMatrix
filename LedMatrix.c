
#include "LedMatrix.h"

inline void setLed(unsigned int x, unsigned int y) {

    if (x >= MATRIX_COL_COUNT || y >= MATRIX_LINE_COUNT) {
        return;
    }

    // Reset all ports
    DDRB = DDRC = DDRD = 0;
    PORTB = PORTC = PORTD = 0;

    DDRB = pgm_read_byte(&(matrix[x][y][_DDRB]));
    DDRC = pgm_read_byte(&(matrix[x][y][_DDRC]));
    DDRD = pgm_read_byte(&(matrix[x][y][_DDRD]));

    //DDRD = (PORTD & 0b11100000) ^ pgm_read_byte(&(matrix[x][y][_DDRD]));

    PORTB = pgm_read_byte(&(matrix[x][y][_PORTB]));
    PORTC = pgm_read_byte(&(matrix[x][y][_PORTC]));
    PORTD = pgm_read_byte(&(matrix[x][y][_PORTD]));

    //PORTD = (PORTD & 0b11100000) ^ pgm_read_byte(&(matrix[x][y][_PORTD]));
}

inline void clearLeds() {
    // Reset all ports
    DDRB = DDRC = DDRD = 0;
    PORTB = PORTC = PORTD = 0;
}

/*
void writeChar(int col, unsigned char c) {

    char temp = 0;
    char b = 0;
    
    for (char i = 0; i < 5; i++) {
        temp = pgm_read_byte(Font5x7 + (c - 0x20) * 5 + i);
        
        for (b = 0; b < 7; b++) {
            if (temp & (1 << b)) {
                setLed(col + i, b);
            } else {
                clearLeds();
            }
        }
    }
}
*/

/*
void writeChar(int col, unsigned char c, unsigned char x) {

    char temp = 0;
    char b = 0;
    
    for (char i = 0; i < 5 - x; i++) {
        temp = pgm_read_byte(Font5x7 + (c - 0x20) * 5 + i + x);
        
        for (b = 0; b < 7; b++) {
            if (temp & (1 << b)) {
                setLed(col + i, b);
            } else {
                clearLeds();
            }
        }
    }
}
*/

/*
unsigned char getChar(char * str, unsigned char num) {
    unsigned int size = strlen(str);
    char c = str[num % size];
    return c;
}
*/

/*
void write(char * str) {

    if (strlen(str) > MATRIX_COL_COUNT / 6) {
//        writeScrollingString(str);
    } else {
        writeString(str);
    }

}

void writeString(char * str) {
    char index, col = 0;
    unsigned char x = 0;

    for (x = 0, col = 0; x < MATRIX_COL_COUNT; x = x + 6, col++) {
        if (str[col + index]) {
            writeChar(x, str[col + index], 0); 
        } else {
            writeChar(x, ' ', 0);
        }
    }
}
*/

void writeLine(char * str, int offset) {
    char col, tmp = 0;
    int x = 0;
    char ii = 0;
    unsigned int size = strlen(str);
    char i = 0;
    unsigned int start_2 = 0;
    int start = 0;

    unsigned int offset_abs = abs(offset);


    if (offset < 0) {
        start_2 = offset_abs % 5;
        x = 0;
        start = offset_abs / 5;
    } else {
        start_2 = 0;
        x = offset;
        start = 0;
    }

   offset = offset_abs % 5;

//    unsigned char space = 0;//offset_abs % 6;
    //if (space == 5)
    //    x++;

    //
    char temp = 0;
    char b = 0;

    for (col = start; x < MATRIX_COL_COUNT; col++) {

        tmp = (col < size) ? str[col] : ' ';

        for (i = start_2, ii = 0; i < 5; i++, ii++) {
            temp = pgm_read_byte(Font5x7 + (tmp - 0x20) * 5 + i);
    
            for (b = 0; b < 7; b++) {
                if (temp & (1 << b)) {
                    setLed(x + ii, b);
                } else {
                    clearLeds();
                }
            }

            start_2 = 0;
        }
        
        // Ecart entre les lettres
        x = x + ii + char_spacing;
    }

    clearLeds();
}

void clearScreen() {
    printf("\033[2J");      // clear screen
    printf("\033[0;0H");    // set cursor to 0,0
    printf("\033[10B");     // move cursor down 10 lines
    printf("\033[5A");      // move cursor up 5 lines
}

void writeMenu() {
    clearScreen();
    printf("Welcome !\n\r\n\r");
    printf("Action :\n\r");
    printf("- [t] Set text...\n\r");
    printf("- [s] Set scroll speed...\n\r");
    printf("- [c] Clear\n\r");
    printf("- [d] Reverse direction\n\r");
    printf("- [l] Letter spacing...\n\r");
    printf("- [w] Write configuration to EEPROM\n\r");
    printf("- [b] Read button value...\n\r");
}

void handleAction() {

    int _index = 0;
    char action[64];

    receivedBuffer = 0;
    while (1) {

        if (!receivedBuffer) {
            continue;
        }

        USART_SendByte(receivedBuffer);

        memset(action, 0, sizeof(action));
        switch (receivedBuffer) {
            case 0:
                break;
            case 't':
                clearScreen();
                printf("Type your message and press [ENTER] to exit : \n\r");
                receivedBuffer = 0;
                memset(message, 0, sizeof(message));
                _index = 0;

                // Reset index
                index = 0;

                while (1) {
                    if (!receivedBuffer) {
                        continue;
                    }

                    USART_SendByte(receivedBuffer);

                    // Enter pressed ?
                    if (receivedBuffer == 13) {
                        break;
                    }

                    // Return ?
                    if (receivedBuffer == 8) {
                        message[--_index] = 0;
                        receivedBuffer = 0;
                        continue;
                    }

                    message[_index++] = receivedBuffer;
                    receivedBuffer = 0;

                    // Test max string size
                    if (_index > sizeof(message)) {
                        strcat(action, "Message size reached !\n\r");
                        break;
                    }
                }

                strcat(action, "Message saved in RAM !\n\r");
                break;
            case 's':
                while (1) {
                    clearScreen();
                    printf("Enter a speed between 0 to 9, [ENTER] to exit : \n\r");
                    receivedBuffer = 0;

                    while (1) {
                        if (receivedBuffer) {
                            break;
                        }
                    }

                    // Set Speed
                    if (receivedBuffer >= '0' && receivedBuffer <= '9') {
                        scroll_speed = speed_table[receivedBuffer - '0'];
                    }
                    
                    // Enter pressed ?
                    if (receivedBuffer == 13) {
                        break;
                    }
                }

                strcat(action, "Speed saved !\n\r");
                break;
            case 'c':
                strcat(action, "Message cleared !\n\r");
                memset(message, 0, sizeof(message));
                break;
            case 'l':
                while (1) {
                    clearScreen();
                    printf("Enter character spacing (0 to 9), [ENTER] to exit : \n\r");
                    receivedBuffer = 0;

                    while (1) {
                        if (receivedBuffer) {
                            break;
                        }
                    }

                    if (receivedBuffer >= '0' && receivedBuffer <= '9') {
                        break;
                    }
                }

                char_spacing = receivedBuffer - '0';

                strcat(action, "Character spacing saved !\n\r");
                break;
            case 'd':
                strcat(action, "Direction switched !\n\r");
                scroll_direction = !scroll_direction;
                break;
            case 'w':
                eeprom_write_block((void*)&message, (const void*)EEPROM_CONFIG_ADDR_MESSAGE, sizeof(message));
                eeprom_update_word((const void*)EEPROM_CONFIG_ADDR_SPEED, scroll_speed);
                eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_DIRECTION, scroll_direction);
                eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_SPACING, char_spacing);
                
                strcat(action, "Configuration saved in EEPROM !\n\r");
                break;
            case 'b':
                receivedBuffer = 0;

                clearScreen();
                printf("Reading button value !\n\r");
                ADCSRA |= (1 << ADSC) | (1 << ADIE);
                uint16_t tmp;

                char key = 0;
                unsigned char value = 0;

                while (1) {
                    key = 0;

                    if (receivedBuffer) {
                        break;
                    }

                    ADCSRA |= (1 << ADSC) | (1 << ADIE);
                    value = adc_value;

                    if (value != tmp) {
                    
                        if (value > 126 && value < 130) {           // 128
                            key = 'A';
                        } else if (value > 152 && value < 157) {    // 155
                            key = 'B';
                        } else if (value > 76 && value < 82) {      // 79
                            key = 'C';
                        } else if (value > 54 && value < 59) {      // 57
                            key = 'D';
                        } else if (value > 17 && value < 23) {      // 19
                            key = 'E';
                        } else if (value > 40 && value < 47) {      // 42
                            key = 'F';
                        } else if (value > 189 && value < 195) {    // 192
                            key = 'G';
                        } else {
                            key = 'x';
                            continue;
                        }

                        printf("<%c", key);

                        while (1) {
                            ADCSRA |= (1 << ADSC) | (1 << ADIE);
                            _delay_us(10);
                            if (adc_value > 250) {
                                printf(">");
                                break;
                            }
                        }
                    }

                    tmp = value;
                }
                break;
        }

        writeMenu();
        if (strlen(action)) {
            printf("\n\r");
            printf(action);
        }
        printf("\n\rType your choice : ");
        receivedBuffer = 0;
    }
}



ISR(TIMER0_OVF_vect)
{
    writeLine(message, index);
}

ISR(TIMER1_OVF_vect)
{
    int size = strlen(message);
    char tab[] = { 5, 5, 4, 4, 3, 3, 3, 3, 2, 2 };

    if (size > tab[char_spacing]) {
        size *= (5 + char_spacing);
        if (scroll_direction) {
            index = index - STEP;
            if (index < -size) {
                index = MATRIX_COL_COUNT;
            }
        } else {
            index = index + STEP;
            if (index > MATRIX_COL_COUNT) {
                index = 0 - size;
            }
        }
    } else {
        index = 0;
    }

    TCNT1H = HIGH(scroll_speed);
    TCNT1L = LOW(scroll_speed);
}

ISR(ADC_vect)
{
	//adc_value = ADCL;
	//adc_value += (ADCH << 8);
    adc_value = ADCH;

/*
    static uint64_t avg = 0;
    static uint8_t count = 0;
    uint16_t tmp = 0;

	tmp = ADCL;// & 0b11111000;
	tmp += (ADCH << 8);

    count++;
    avg += tmp;
    if (count == 4) {
        adc_value = avg / 4;
        count = 0;
        avg = 0;
    }
    */
}

int main(void) {

	clearLeds();

    // Timer 0 configuration
    TIMSK0 |= (1<<TOIE0);
    TCCR0A = 0;
    TCCR0B |= ((0<<CS02) | (1<<CS01) | (1<<CS00));
    TCNT0 = 0;

    // Timer 1 configuration
    TIMSK1 |= (1<<TOIE1);
    TCCR1A = 0;
    TCCR1B |= ((0<<CS12) | (1<<CS11) | (1<<CS10));
    TCCR1C  = 0;
    TCNT1 = 0;

    DDRD = 2;
    PORTD = 0;

    // Init ADC
    //ADMUX |= (0 << REFS1) | (0 << REFS0) | (1 << MUX2) | (1 << MUX1);
    ADMUX = 0b01100110;

    //set prescaller and enable ADC
    ADCSRA |= (1 << ADEN) | (1 << ADIE);

    USART_Init();

    stdout = &mystdout;

    writeMenu();
    printf("\n\rType your choice : ");

    memset(message, 0, sizeof(message));

    eeprom_read_block((void*)&message, (const void*)EEPROM_CONFIG_ADDR_MESSAGE, sizeof(message));
    scroll_speed = eeprom_read_word((const void*)EEPROM_CONFIG_ADDR_SPEED);
    scroll_direction = eeprom_read_byte((const void*)EEPROM_CONFIG_ADDR_DIRECTION);
    char_spacing = eeprom_read_byte((const void*)EEPROM_CONFIG_ADDR_SPACING);

    sei();

    handleAction();
}

