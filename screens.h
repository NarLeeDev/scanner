#ifndef display_h
#define display_h


#include <pgmspace.h>// stdint.h
#include <stdint.h>
//Each screen has the following
// public entry method
// private draw method
// public update method
// private update draw method

class screens
{
    private: // helper functions for screens.
        uint8_t last_rssi;
        uint8_t best_rssi;
        uint8_t last_channel;
        void reset();
        void drawTitleBox(const char *title);

    public:
        screens();
        char begin(const char *call_sign);
        void flip();

        // MAIN MENU
        void mainMenu(uint8_t menu_id);

        // SEEK & MANUAL MODE
        void seekMode(uint8_t state); // seek and manual mode
        void updateSeekMode(uint8_t state, uint8_t channelIndex, uint8_t channel, uint8_t rssi, uint16_t channelFrequency, uint8_t rssi_seek_threshold, bool locked); // seek and manual mode

        // BAND SCAN
        void bandScanMode(uint8_t state);
        void updateBandScanMode(bool in_setup, uint8_t channel, uint8_t rssi, uint8_t channelName, uint16_t channelFrequency, uint16_t rssi_setup_min_a, uint16_t rssi_setup_max_a);

        // SCREEN SAVER
        void screenSaver(uint8_t channelName, uint16_t channelFrequency, const char *call_sign);
        void screenSaver(uint8_t diversity_mode, uint8_t channelName, uint16_t channelFrequency, const char *call_sign);
        void updateScreenSaver(uint8_t rssi);
        void updateScreenSaver(char active_receiver, uint8_t rssi, uint8_t rssiA, uint8_t rssiB); // diversity

        // DIVERSITY
        void diversity(uint8_t diversity_mode);
        void updateDiversity(char active_receiver, uint8_t rssiA, uint8_t rssiB);

        // VOLTAGE
        void voltageAlert();
        void updateSceenSaverVoltage(uint8_t voltage, bool isAlert);          

        // SETUP MENU
        void setupMenu();
        void updateSetupMenu(uint8_t menu_id,bool settings_beeps,bool settings_orderby_channel, const char *call_sign, uint8_t voltage_alert, char editing);

        // SAVE
        void save(uint8_t mode, uint8_t channelIndex, uint16_t channelFrequency, const char *call_sign);
        void updateSave(const char *msg);
};
#endif
