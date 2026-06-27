#pragma once

#include "../../core/module.h"
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <driver/i2s.h>

class VoiceToyModule : public Module {
public:
    VoiceToyModule();

    const char* name() const override { return "VoiceToy"; }
    void begin() override;
    void loop() override;
    void registerRoutes(WebRouter& router) override;

private:
    enum Effect : uint8_t {
        EFFECT_ROBOT = 0,
        EFFECT_ALIEN,
        EFFECT_VADOR,
        EFFECT_ECHO,
        EFFECT_CHIPMUNK,
        EFFECT_RADIO,
        EFFECT_MEGAPHONE,
        EFFECT_DARK_CAVE,
        EFFECT_DALEK,
        EFFECT_DEMON,
        EFFECT_COUNT
    };

    struct ButtonState {
        uint8_t pin = 0;
        bool current = false;
        bool previous = false;
        unsigned long changedAt = 0;
        unsigned long pressedAt = 0;
        bool longHandled = false;
    };

    static constexpr uint32_t SAMPLE_RATE = 16000;
    static constexpr size_t AUDIO_FRAMES = 64;
    static constexpr size_t ECHO_BUFFER_SIZE = 4096;
    static constexpr uint16_t POT_HYSTERESIS = 24;
    static constexpr unsigned long OLED_REFRESH_MS = 100;
    static constexpr unsigned long BUTTON_DEBOUNCE_MS = 35;

    void setupPins();
    bool setupDisplay();
    bool setupI2S();
    void updateControls();
    void updateVolume();
    void updateButton(ButtonState& button);
    void handleButtonRelease(ButtonState& button, unsigned long pressMs);
    void setEffect(uint8_t effect);
    void changeEffect(int8_t delta);
    void processAudio();
    int16_t applyEffect(int16_t sample);
    int16_t applyVolume(int16_t sample) const;
    void writeSilence(size_t frames);
    void refreshDisplay(bool force = false);
    const char* effectName(uint8_t effect) const;
    String statusJson() const;

    Adafruit_SSD1306 _display;
    bool _displayReady = false;
    bool _audioReady = false;
    bool _muted = false;
    uint8_t _effect = EFFECT_ROBOT;
    int _lastPotValue = -1;
    uint16_t _volume = 2048;
    unsigned long _lastDisplayMs = 0;
    unsigned long _phase = 0;
    int16_t _lastSample = 0;
    int16_t _lowpass = 0;
    int16_t _echoBuffer[ECHO_BUFFER_SIZE] = {};
    size_t _echoIndex = 0;

    ButtonState _muteButton;
    ButtonState _prevButton;
    ButtonState _nextButton;
};
