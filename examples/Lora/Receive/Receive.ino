/*
LORA receive Test example

This example is a receive test for Lora. Using lora testing requires two devices to test, 
one of which burns the lora receive sample and the other writes the Lora transmit sample.
You can view the output in the serial port.

 This example code is in the public domain.
 */
#include "utilities.h"
#include <SPI.h>

// #include <GxEPD.h>
//#include <GxGDEP015OC1/GxGDEP015OC1.h>    // 1.54" b/w
//#include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w
// #include <GxDEPG0150BN/GxDEPG0150BN.h>  // 1.54" b/w

// #include GxEPD_BitmapExamples
// #include <Fonts/FreeMonoBold12pt7b.h>
// #include <GxIO/GxIO_SPI/GxIO_SPI.h>
// #include <GxIO/GxIO.h>

#include <RadioLib.h>

// Three states: 0, 1, and 2.
int state = 0;

// Change button increments this counter in Edit mode.
int counter = 0;

// LongPress on ModeButton will go into "edit" mode.
bool isEditting = false;

// In edit mode, the "field" is blinking. But when the Change button is
// Pressed or LongPressed, the blinking temporarily stops.
bool isBlinking = false;

SPIClass        *dispPort  = nullptr;
SPIClass        *rfPort    = nullptr;
GxIO_Class      *io        = nullptr;
GxEPD_Class     *display   = nullptr;
SX1262          radio      = nullptr;       //SX1262
uint32_t        blinkMillis = 0;
uint8_t rgb = 0;
// flag to indicate that a packet was received
volatile bool receivedFlag = false;
// disable interrupt when it's not needed
volatile bool enableInterrupt = true;


void setFlag(void);
// void setupDisplay();
// void enableBacklight();
void loopReciver();
bool setupLoRa();
void configVDD(void);
void boardInit();
// void LilyGo_logo(void);

void setup()
{
    Serial.begin(115200);
    delay(200);
    boardInit();
    delay(200);
    // LilyGo_logo();

}

void loop()
{

    if (millis() - blinkMillis > 1000) {

        blinkMillis = millis();
        switch (rgb) {
        case 0:
            digitalWrite(GreenLed_Pin, LOW);
            digitalWrite(RedLed_Pin, HIGH);
            digitalWrite(BlueLed_Pin, HIGH);
            break;
        case 1:
            digitalWrite(GreenLed_Pin, HIGH);
            digitalWrite(RedLed_Pin, LOW);
            digitalWrite(BlueLed_Pin, HIGH);
            break;
        case 2:
            digitalWrite(GreenLed_Pin, HIGH);
            digitalWrite(RedLed_Pin, HIGH);
            digitalWrite(BlueLed_Pin, LOW);
            break;
        default :
            break;
        }
        rgb++;
        rgb %= 3;
    }
    loopReceiver();
}

void LilyGo_logo(void)
{
    display->fillScreen(GxEPD_WHITE);
    display->drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display->update();
}
 
void enableBacklight(bool en)
{
    digitalWrite(ePaper_Backlight, en);
}

// void setupDisplay()
// {
//     dispPort = new SPIClass(
//         /*SPIPORT*/NRF_SPIM2,
//         /*MISO*/ ePaper_Miso,
//         /*SCLK*/ePaper_Sclk,
//         /*MOSI*/ePaper_Mosi);

//     io = new GxIO_Class(
//         *dispPort,
//         /*CS*/ ePaper_Cs,
//         /*DC*/ ePaper_Dc,
//         /*RST*/ePaper_Rst);

//     display = new GxEPD_Class(
//         *io,
//         /*RST*/ ePaper_Rst,
//         /*BUSY*/ ePaper_Busy);

//     dispPort->begin();
//     display->init(/*115200*/);
//     display->setRotation(2);
//     display->fillScreen(GxEPD_WHITE);
//     display->setTextColor(GxEPD_BLACK);
//     display->setFont(&FreeMonoBold12pt7b);
// }

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void)  
{
    // check if the interrupt is enabled  
    if (!enableInterrupt) {
        return;
    }

    // we got a packet, set the flag
    receivedFlag = true;
}

bool setupLoRa()
{
    rfPort = new SPIClass(
        /*SPIPORT*/NRF_SPIM3,
        /*MISO*/ LoRa_Miso,
        /*SCLK*/LoRa_Sclk,
        /*MOSI*/LoRa_Mosi);
    rfPort->begin();

    SPISettings spiSettings;

    radio = new Module(LoRa_Cs, LoRa_Dio1, LoRa_Rst, LoRa_Busy, *rfPort, spiSettings);

    SerialMon.print("[SX1262] Initializing ...  ");
    // carrier frequency:           868.0 MHz
    // bandwidth:                   125.0 kHz
    // spreading factor:            9
    // coding rate:                 7
    // sync word:                   0x12 (private network)
    // output power:                14 dBm
    // current limit:               60 mA
    // preamble length:             8 symbols
    // TCXO voltage:                1.6 V (set to 0 to not use TCXO)
    // regulator:                   DC-DC (set to true to use LDO)
    // CRC:                         enabled
    int state = radio.begin(868.0);
    if (state != ERR_NONE) {
        SerialMon.print(("failed, code "));
        SerialMon.println(state);
        return false;
    }



    SerialMon.println(" success");
    return true;


}

void loopReceiver()
{

    SerialMon.print(F("[SX1262] Waiting for incoming transmission ... "));

    // you can receive data as an Arduino String
    // NOTE: receive() is a blocking method!
    //       See example ReceiveInterrupt for details
    //       on non-blocking reception method.
    String str;
    int state = radio.receive(str);

    // you can also receive data as byte array
    /*
      byte byteArr[8];
      int state = radio.receive(byteArr, 8);
    */

    if (state == ERR_NONE) {
        // packet was successfully received
        SerialMon.println(F("success!"));

        // print the data of the packet
        SerialMon.print(F("[SX1262] Data:\t\t"));
        SerialMon.println(str);

        // print the RSSI (Received Signal Strength Indicator)
        // of the last received packet
        SerialMon.print(F("[SX1262] RSSI:\t\t"));
        SerialMon.print(radio.getRSSI());
        SerialMon.println(F(" dBm"));

        // print the SNR (Signal-to-Noise Ratio)
        // of the last received packet
        SerialMon.print(F("[SX1262] SNR:\t\t"));
        SerialMon.print(radio.getSNR());
        SerialMon.println(F(" dB"));

    } else if (state == ERR_RX_TIMEOUT) {
        // timeout occurred while waiting for a packet
        SerialMon.println(F("timeout!"));

    } else if (state == ERR_CRC_MISMATCH) {
        // packet was received, but is malformed
        SerialMon.println(F("CRC error!"));

    } else {
        // some other error occurred
        SerialMon.print(F("failed, code "));
        SerialMon.println(state);

    }

}

void configVDD(void)
{
    // Configure UICR_REGOUT0 register only if it is set to default value.
    if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
            (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos)) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}

        NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                            (UICR_REGOUT0_VOUT_3V3 << UICR_REGOUT0_VOUT_Pos);

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}

        // System reset is needed to update UICR registers.
        NVIC_SystemReset();
    }
}

void boardInit()
{
    uint8_t rlst = 0;

#ifdef HIGH_VOLTAGE
    configVDD();
#endif

    SerialMon.begin(MONITOR_SPEED);
    // delay(5000);
    // while (!SerialMon);
    SerialMon.println("Start\n");

    uint32_t reset_reason;
    sd_power_reset_reason_get(&reset_reason);
    SerialMon.print("sd_power_reset_reason_get:");
    SerialMon.println(reset_reason, HEX);

    pinMode(Power_Enable_Pin, OUTPUT);
    digitalWrite(Power_Enable_Pin, HIGH);

    pinMode(ePaper_Backlight, OUTPUT);
    enableBacklight(false);//off Backlight

    pinMode(GreenLed_Pin, OUTPUT);
    pinMode(RedLed_Pin, OUTPUT);
    pinMode(BlueLed_Pin, OUTPUT);

    pinMode(UserButton_Pin, INPUT_PULLUP);
    pinMode(Touch_Pin, INPUT_PULLUP);

    int i = 10;
    while (i--) {
        digitalWrite(GreenLed_Pin, !digitalRead(GreenLed_Pin));
        digitalWrite(RedLed_Pin, !digitalRead(RedLed_Pin));
        digitalWrite(BlueLed_Pin, !digitalRead(BlueLed_Pin));
        delay(300);
    }
    digitalWrite(GreenLed_Pin, HIGH);
    digitalWrite(RedLed_Pin, HIGH);
    digitalWrite(BlueLed_Pin, HIGH);

    setupDisplay();
    setupLoRa();
}
