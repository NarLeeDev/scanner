#include "settings.h"

#ifdef OLED_128x64_ADAFRUIT_SCREENS
#include "screens.h" // function headers
#ifdef SH1106
	#include <Adafruit_SH1106.h>
#else
	#include <Adafruit_SSD1306.h>
#endif
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <SPI.h>

// New version of PSTR that uses a temp buffer and returns char *
// by Shea Ivey
#define PSTR2(x) PSTRtoBuffer_P(PSTR(x))
char PSTR2_BUFFER[30]; // adjust size depending on need.
char *PSTRtoBuffer_P(PGM_P str) { uint8_t c='\0', i=0; for(; (c = pgm_read_byte(str)) && i < sizeof(PSTR2_BUFFER); str++, i++) PSTR2_BUFFER[i]=c;PSTR2_BUFFER[i]=c; return PSTR2_BUFFER;}

#define INVERT INVERSE
#define OLED_RESET 4
#ifdef SH1106
	Adafruit_SH1106 display(OLED_RESET);
	#if !defined SH1106_128_64
		//#error("Screen size incorrect, please fix Adafruit_SH1106.h!");
	#endif
#else
	Adafruit_SSD1306 display(OLED_RESET);
	#if !defined SSD1306_128_64
		//#error("Screen size incorrect, please fix Adafruit_SSD1306.h!");
	#endif
#endif


screens::screens() {
    last_channel = -1;
    last_rssi = 0;
}

char screens::begin(const char *call_sign) {
    // Set the address of your OLED Display.
    // 128x64 ONLY!!
#ifdef SH1106
    display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D or 0x3C (for the 128x64)
#else
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D or 0x3C (for the 128x64)
#endif


#ifdef USE_FLIP_SCREEN
    flip();
#endif

#ifdef USE_BOOT_LOGO
    display.display(); // show splash screen
    delay(3000);
#endif
    // init done
    reset();

    display.fillRect(0, 0, display.width(), 11,WHITE);
    display.setTextColor(BLACK);
    display.setCursor(((display.width() - (10*6)) / 2),2);
    display.print(PSTR2("ПРОВЕРКА"));

    display.setTextColor(WHITE);
    display.setCursor(0,8*1+4);
    display.print(PSTR2("БАТ:"));
    display.setCursor(display.width()-6*3,8*1+4);
    display.print(PSTR2("ДА"));
    display.setCursor(0,8*2+4);

    display.display();
#ifdef USE_DIVERSITY
    display.print(PSTR2("ПАРА:"));
    display.display();
    delay(250);
    display.setCursor(display.width()-6*9,8*2+4);
    if(isDiversity()) {
        display.print(PSTR2(" ВКЛ"));
    }
    else {
        display.print(PSTR2("ОТКЛ"));
    }
#endif
    display.setCursor(((display.width() - (strlen(call_sign)*12)) / 2),8*4+4);
    display.setTextSize(2);
    display.print(call_sign);
    display.display();
    delay(1250);
    return 0; // no errors
}

void screens::reset() {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
}

void screens::flip() {
    display.setRotation(2);
}

void screens::drawTitleBox(const char *title) {
    display.drawRect(0, 0, display.width(), display.height(),WHITE);
    display.fillRect(0, 0, display.width(), 11,WHITE);

    display.setTextSize(1);
    display.setTextColor(BLACK);
    // center text
    display.setCursor(((display.width() - (strlen(title)*6)) / 2),2);
    display.print(title);
    display.setTextColor(WHITE);
}

void screens::mainMenu(uint8_t menu_id) {
    reset(); // start from fresh screen.
    drawTitleBox(PSTR2("РЕЖИМ"));

    display.fillRect(0, 10*menu_id+12, display.width(), 10,WHITE);

    display.setTextColor(menu_id == 0 ? BLACK : WHITE);
    display.setCursor(5,10*0+13);
    display.print(PSTR2("АВТОПОИСК"));
    display.setTextColor(menu_id == 1 ? BLACK : WHITE);
    display.setCursor(5,10*1+13);
    display.print(PSTR2("СКАНЕР КАНАЛА"));
    display.setTextColor(menu_id == 2 ? BLACK : WHITE);
    display.setCursor(5,10*2+13);
    display.print(PSTR2("РУЧНОЙ"));

#ifdef USE_DIVERSITY
    if(isDiversity())
    {
        display.setTextColor(menu_id == 3 ? BLACK : WHITE);
        display.setCursor(5,10*3+13);
        display.print(PSTR2("ПАРА"));
    }
#endif
    display.setTextColor(menu_id == 4 ? BLACK : WHITE);
    display.setCursor(5,10*4+13);
    display.print(PSTR2("МЕНЮ"));

    display.display();
}

void screens::seekMode(uint8_t state) {
    last_channel = -1;
    reset(); // start from fresh screen.
    if (state == STATE_MANUAL)
    {
        drawTitleBox(PSTR2("РУЧНОЙ"));
    }
    else if(state == STATE_SEEK)
    {
        drawTitleBox(PSTR2("АВТОПОИСК"));
    }
    display.setTextColor(WHITE);
    display.drawLine(0, 20, display.width(), 20, WHITE);
    display.drawLine(0, 32, display.width(), 32, WHITE);
    display.setCursor(5,12);
    display.drawLine(97,11,97,20,WHITE);
    display.print(PSTR2("КАНАЛ:"));
    for(uint16_t i=0;i<8;i++) {
        display.setCursor(15*i+8,23);
        display.print((char) (i+'1'));
    }
    display.drawLine(0, 36, display.width(), 36, WHITE);
    display.drawLine(0, display.height()-11, display.width(), display.height()-11, WHITE);
    display.setCursor(2,display.height()-9);
#ifdef USE_LBAND
    display.print(PSTR2("5362"));
#else
    display.print(PSTR2("5645"));
#endif
    display.setCursor(55,display.height()-9);
    display.print(PSTR2("5800"));
    display.setCursor(display.width()-25,display.height()-9);
    display.print(PSTR2("5945"));
    display.display();
}

char scan_position = 3;

void screens::updateSeekMode(uint8_t state, uint8_t channelIndex, uint8_t channel, uint8_t rssi, uint16_t channelFrequency, uint8_t rssi_seek_threshold, bool locked) {
    // display refresh handler
    if(channel != last_channel) // only updated on changes
    {
        if(channel > last_channel) {
            scan_position = 3;
        }
        else {
            scan_position = -1;
        }
        display.setTextColor(WHITE,BLACK);
        display.setCursor(36,12);
        // show current used channel of bank
#ifdef USE_LBAND
        if(channelIndex > 39)
        {
            display.print(PSTR2("D/5.3    "));
        }
        else if(channelIndex > 31)
#else
	if(channelIndex > 31)
#endif
        {
            display.print(PSTR2("C/Race   "));
        }
        else if(channelIndex > 23)
        {
            display.print(PSTR2("F/Airwave"));
        }
        else if (channelIndex > 15)
        {
            display.print(PSTR2("E        "));
        }
        else if (channelIndex > 7)
        {
            display.print(PSTR2("B        "));
        }
        else
        {
            display.print(PSTR2("A        "));
        }

        uint8_t active_channel = channelIndex%CHANNEL_BAND_SIZE; // get channel inside band
        for(int i=0;i<8;i++) {
            display.fillRect(15*i+4,21,14,11,i==active_channel? WHITE:BLACK);
            display.setTextColor(i==active_channel? BLACK:WHITE);
            display.setCursor(15*i+8,23);
            display.print((char) (i+'1'));
        }

        // show frequence
        display.setCursor(101,12);
        display.setTextColor(WHITE,BLACK);
        display.print(channelFrequency);
    }
    // show signal strength
    uint8_t rssi_scaled=map(rssi, 1, 100, 1, display.width()-3);

    display.fillRect(1+rssi_scaled, 33, display.width()-3-rssi_scaled, 3, BLACK);
    display.fillRect(1, 33, rssi_scaled, 3, WHITE);

    rssi_scaled=map(rssi, 1, 100, 1, 14);
#ifdef USE_LBAND
    display.fillRect((channel*3)+4,display.height()-12-14,5/2,14-rssi_scaled,BLACK);
    display.fillRect((channel*3)+4,(display.height()-12-rssi_scaled),5/2,rssi_scaled,WHITE);
#else
    display.fillRect((channel*3)+4,display.height()-12-14,3,14-rssi_scaled,BLACK);
    display.fillRect((channel*3)+4,(display.height()-12-rssi_scaled),3,rssi_scaled,WHITE);
#endif    
    
    // handling for seek mode after screen and RSSI has been fully processed
    if(state == STATE_SEEK) //
    { // SEEK MODE
       
        // Show Scan Position
#ifdef USE_LBAND
        display.fillRect((channel*5/2)+4+scan_position,display.height()-12-14,1,14,BLACK);
#else
        display.fillRect((channel*3)+4+scan_position,display.height()-12-14,1,14,BLACK);
#endif

        rssi_scaled=map(rssi_seek_threshold, 1, 100, 1, 14);

        display.fillRect(1,display.height()-12-14,2,14,BLACK);
        display.drawLine(1,display.height()-12-rssi_scaled,2,display.height()-12-rssi_scaled, WHITE);
        display.fillRect(display.width()-3,display.height()-12-14,2,14,BLACK);
        display.drawLine(display.width()-3,display.height()-12-rssi_scaled,display.width(),display.height()-12-rssi_scaled, WHITE);

        if(locked) // search if not found
        {
            display.setTextColor(BLACK,WHITE);
            display.setCursor(((display.width()-14*6)/2),2);
            display.print(PSTR2("НАШЕЛ"));
        }
        else
        {
            display.setTextColor(BLACK,WHITE);
            display.setCursor(((display.width()-14*6)/2),2);
            display.print(PSTR2("ПОИСК"));
        }
    }

    last_channel = channel;
    display.display();
}

void screens::bandScanMode(uint8_t state) {
    reset(); // start from fresh screen.
    best_rssi = 0;
    if(state==STATE_SCAN)
    {
        drawTitleBox(PSTR2("СКАНЕР КАНАЛА"));
        display.setCursor(5,12);
        display.print(PSTR2("ПИК:"));
    }
    else
    {
        drawTitleBox(PSTR2("RSSI УСТ"));
        display.setCursor(5,12);
        display.print(PSTR2("МИН:     МАКС:"));
    }
    display.drawLine(0, 20, display.width(), 20, WHITE);

    display.drawLine(0, display.height()-11, display.width(), display.height()-11, WHITE);
    display.setCursor(2,display.height()-9);
#ifdef USE_LBAND
    display.print(PSTR2("5362"));
#else
    display.print(PSTR2("5645"));
#endif
    display.setCursor(55,display.height()-9);
    display.print(PSTR2("5800"));
    display.setCursor(display.width()-25,display.height()-9);
    display.print(PSTR2("5945"));
    display.display();
}

void screens::updateBandScanMode(bool in_setup, uint8_t channel, uint8_t rssi, uint8_t channelName, uint16_t channelFrequency, uint16_t rssi_setup_min_a, uint16_t rssi_setup_max_a) {
    #define SCANNER_LIST_X_POS 60
    static uint8_t writePos = SCANNER_LIST_X_POS;
    uint8_t rssi_scaled=map(rssi, 1, 100, 1, 30);
    uint16_t hight = (display.height()-12-rssi_scaled);
    if(channel != last_channel) // only updated on changes
    {
#ifdef USE_LBAND
        display.fillRect((channel*5/2)+4,display.height()-12-30,5/2,30-rssi_scaled,BLACK);
        display.fillRect((channel*5/2)+4,hight,5/2,rssi_scaled,WHITE);
        // Show Scan Position
        display.fillRect((channel*5/2)+4+3,display.height()-12-30,1,30,BLACK);
#else
        display.fillRect((channel*3)+4,display.height()-12-30,3,30-rssi_scaled,BLACK);
        display.fillRect((channel*3)+4,hight,3,rssi_scaled,WHITE);
        // Show Scan Position
        display.fillRect((channel*3)+4+3,display.height()-12-30,1,30,BLACK);
#endif
    }
    if(!in_setup) {
        if (rssi > RSSI_SEEK_TRESHOLD) {
            if(best_rssi < rssi) {
                best_rssi = rssi;
                display.setTextColor(WHITE,BLACK);
                display.setCursor(36,12);
                display.print(channelName, HEX);
                display.setCursor(52,12);
                display.print(channelFrequency);
            }
            else {
                if(writePos+10>display.width()-12)
                { // keep writing on the screen
                    writePos=SCANNER_LIST_X_POS;
                }
            }
        }
    }
    else {
        display.setCursor(30,12);
        display.setTextColor(WHITE,BLACK);
        display.print( PSTR2("   ") );
        display.setCursor(30,12);
        display.print( rssi_setup_min_a , DEC);

        display.setCursor(85,12);
        display.setTextColor(WHITE,BLACK);
        display.setCursor(85,12);
        display.print( PSTR2("   ") );
        display.print( rssi_setup_max_a , DEC);
    }
    display.display();
    last_channel = channel;
}

void screens::screenSaver(uint8_t channelName, uint16_t channelFrequency, const char *call_sign) {
    screenSaver(-1, channelName, channelFrequency, call_sign);
}

void screens::screenSaver(uint8_t diversity_mode, uint8_t channelName, uint16_t channelFrequency, const char *call_sign) {
    reset();
    display.setTextSize(6);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print(channelName, HEX);
    display.setTextSize(1);
    display.setCursor(70,0);
    display.print(call_sign);
    display.setTextSize(2);
    display.setCursor(70,28);
    display.setTextColor(WHITE);
    display.print(channelFrequency);
    display.setTextSize(1);
#ifdef USE_DIVERSITY
    if(isDiversity()) {
        display.setCursor(70,18);
        switch(diversity_mode) {
            case useReceiverAuto:
                display.print(PSTR2("AUTO"));
                break;
            case useReceiverA:
                display.print(PSTR2("АНТ 1"));
                break;
            case useReceiverB:
                display.print(PSTR2("АНТ 2"));
                break;
        }
        display.setTextColor(BLACK,WHITE);
        display.fillRect(0, display.height()-19, 7, 9, WHITE);
        display.setCursor(1,display.height()-18);
        display.print("A");
        display.setTextColor(BLACK,WHITE);
        display.fillRect(0, display.height()-9, 7, 9, WHITE);
        display.setCursor(1,display.height()-8);
        display.print("B");
    }
#endif
    display.display();
}

void screens::updateScreenSaver(uint8_t rssi) {
    updateScreenSaver(-1, rssi, -1, -1);
}
#ifdef USE_VOLTAGE_ALERT
  void screens::updateSceenSaverVoltage(uint8_t voltage, bool isAlert){
    if(isAlert)
    {
        display.setTextColor((millis()%250 < 125) ? WHITE : BLACK, BLACK);
    }else{
        display.setTextColor(WHITE);
    }
    
    display.fillRect(70, 9, display.width() - 70, 8, BLACK);
    display.setCursor(70,9);
    display.print(String((float)voltage/10.0, 1) + "V");
    display.display();
  }
#endif

void screens::updateScreenSaver(char active_receiver, uint8_t rssi, uint8_t rssiA, uint8_t rssiB) {
#ifdef USE_DIVERSITY
    if(isDiversity()) {
        // read rssi A
        #define RSSI_BAR_SIZE 119
        uint8_t rssi_scaled=map(rssiA, 1, 100, 3, RSSI_BAR_SIZE);
        display.fillRect(7 + rssi_scaled, display.height()-19, (RSSI_BAR_SIZE-rssi_scaled), 9, BLACK);
        if(active_receiver == useReceiverA)
        {
            display.fillRect(7, display.height()-19, rssi_scaled, 9, WHITE);
        }
        else
        {
            display.fillRect(7, display.height()-19, (RSSI_BAR_SIZE), 9, BLACK);
            display.drawRect(7, display.height()-19, rssi_scaled, 9, WHITE);
        }

        // read rssi B
        rssi_scaled=map(rssiB, 1, 100, 3, RSSI_BAR_SIZE);
        display.fillRect(7 + rssi_scaled, display.height()-9, (RSSI_BAR_SIZE-rssi_scaled), 9, BLACK);
        if(active_receiver == useReceiverB)
        {
            display.fillRect(7, display.height()-9, rssi_scaled, 9, WHITE);
        }
        else
        {
            display.fillRect(7, display.height()-9, (RSSI_BAR_SIZE), 9, BLACK);
            display.drawRect(7, display.height()-9, rssi_scaled, 9, WHITE);
        }
    }
    else {
        display.setTextColor(BLACK);
        display.fillRect(0, display.height()-19, 25, 19, WHITE);
        display.setCursor(1,display.height()-13);
        display.print(PSTR2("RSSI"));
        #define RSSI_BAR_SIZE 101
        uint8_t rssi_scaled=map(rssi, 1, 100, 1, RSSI_BAR_SIZE);
        display.fillRect(25 + rssi_scaled, display.height()-19, (RSSI_BAR_SIZE-rssi_scaled), 19, BLACK);
        display.fillRect(25, display.height()-19, rssi_scaled, 19, WHITE);
    }
#else
    display.setTextColor(BLACK);
    display.fillRect(0, display.height()-19, 25, 19, WHITE);
    display.setCursor(1,display.height()-13);
    display.print(PSTR2("RSSI"));
    #define RSSI_BAR_SIZE 101
    uint8_t rssi_scaled=map(rssi, 1, 100, 1, RSSI_BAR_SIZE);
    display.fillRect(25 + rssi_scaled, display.height()-19, (RSSI_BAR_SIZE-rssi_scaled), 19, BLACK);
    display.fillRect(25, display.height()-19, rssi_scaled, 19, WHITE);
#endif
    if(rssi < 20)
    {
        display.setTextColor((millis()%250 < 125) ? WHITE : BLACK, BLACK);
        display.setCursor(50,display.height()-13);
        display.print(PSTR2("LOW SIGNAL"));
    }
#ifdef USE_DIVERSITY
    else if(isDiversity()) {
        display.drawLine(50,display.height()-10,110,display.height()-10,BLACK);
    }
#endif
    display.display();
}

#ifdef USE_DIVERSITY
void screens::diversity(uint8_t diversity_mode) {

    reset();
    drawTitleBox(PSTR2("ПАРА"));

    //selected
    display.fillRect(0, 10*diversity_mode+12, display.width(), 10, WHITE);

    display.setTextColor(diversity_mode == useReceiverAuto ? BLACK : WHITE);
    display.setCursor(5,10*1+3);
    display.print(PSTR2("АВТО"));

    display.setTextColor(diversity_mode == useReceiverA ? BLACK : WHITE);
    display.setCursor(5,10*2+3);
    display.print(PSTR2("ПРИЕМ 1"));
    display.setTextColor(diversity_mode == useReceiverB ? BLACK : WHITE);
    display.setCursor(5,10*3+3);
    display.print(PSTR2("ПРИЕМ 2"));

    // RSSI Strength
    display.setTextColor(WHITE);
    display.drawRect(0, display.height()-21, display.width(), 11, WHITE);
    display.setCursor(5,display.height()-19);
    display.print("A:");
    display.setCursor(5,display.height()-9);
    display.print("B:");
    display.display();
}

void screens::updateDiversity(char active_receiver, uint8_t rssiA, uint8_t rssiB){
    #define RSSI_BAR_SIZE 108
    uint8_t rssi_scaled=map(rssiA, 1, 100, 1, RSSI_BAR_SIZE);

    display.fillRect(18 + rssi_scaled, display.height()-19, (RSSI_BAR_SIZE-rssi_scaled), 7, BLACK);
    if(active_receiver==useReceiverA)
    {
        display.fillRect(18, display.height()-19, rssi_scaled, 7, WHITE);
    }
    else
    {
        display.fillRect(18, display.height()-19, rssi_scaled, 7, BLACK);
        display.drawRect(18, display.height()-19, rssi_scaled, 7, WHITE);
    }

    // read rssi B
    rssi_scaled=map(rssiB, 1, 100, 1, RSSI_BAR_SIZE);
    display.fillRect(18 + rssi_scaled, display.height()-9, (RSSI_BAR_SIZE-rssi_scaled), 7, BLACK);
    if(active_receiver==useReceiverB)
    {
        display.fillRect(18, display.height()-9, rssi_scaled, 7, WHITE);
    }
    else
    {
        display.fillRect(18, display.height()-9, rssi_scaled, 7, BLACK);
        display.drawRect(18, display.height()-9, rssi_scaled, 7, WHITE);
    }
    display.display();
}
#endif

#ifdef USE_VOLTAGE_ALERT
  void screens::voltageAlert() {
  
      reset();
      drawTitleBox(PSTR2("КРИТ БАТ"));

      display.fillTriangle(35, 60, 85, 60, 60, 15, WHITE);
      display.fillRect(57, 23, 7 , 26, BLACK);
      display.fillCircle(60, 55, 4, BLACK);
      
      display.display();
  }
#endif


void screens::setupMenu(){
}
void screens::updateSetupMenu(uint8_t menu_id, bool settings_beeps, bool settings_orderby_channel, const char *call_sign, uint8_t voltage_alert, char editing){
    reset();
    
    drawTitleBox(PSTR2("МЕНЮ"));
    //selected

    if(menu_id <= 4){ // first page
        display.fillRect(0, 10*menu_id+12, display.width(), 10, WHITE);
        display.setTextColor(menu_id == 0 ? BLACK : WHITE);
        display.setCursor(5,10*1+3);
        display.print(PSTR2("ПОРЯДОК: "));
        if(settings_orderby_channel) {
            display.print(PSTR2("НОМ КАНАЛА  "));
        }
        else {
            display.print(PSTR2("ЧАСТОТА"));
        }
    
        display.setTextColor(menu_id == 1 ? BLACK : WHITE);
        display.setCursor(5,10*2+3);
        display.print(PSTR2("СИГНАЛ: "));
        if(settings_beeps) {
            display.print(PSTR2("ДА "));
        }
        else {
            display.print(PSTR2("НЕТ"));
        }
    
    
        display.setTextColor(menu_id == 2 ? BLACK : WHITE);
        display.setCursor(5,10*3+3);
        display.print(PSTR2("SIGN : "));
        if(editing>=0) {
            display.fillRect(6*6+5, 10*2+13, display.width()-(6*6+6), 8, BLACK);
            display.fillRect(6*7+6*(editing)+4, 10*2+13, 7, 8, WHITE); //set cursor
            for(uint8_t i=0; i<10; i++) {
                display.setTextColor(i == editing ? BLACK : WHITE);
                display.print(call_sign[i]);
            }
        }
        else {
            display.print(call_sign);
        }
    
        display.setTextColor(menu_id == 3 ? BLACK : WHITE);
        display.setCursor(5,10*4+3);
        display.print(PSTR2("КАЛ RSSI"));
    
        display.setTextColor(menu_id == 4 ? BLACK : WHITE);
        display.setCursor(5,10*5+3);
        display.print(PSTR2("СОХР"));
    }else{ //second page
        display.fillRect(0, 10*(menu_id - 5)+12, display.width(), 10, WHITE);
        display.setTextColor(menu_id == 5 ? BLACK : WHITE);
        display.setCursor(5,10*1+3);
        display.print(PSTR2("БАТ ЛИМИТ: "));
        
        if(editing == 0 && menu_id == 5) {
          display.fillRect(85, 10*(menu_id - 5)+10, 40, 12, BLACK);
          display.setTextColor(WHITE);
        }
        
        if(voltage_alert > 0){
          display.print(String((float)voltage_alert / 10.0, 1));
        }else{
          display.print(PSTR2("ВЫКЛ"));
        }
    }
    
    display.display();
}

void screens::save(uint8_t mode, uint8_t channelIndex, uint16_t channelFrequency,const char *call_sign) {
    reset();
    drawTitleBox(PSTR2("СОХРАНИТЬ"));

    display.setTextColor(WHITE);
    display.setCursor(5,8*1+4);
    display.print(PSTR2("MODE:"));
    display.setCursor(38,8*1+4);
    switch (mode)
    {
        case STATE_SCAN: // Band Scanner
            display.print(PSTR2("СКАНЕР КАНАЛА"));
        break;
        case STATE_MANUAL: // manual mode
            display.print(PSTR2("РУЧНОЙ"));
        break;
        case STATE_SEEK: // seek mode
            display.print(PSTR2("АВТО"));
        break;
    }

    display.setCursor(5,8*2+4);
    display.print(PSTR2("КАНАЛ:"));
    display.setCursor(38,8*2+4);
    // print band
#ifdef USE_LBAND
    if(channelIndex > 39)
    {
        display.print(PSTR2("D/5.3    "));
    }
    else if(channelIndex > 31)
#else
    if(channelIndex > 31)
#endif
    {
        display.print(PSTR2("C/Race"));
    }
    else if(channelIndex > 23)
    {
        display.print(PSTR2("F/Airwave"));
    }
    else if (channelIndex > 15)
    {
        display.print(PSTR2("E"));
    }
    else if (channelIndex > 7)
    {
        display.print(PSTR2("B"));
    }
    else
    {
        display.print(PSTR2("A"));
    }

    display.setCursor(5,8*3+4);
    display.print(PSTR2("КАНАЛ:"));
    display.setCursor(38,8*3+4);
    uint8_t active_channel = channelIndex%CHANNEL_BAND_SIZE+1; // get channel inside band
    display.print(active_channel,DEC);
    display.setCursor(5,8*4+4);
    display.print(PSTR2("ЧАСТ:     ГГц"));
    display.setCursor(38,8*4+4);
    display.print(channelFrequency);

    display.setCursor(5,8*5+4);
    display.print(PSTR2("SIGN:"));
    display.setCursor(38,8*5+4);
    display.print(call_sign);

    display.setCursor(((display.width()-11*6)/2),8*6+4);
    display.print(PSTR2("-- SAVED --"));
    display.display();
}

void screens::updateSave(const char * msg) {
    display.setTextColor(WHITE,BLACK);
    display.setCursor(((display.width()-strlen(msg)*6)/2),8*6+4);
    display.print(msg);
    display.display();
}


#endif
