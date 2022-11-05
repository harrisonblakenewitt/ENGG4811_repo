 /** 
 **************************************************************
 * @file M5StickCPlus_Thesis_Code.ino
 * @author HBN - 45300747
 * @date 02102022
 * @brief M5StickC Plus driver file. This file is the M5StickC Plus driver
 *        file for the FarmBot water tank level monitoring & tank level
 *        control thesis. This file handles functionality specific to writing 
 *        to the M5StickC Plus display, requesting height measurement
 *        readings from the Raspberry Pi Pico via UART, connecting to Wi-Fi
 *        and posting height readings to the web dashboard via HTTP. 
 ***************************************************************
 */

#include <M5StickCPlus.h>
#include <WiFi.h>
#include <HTTPClient.h>

// On time of the 'Fetching Readings' screen, in msec
#define FETCH_SCREEN_ON_TIME_MSEC 5000

// On time of the readings display screen, in msec
#define READINGS_SCREEN_ON_TIME_MSEC 5000

// Maximum length of string to be read over UART
#define UART_STR_LEN 30

// UART scan timeout, in msec
#define UART_SCAN_TIMEOUT_MSEC 10000

// Time to sleep between reading requests, in sec
#define SLEEP_TIMEOUT_SEC 300

// Current drawn by battery when almost charged, in mA
#define CHARGED_CURRENT_DRAW_MA 25

// Wi-Fi network name
#define WIFI_NETWORK_NAME "infrastructure"

// Wi-Fi password
#define WIFI_PASSWORD "jvz348qY65Gz"

// Time Wi-Fi connection will be attempted for before giving up
#define WIFI_TIMEOUT_MS 60000

// Name of server with dashboards for which readings are posted to
#define SERVER_NAME "https://api.thingspeak.com/update"

// API keys for each channels which level readings are being written to
#define TANK_1_API_KEY "api_key=9SG92CN42E9MPIX4&field1="
#define TANK_2_API_KEY "api_key=MW670YTJ26U91WLQ&field1="

/**
 * @brief Home pushbutton checker. This function returns the state of the 
 *        home pushbutton
 * @param None. 
 * @retval true if button is pressed, false if it isn't pressed. 
 */
bool check_pushbutton() {
    if (digitalRead(M5_BUTTON_HOME) == LOW) {
        // Loop while button is being held down, so multiple button presses
        // aren't registered. 
        while (digitalRead(M5_BUTTON_HOME) == LOW);
        return true;
    }
    
    return false;
}

/**
 * @brief Fetching Readings screen print function. This function handles 
 *        printing the "Fetching Readings" screen displayed when the user
 *        presses the home button. 
 * @param None. 
 * @retval None. 
 */
void print_fetch_screen() {
    M5.Lcd.setRotation(4);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(18, 30, 4);
    M5.Lcd.print("Fetching");
    M5.Lcd.setCursor(15, 70, 4);
    M5.Lcd.print("Readings");

    // Delay so text on display can be read
    delay(FETCH_SCREEN_ON_TIME_MSEC);
}

/**
 * @brief Reading request function. This function handles height measurement 
 *        requests, which are sent to the Raspberry Pi Pico via UART. 
 * @param None. 
 * @retval None. 
 */
void request_readings() {
    // 'R' is a request for the most recent level height data
    Serial2.print("R");
}

/**
 * @brief String split function. This function takes the string received 
 *        via UART from the Raspberry Pi Pico and splits it to extract the 
 *        height readings, with respect to the agreed string format (which is 
 *        "T1=XX.XT2=XX.X!", where 'XX.X' are height readings). 
 * @param string String for which needs to be split
 * @param first_value Pointer to char array holding the height reading for 
 *        the first tank (passed by reference, which is declared in loop())
 * @param second_value Pointer to char array holding the height reading for 
 *        the second tank (passed by reference, which is declared in loop()) 
 * @retval None. 
 */
void split_str(char *string, char *first_value, char *second_value) {
    // Declare local variables used during string splitting. Note that
    // equal indexes are the indexes of the equals signs in the string to be 
    // split. These are used as index offsets when extracting height values. 
    uint8_t second_val_index = strlen(string) + 1;
    uint8_t equal_1_index = 2, equal_2_index = -1;

    // Begin at index 3. because it is known that index 3 is the location of 
    // the first digit of the tank 1 height. 
    for (uint8_t i = 3; i < strlen(string); i++) {
        // When we read a 'T', first tank height has been extracted and second 
        // tank height is the only tank height left to extract. 
        if (string[i] == 'T') {
            // Set second value index, and as per the agreed message format
            // the index of the second equals is two elements past the second
            // value index. 
            second_val_index = i;
            equal_2_index = (i + 2);
        }

        // If second value hasn't been reached, populate the first value
        // string.
        if (i < second_val_index) {
            first_value[i - (equal_1_index + 1)] = string[i];

        // If second value has been reached, populate the second value
        // string. 
        } else {
            if (i > equal_2_index) {
                second_value[i - (equal_2_index + 1)] = string[i];
            }
        }
    }
}

/**
 * @brief Scan UART function. This function handles scanning for new tank
 *        level measurement readings from the Raspberry Pi Pico via UART. 
 * @param first_value Pointer to char array holding the height reading for 
 *        the first tank (passed by reference, which is declared in loop())
 * @param second_value Pointer to char array holding the height reading for 
 *        the first tank (passed by reference, which is declared in loop())
 * @retval true if a message was received, false if a message was not received.
 */
bool scan_uart(char *first_value, char *second_value) {
    if (Serial2.available()) {
        // Declare received string array
        char receivedString[UART_STR_LEN] = {'\0'};

        // Populate received string array until the agreed termination
        // character is received (which is '!'). 
        for (uint8_t i = 0; i < UART_STR_LEN; i++) {
            char receivedChar = Serial2.read();

            if (receivedChar == '!') {
                break;
            }

            receivedString[i] = receivedChar;
        }

        // Split the received string to extract the heights
        split_str(receivedString, first_value, second_value);

        return true;
    }

    return false;
}

/**
 * @brief Height values screen print function. This function handles 
 *        printing the most recent water tank height values to the display 
 *        after new readings have been received. 
 * @param tank_1_value
 * @param tank_2_value
 * @retval None. 
 */
void print_values_screen(char *tank_1_value, char *tank_2_value) {
    M5.Lcd.setRotation(4);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);

    M5.Lcd.setCursor(20, 30, 2);
    M5.Lcd.print("Tank 1 Level = ");

    M5.Lcd.setCursor(20, 50, 2);
    M5.Lcd.print(String(tank_1_value));
    M5.Lcd.print("cm");

    M5.Lcd.setCursor(20, 90, 2);
    M5.Lcd.print("Tank 2 Level = ");

    M5.Lcd.setCursor(20, 110, 2);
    M5.Lcd.print(String(tank_2_value));
    M5.Lcd.print("cm");

    // Delay so text on display can be read
    delay(READINGS_SCREEN_ON_TIME_MSEC);
    // Reset the display to black
    M5.Lcd.fillScreen(BLACK);
}

/**
 * @brief UART timeout screen print function. This function handles 
 *        printing the timeout screen displayed when waiting for a 
 *        message over UART exceeds the timeout period. 
 * @param None. 
 * @retval None. 
 */
void print_uart_timeout_screen() {
    M5.Lcd.setRotation(4);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(18, 30, 4);
    M5.Lcd.print("Couldn't");
    M5.Lcd.setCursor(34, 70, 4);
    M5.Lcd.print("Fetch");
    M5.Lcd.setCursor(15, 110, 4);
    M5.Lcd.print("Readings");

    // Delay so text on display can be read
    delay(FETCH_SCREEN_ON_TIME_MSEC);
    // Reset the display to black
    M5.Lcd.fillScreen(BLACK);
}

/**
 * @brief Wi-Fi connection handling function. This function handles connecting
 *        to Wi-Fi given the defined network name and password. 
 * @param None. 
 * @retval true if Wi-Fi connection was established, false if Wi-Fi connection
 *         was not established. 
 */
bool wifi_connect() {
    // Set Wi-Fi to station mode to allow connection to an access point, then
    // begin Wi-Fi to attempt to connect. 
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_NETWORK_NAME, WIFI_PASSWORD);

    // Timestamp of connection attempt
    unsigned long wifi_begin_timestamp = millis();

    // Wait until either connection or timeout occurs
    while((WiFi.status() != WL_CONNECTED) && (millis() 
            - wifi_begin_timestamp < WIFI_TIMEOUT_MS));

    // If timeout occurred, return false to denote that connection wasn't
    // established. 
    if(WiFi.status() != WL_CONNECTED){
        return false;
    }
  
    return true;
}

/**
 * @brief Transmit water tank height readings to the web dashboard. This is
 *        done via HTTP post requests. 
 * 
 *        This function was adapted from the example written by Rui Santos, 
 *        see copyright and permission notices below:
 * 
 *        Complete project details at Complete project details at 
 *        https://RandomNerdTutorials.com/esp32-http-get-post-arduino/
 *
 *        Permission is hereby granted, free of charge, to any person obtaining a copy
 *        of this software and associated documentation files.
 *
 *        The above copyright notice and this permission notice shall be included in all
 *        copies or substantial portions of the Software.
 * 
 * @param tank_1_reading Water height reading for tank 1
 * @param tank_2_reading Water height reading for tank 2
 * @retval None. 
 */
void transmit_level_readings(char *tank_1_reading, char *tank_2_reading) {
    HTTPClient http;

    // Connect to server and set the content-type header
    http.begin(SERVER_NAME);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Post readings
    http.POST(String(TANK_1_API_KEY) + String(tank_1_reading));
    http.POST(String(TANK_2_API_KEY) + String(tank_2_reading));

    http.end();
}

/**
 * @brief Busy sleep function. This function executes between tank height 
 *        measurement requests, when the device is charging. 
 * @param None. 
 * @retval None. 
 */
void busy_sleep(void) {
    // Reset display to black
    M5.Lcd.fillScreen(BLACK);

    // Take timestamps of start sleep time, and current runtime
    int start_sleep_timestamp = millis();
    int current_timestamp = millis();

    // Busy sleep while the sleep timeout period hasn't elapsed between sleep
    // start and current time. This loop also breaks out if overflow occurs
    // and the value returned by the millis() call resets to zero. 
    while (((current_timestamp - start_sleep_timestamp) < (SLEEP_TIMEOUT_SEC * 1000)) 
            && (current_timestamp >= start_sleep_timestamp)) {
        // Update current timestamp
        current_timestamp = millis();

        // Check if user has pressed pushbutton
        if (digitalRead(M5_BUTTON_HOME) == LOW) {
            break;
        }

        delay(10);
    }
}

/**
 * @brief Setup. 
 */
void setup() {
    // Begin device and set CPU frequency to 80MHz (to reduce power
    // consumption). 
    M5.begin();
    setCpuFrequencyMhz(80);

    // Initialise UART
    Serial2.begin(9600, SERIAL_8N1, 0, 26);

    // Pull up home button and set it to be a wake up source for deep sleep. 
    pinMode(M5_BUTTON_HOME, INPUT_PULLUP);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 0);

    // Set display to be black. 
    M5.Lcd.fillScreen(BLACK);
}

/**
 * @brief Loop. 
 */
void loop() {  
    // Declare visuals local variable. 
    bool visuals = false;

    // If pushbutton has been pressed, user is requesting to see height
    // readings on the display. Therefore, set visual mode to true and
    // print the fetch screen. 
    if (check_pushbutton()) {
        print_fetch_screen();
        visuals = true;
    }

    // Request new height readings from the Raspberry Pi Pico
    request_readings();

    // Initialise char arrays to hold the strings representing tank heights
    char tank_1_value[6] = {'\0'}, tank_2_value[6] = {'\0'};

    // Take UART start scan timestamp, and declare variable representing
    // UART timeout status. 
    int start_scan_timestamp = millis();
    bool uart_scan_timed_out = false;

    // Scan UART until either a message has been received, or timeout has 
    // occurred. 
    while(!scan_uart((char*)&tank_1_value, (char*)&tank_2_value)) {
        int current_scan_timestamp = millis();
        
        // If timeout occurs, break from loop
        if ((current_scan_timestamp - start_scan_timestamp) > UART_SCAN_TIMEOUT_MSEC) {
            uart_scan_timed_out = true;
            break;
        }
    }

    // If UART didn't time out, print the received values to the 
    // display (if visual mode is on), and transmit the readings
    // to the web dashboard. 
    if (!uart_scan_timed_out) {
        if (visuals) {
            // Print values
            print_values_screen(tank_1_value, tank_2_value);
        }

        if (wifi_connect()) {
            // Send readings to dashboard
            transmit_level_readings(tank_1_value, tank_2_value);
        }
    
    // If UART did time out and visual mode is on, print timeout
    // notification to the display. 
    } else {
        if (visuals) {
            print_uart_timeout_screen();
        }
    }

    // Declare variable denoting if device can deep sleep as per its charge
    // status, and calculate the charging current drawn (as done by the 
    // calculation example in the M5StickC documentation). 
    bool can_sleep = false;
    int charging_current = M5.Axp.GetIchargeData() / 2;

    // If charging current drawn is lower than the charged current draw
    // threshold, device can deep sleep. 
    if (charging_current <= CHARGED_CURRENT_DRAW_MA) {
        can_sleep = true;
    }

    // If device can deep sleep, put device into deep sleep
    if (can_sleep) {
        M5.Axp.DeepSleep(SLEEP_SEC(SLEEP_TIMEOUT_SEC));

    // If device can't deep sleep, put device into busy sleep
    } else {
        busy_sleep();
    }
}