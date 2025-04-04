#include "settings.h"

#ifdef OLED_128x64_U8G_SCREENS
#include "screens.h" // function headers
#include <U8glib.h>

#define BLACK 0
#define WHITE 1


U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);	// Fast I2C / TWI

screens::screens() {
    reset();
}


char screens::begin() {
    return 0;
}


void screens::reset() {
    u8g.setFont(u8g_font_fixed_v0);
    u8g.setFontRefHeightExtendedText();
    u8g.setDefaultForegroundColor();
    u8g.setFontPosTop();
    u8g.setColorIndex(WHITE);
}

void screens::flip() {
    u8g.setRot180();
}

void screens::drawTitleBox(const char *title) {
    u8g.setColorIndex(WHITE);
    u8g.drawFrame(0, 0, u8g.getWidth(), u8g.getHeight());
    u8g.drawBox(0, 0, u8g.getWidth(), 11);
    u8g.setColorIndex(BLACK);

    // center text
    u8g.setPrintPos(((u8g.getWidth() - u8g.getStrWidth(title)) / 2),1);
    u8g.print(title);
}


void screens::mainMenu(uint8_t menu_id){
    u8g.firstPage();
    do {
        reset(); // start from fresh screen.
        drawTitleBox("ВЫБОР");
        u8g.setColorIndex(WHITE);
        u8g.drawBox(0, 10*menu_id+12, u8g.getWidth(), 10);

        u8g.setColorIndex(menu_id == 0 ? BLACK : WHITE);
        u8g.setPrintPos(5,10*0+12);
        u8g.print("АВТО ПОИСК");
        u8g.setColorIndex(menu_id == 1 ? BLACK : WHITE);
        u8g.setPrintPos(5,10*1+12);
        u8g.print("СКАНЕР КАНАЛА");
        u8g.setColorIndex(menu_id == 2 ? BLACK : WHITE);
        u8g.setPrintPos(5,10*2+12);
        u8g.print("РУЧНОЙ РЕЖИМ");

#ifdef USE_DIVERSITY
        u8g.setColorIndex(menu_id == 3 ? BLACK : WHITE);
        u8g.setPrintPos(5,10*3+12);
        u8g.print("ПАРА");
#endif
        u8g.setColorIndex(menu_id == 4 ? BLACK : WHITE);
        u8g.setPrintPos(5,10*4+12);
        u8g.print("СОХРАНИТЬ");
    } while( u8g.nextPage() );
}

void screens::seekMode(uint8_t state) {
    last_channel = -1;
    reset(); // start from fresh screen.
}

void screens::updateSeekMode(uint8_t state, uint8_t channelIndex, uint8_t channel, uint8_t rssi, uint16_t channelFrequency, bool locked) {

    last_channel = channel;
}

void screens::bandScanMode(uint8_t state) {
    reset(); // start from fresh screen.
    best_rssi = 0;
    if(state==STATE_SCAN)
    {
        drawTitleBox("СКАНЕР КАНАЛА");
    }
    else
    {
        drawTitleBox("RSSI НАСТРОЙКА");
    }
}

void screens::updateBandScanMode(bool in_setup, uint8_t channel, uint8_t rssi, uint8_t channelName, uint16_t channelFrequency, uint16_t rssi_setup_min_a, uint16_t rssi_setup_max_a) {
    static uint8_t writePos=SCANNER_LIST_X_POS;
    last_channel = channel;
}

void screens::screenSaver(uint8_t channelName, uint16_t channelFrequency) {
    screenSaver(-1, channelName, channelFrequency);
}
void screens::screenSaver(uint8_t diversity_mode, uint8_t channelName, uint16_t channelFrequency) {

}

void screens::updateScreenSaver(uint8_t rssi) {
    updateScreenSaver(-1, rssi, -1, -1);
}
void screens::updateScreenSaver(char active_receiver, uint8_t rssi, uint8_t rssiA, uint8_t rssiB) {

}

#ifdef USE_DIVERSITY
void screens::diversity(uint8_t diversity_mode) {
    reset();
    drawTitleBox("ПАРА");
}
void screens::updateDiversity(char active_receiver, uint8_t rssiA, uint8_t rssiB){

}
#endif

void screens::save(uint8_t mode, uint8_t channelIndex, uint16_t channelFrequency) {
    reset();
    drawTitleBox("СОХРАНИТЬ");
}

void screens::updateSave(const char * msg) {
}
#endif
