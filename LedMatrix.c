
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
    //DDRD = pgm_read_byte(&(matrix[x][y][_DDRD]));

    DDRD = 0b11100000 & pgm_read_byte(&(matrix[x][y][_DDRD]));

    PORTB = pgm_read_byte(&(matrix[x][y][_PORTB]));
    PORTC = pgm_read_byte(&(matrix[x][y][_PORTC]));
    //PORTD = pgm_read_byte(&(matrix[x][y][_PORTD]));

    PORTD = 0b11100000 & pgm_read_byte(&(matrix[x][y][_PORTD]));
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

inline void writeLine(char * str, int offset) {
    char col, tmp = 0;
    int x = 0;
    unsigned int start_2 = 0;
    int start = 0;
    char ii = 0;
    char i = 0;
    unsigned char b = 0;

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

    //offset = offset_abs % 5;
    //unsigned char space = 0;//offset_abs % 6;
    //if (space == 5)
    //    x++;
    

    for (col = start; x < MATRIX_COL_COUNT; col++) {

        // strlen(str)
        tmp = (col < message_size) ? str[col] : ' ';

        for (i = start_2, ii = 0; i < 5; i++, ii++) {    
            for (b = 0; b < 7; b++) {
                if (pgm_read_byte(Font5x7 + (tmp - 0x20) * 5 + i) & (1 << b)) {
                    setLed(x + ii, b);
                    asm("nop");
                    asm("nop");
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

#if INTERACTIVE_MODE
void clearScreen() {
    printf("\033[2J");      // clear screen
    printf("\033[0;0H");    // set cursor to 0,0
    //printf("\033[10B");     // move cursor down 10 lines
    //printf("\033[5A");      // move cursor up 5 lines
}

void writeMenu() {
    clearScreen();
    
    printf("M:[%i], S:[%i], D:[%i], L:[%i], C:[%i]\n\r", current_message, scroll_speed, scroll_direction, char_spacing, auto_chain_message);
    
    printf("Action :\n\r");
    printf(" [0...3] Select msg\n\r");
    printf(" [t] Edit msg\n\r");
    printf(" [c] Clear\n\r");

    printf(" [s] Set scroll speed...\n\r");
    printf(" [l] Letter spacing...\n\r");

    printf(" [d] Reverse direction\n\r");
    printf(" [a] Chain mode\n\r");
    printf(" [w] Save conf to EEPROM\n\r");
    printf(" [b] Read button...\n\r");
}

void handleAction() {

    int _index = 0;
    unsigned char auto_chain = 0;
    char action[64];

    receivedBuffer = 1;
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

                if (auto_chain_message) {
                    auto_chain = auto_chain_message;
                    auto_chain_message = 0;
                }

                message_size = 0;
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
                        message[--message_size] = 0;
                        receivedBuffer = 0;
                        continue;
                    }

                    message[message_size++] = receivedBuffer;
                    receivedBuffer = 0;

                    // Test max string size
                    if (message_size == sizeof(message) - 1) {
                        strcat(action, "Message size reached !\n\r");
                        break;
                    }
                }

                if (auto_chain_message) {
                    saveMessage(current_message);
                }

                auto_chain_message = auto_chain;

                strcat(action, "Message saved !\n\r");
                break;
            case '0':
            case '1':
            case '2':
            case '3':
                saveMessage(current_message);
                current_message = receivedBuffer - '0';
                loadMessage(current_message);
                index = 0;
                strcat(action, "Message switched !\n\r");
                break;
            case 'a':
                auto_chain_message = !auto_chain_message;
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
                        //scroll_speed = speed_table[receivedBuffer - '0'];
                        scroll_speed = receivedBuffer - '0';
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
                saveMessage(current_message);
                //eeprom_write_block((void*)&message, (const void*)EEPROM_CONFIG_ADDR_MESSAGE, sizeof(message));
                eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_SPEED, scroll_speed);
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
        printf("\n\rYour choice : ");
        receivedBuffer = 0;
    }
}
#else

#define SEND_ACK    if (verbose) {          \
                        printf("[ACK]");    \
                    } else {                \
                        printf(".");        \
                    }                       \

#define RESPONSE(c) USART_SendByte(c);

void debug(char * str, ...) {
    if (verbose) {
        va_list arg;
        va_start(arg, str);
        vprintf(str, arg);
        va_end(arg);
    }
}

inline char waitData() {
    char value = 0;
    while (!receivedBuffer);
    value = receivedBuffer;
    receivedBuffer = 0;
    return value;
}

void handleAction() {

    char command = 0;
    char data = 0;
    int size = 0;

    char key, key_tmp = 0;
    char value, in = 0;

    SEND_ACK

    receivedBuffer = 0;
    while (1) {

        command = 0;
        debug("Wait cmd...");

        //command = waitData();

        // Wait for command
        while (1) {

            if (receivedBuffer) {
                command = receivedBuffer;
                receivedBuffer = 0;
                break;
            }

            ADCSRA |= (1 << ADSC) | (1 << ADIE);
            value = adc_value;        
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
                key = '?';
                continue;
            }

            // Todo: Faire une boucle d'attente de message sur le port serie...??
            _delay_ms(100);

            if (key != key_tmp) {
                printf("%c", key);
                key_tmp = key;
            }

            ADCSRA |= (1 << ADSC) | (1 << ADIE);
            if (adc_value > 250) {
                printf("%c", tolower(key_tmp));
                key_tmp = 0;
            }
        }

        if (isspace(command)) {
            debug("\n\r");
            continue;
        }

        // Response
        _delay_ms(500);
        RESPONSE(command);

        switch (command) {
            case 'm':
                data = 0;
#if SYNCHRONIZE
                const char tab[] = { 5, 5, 4, 4, 3, 3, 3, 3, 2, 2 };

                // Wait for first char
                size = message_size * (5 + char_spacing);
                while (1) {
                    if ((index - STEP) == -size) {
                        _delay_ms(100);
                        SEND_ACK
                    }

                    if (receivedBuffer) {
                        data = receivedBuffer;
                        receivedBuffer = 0;
                        break;
                    }
                }
#endif

                debug(", set msg :\n\r");

                message_size = 0;
                memset(message, 0, sizeof(message));
                while (1) {
                    // Return ?
                    if (data == 8) {
                        message[--message_size] = 0;
                        continue;
                    }

                    // Enter pressed ?
                    if (data == 13) {
                        break;
                    }

                    message[message_size++] = data;

                    // Test max string size
                    if (message_size == sizeof(message) - 1) {
                        break;
                    }

                    data = waitData();
                }

                _delay_ms(500);

                // Send message size
                printf("%03i", message_size);
                goto end;
            case '?':
                printf("\n\rwww.digitalspirit.org - hugo\n\r");
                printf("AVR Libc %s, %s, %s\n\r", __AVR_LIBC_VERSION_STRING__, __DATE__, __TIME__);
                printf("m(msg), s(speed), d(direction), l(spacing), e(msg > eeprom), p(print), c(conf), w(watchdog), v(verbose)");
                SEND_ACK
                goto end;
            case 'p':
                printf("%s", message);
                SEND_ACK
                goto end;
            case 'c':
                if (verbose) {
                    printf("\n\rMsg size: %i (max: %u), Speed: %i, Direction: %i, Spacing: %i, Intensity: %i, Watchdog: %i",
                        message_size, sizeof(message), scroll_speed, scroll_direction, char_spacing, intensity, watchdog_value);
                } else {
                    printf("%i|%u|%i|%i|%i|%i|%i",
                        message_size, sizeof(message), scroll_speed, scroll_direction, char_spacing, intensity, watchdog_value);
                }
                SEND_ACK
                goto end;
            case 'e':
                eeprom_write_block((void*)&message, EEPROM_CONFIG_ADDR_MESSAGE, message_size + 1);   // strlen + 0x00
                SEND_ACK
                goto end;
            // Command with argument
            case 'v':
            case 's':
            case 'd':
            case 'l':
            case 'i':
            case 'w':
                break;
            default:
                debug(", unknow cmd %c !\n\r", command);
                continue;
        }

        // Get arg
        debug(", wait arg...");
        data = waitData();

        debug(", wait validation...");
        while (1) {
            in = waitData();

            // Wait Return
            if (in == 13) {
                break;
            }

            // Cancel ?
            if (in == 27) {
                debug("Cancelled !");
                goto end;
            }
        }

        if (data < '0' || data > '9') {
            debug("Arg not valid !");
            goto end;
        }
        
        // Send value for ACK
        if (!verbose) {
            USART_SendByte(data);
        } else {
            printf(", run '%c', arg '%c' ", command, data);
        }

        data = data - '0';

        switch (command) {
            case 's':
                scroll_speed = data;
                eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_SPEED, scroll_speed);
                break;
            case 'd':
                scroll_direction = data;
                eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_DIRECTION, scroll_direction);
                break;
            case 'l':
                char_spacing = data;
                eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_SPACING, char_spacing);
                break;
            case 'w':
                watchdog_value = data;
                if (watchdog_value) {
                    WATCHDOG_RESET
                    TIMSK2 |= (1<<TOIE2);
                } else {
                    TIMSK2 = 0;
                }
                eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_WATCHDOG, watchdog_value);
                break;
            case 'i':
                intensity = data;
                eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_INTENSITY, intensity);

                if (intensity) {
                    TCCR0B = (0<<CS02) | (1<<CS01) | (1<<CS00);
                } else {
                    TCCR0B = (1<<CS02);
                }

                break;
            case 'v':
                verbose = data;
        }

end:
        debug("\n\r");
    }
}
#endif



ISR(TIMER0_OVF_vect)
{
    writeLine(message, index);
}

ISR(TIMER1_OVF_vect)
{
    int size = message_size;

    // Always scroll
#if INTERACTIVE_MODE || !SYNCHRONIZE
    const unsigned char tab[] = { 5, 5, 4, 4, 3, 3, 3, 3, 2, 2 };
    // Scroll auto if str greater than value or if auto chain = true
    if (size > tab[char_spacing] || auto_chain_message) {
#endif
        size *= (5 + char_spacing);
        if (scroll_direction) {
            index = index - STEP;
            if (index < -size) {
                index = MATRIX_COL_COUNT;

#if INTERACTIVE_MODE
                if (auto_chain_message) {
                    loadNextMessage();
                }
#endif
            }
        } else {
            index = index + STEP;
            if (index > MATRIX_COL_COUNT) {
                index = 0 - size;

#if INTERACTIVE_MODE
                if (auto_chain_message) {
                    loadNextMessage();
                }
#endif
            }
        }
#if INTERACTIVE_MODE || !SYNCHRONIZE
    } else {
        index = 0;
    }
#endif

    TCNT1H = speed_table[scroll_speed];
    TCNT1L = 0;
}

ISR(TIMER2_OVF_vect)
{
    if (watchdog_value && watchdog_counter++ >= watchdog_table[watchdog_value]) {
        SET_MESSAGE(WATCHDOG_ERROR_MSG)
        WATCHDOG_RESET
    }
}

ISR(ADC_vect)
{
    adc_value = ADCH;
}

#if INTERACTIVE_MODE
inline void loadNextMessage() {
    unsigned int addr = 0;
    unsigned char magic = 0;
    unsigned int i = 0;

    while (1) {
        current_message++;
        if (current_message >= INTERNAL_MESSAGE_COUNT) {
            current_message = 0;
        }

        addr = EEPROM_CONFIG_ADDR_MESSAGE + (current_message * sizeof(message));

        // Read magic character
        magic = eeprom_read_byte((const void*)addr);
        if (magic == EEPROM_MAGIC_CHAR) {
            break;
        }
    }

    eeprom_read_block((void*)&message, (const void*)addr + 1, sizeof(message) - 1);
}

void loadMessage(unsigned char num) {
    unsigned int addr = 0;
    unsigned char magic = 0;

    if (num > INTERNAL_MESSAGE_COUNT) {
        num = 0;
    }

    addr = EEPROM_CONFIG_ADDR_MESSAGE + (num * sizeof(message));

    // Read magic character
    magic = eeprom_read_byte((const void*)addr);
    if (magic != EEPROM_MAGIC_CHAR) {
        eeprom_update_byte((const void*)addr, EEPROM_MAGIC_CHAR);
        eeprom_update_byte((const void*)addr + 1, 0);
    }

    eeprom_read_block((void*)&message, (const void*)addr + 1, sizeof(message) - 1);
}

void saveMessage(unsigned char num) {
    unsigned int addr = 0;

    if (num > INTERNAL_MESSAGE_COUNT) {
        num = 0;
    }

    addr = EEPROM_CONFIG_ADDR_MESSAGE + (num * sizeof(message));

    eeprom_update_byte((const void*)addr, EEPROM_MAGIC_CHAR);
    eeprom_write_block((void*)&message, (const void*)addr + 1, sizeof(message) - 1);
}
#endif

void loadData() {
    int i = 0;

#if INTERACTIVE_MODE
    // Load data
    loadMessage(current_message);
#else
    eeprom_read_block((void*)&message, (const void*)EEPROM_CONFIG_ADDR_MESSAGE, sizeof(message));

    // First boot ?
    char found = 0;
    for (i = 0; i < sizeof(message); i++) {
        // No 0 ?
        if (!message[i]) {
            found = 1;
            break;
        }
    }

    if (!found) {
        memset(message, 0, sizeof(message));
    }
#endif

    message_size = strlen(message);

/*
    char load_table[] = {
        scroll_speed,       EEPROM_CONFIG_ADDR_SPEED,
        scroll_direction,   EEPROM_CONFIG_ADDR_DIRECTION,
        char_spacing,       EEPROM_CONFIG_ADDR_SPACING,
        intensity,          EEPROM_CONFIG_ADDR_INTENSITY,
        watchdog_value,     EEPROM_CONFIG_ADDR_WATCHDOG
    };

    for (i = 0; i < sizeof(load_table); i++) {
        load_table[i] = eeprom_read_byte(load_table[i + 1]);
        if (load_table[i] < 0 || load_table[i] > 9) {
            load_table[i] = 1;
            eeprom_update_byte(load_table[i + 1], load_table[i]);
        }
    }
*/

    scroll_speed = eeprom_read_byte((const void*)EEPROM_CONFIG_ADDR_SPEED);
    if (scroll_speed >= 9) {
        scroll_speed = 9;
        eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_SPEED, scroll_speed);
    }

    scroll_direction = eeprom_read_byte((const void*)EEPROM_CONFIG_ADDR_DIRECTION);
    if (scroll_direction >= 9) {
        scroll_direction = 9;
        eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_DIRECTION, scroll_direction);
    }

    char_spacing = eeprom_read_byte((const void*)EEPROM_CONFIG_ADDR_SPACING);
    if (char_spacing >= 9) {
        char_spacing = 9;
        eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_SPACING, char_spacing);
    }

    intensity = eeprom_read_byte((const void*)EEPROM_CONFIG_ADDR_INTENSITY);
    if (intensity >= 9) {
        intensity = 9;
        eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_INTENSITY, intensity);
    }

    watchdog_value = eeprom_read_byte((const void*)EEPROM_CONFIG_ADDR_WATCHDOG);
    if (watchdog_value >= 9) {
        watchdog_value = 9;
        eeprom_update_byte((const void*)EEPROM_CONFIG_ADDR_WATCHDOG, watchdog_value);
    }
}

int main(void) {

	clearLeds();

    // Timer 0 configuration : Print message
    TIMSK0 |= (1<<TOIE0);
    TCCR0A = 0;
    TCCR0B |= ((0<<CS02) | (1<<CS01) | (1<<CS00));
    TCNT0 = 0;

    // Timer 1 configuration : Scroll message
    TIMSK1 |= (1<<TOIE1);
    TCCR1A = 0;
    TCCR1B |= ((0<<CS12) | (1<<CS11) | (1<<CS10));
    TCCR1C  = 0;
    TCNT1 = 0;

    // Timer 2 configuration
    TIMSK2 |= 0;
    TCCR2A = 0;
    TCCR2B |= ((1<<CS22) | (1<<CS21) | (1<<CS20));
    TCNT2 = 0;

    // Init ADC
    //ADMUX |= (0 << REFS1) | (0 << REFS0) | (1 << MUX2) | (1 << MUX1);
    ADMUX = 0b01100110;

    //set prescaller and enable ADC
    ADCSRA |= (1 << ADEN) | (1 << ADIE);

    USART_Init();
    stdout = &mystdout;

    memset(message, 0, sizeof(message));

#if INTERACTIVE_MODE
    current_message = 0;
#endif

    loadData();

    sei();

    DDRD = 0;
    PORTD = 0;

    handleAction();
}

