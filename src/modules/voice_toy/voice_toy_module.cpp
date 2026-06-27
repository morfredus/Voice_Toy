#include "voice_toy_module.h"

#include "../../api/api_router/api_router.h"
#include "../../services/config_manager/config_manager.h"
#include "../../services/log_manager/log_manager.h"
#include "board_config.h"

#include <math.h>

static constexpr i2s_port_t MIC_I2S_PORT = I2S_NUM_0;
static constexpr i2s_port_t AMP_I2S_PORT = I2S_NUM_1;

VoiceToyModule::VoiceToyModule()
    : _display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {
    _muteButton.pin = BUTTON_MUTE_PIN;
    _prevButton.pin = BUTTON_PREV_PIN;
    _nextButton.pin = BUTTON_NEXT_PIN;
}

void VoiceToyModule::begin() {
    setupPins();
    _effect = constrain(config.getInt("voice_effect", EFFECT_ROBOT), 0, EFFECT_COUNT - 1);
    _muted = config.getBool("voice_muted", false);
    updateVolume();

    _displayReady = setupDisplay();
    _audioReady = setupI2S();
    refreshDisplay(true);

    LOG_INFO(name(), "Démarré — effet=%s, volume=%u, mute=%s, audio=%s, oled=%s",
             effectName(_effect), (unsigned)_volume, _muted ? "oui" : "non",
             _audioReady ? "ok" : "erreur",
             _displayReady ? "ok" : "erreur");
}

void VoiceToyModule::loop() {
    updateControls();
    processAudio();
    refreshDisplay();
}

void VoiceToyModule::registerRoutes(WebRouter& router) {
    WebServer& server = router.raw();

    router.get("/api/voice-toy/status", [this, &server]() {
        server.send(200, "application/json", statusJson());
    });

    router.get("/api/voice-toy/next", [this, &server]() {
        changeEffect(1);
        server.send(200, "application/json", statusJson());
    });

    router.get("/api/voice-toy/prev", [this, &server]() {
        changeEffect(-1);
        server.send(200, "application/json", statusJson());
    });

    router.get("/api/voice-toy/toggle", [this, &server]() {
        _muted = !_muted;
        config.setBool("voice_muted", _muted);
        refreshDisplay(true);
        server.send(200, "application/json", statusJson());
    });
}

void VoiceToyModule::setupPins() {
    analogReadResolution(12);
    pinMode(POTENTIOMETER_PIN, INPUT);
    pinMode(_muteButton.pin, INPUT_PULLUP);
    pinMode(_prevButton.pin, INPUT_PULLUP);
    pinMode(_nextButton.pin, INPUT_PULLUP);
}

bool VoiceToyModule::setupDisplay() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        LOG_WARNING(name(), "OLED SSD1306 non détecté à 0x%02X", OLED_ADDRESS);
        return false;
    }
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.display();
    return true;
}

bool VoiceToyModule::setupI2S() {
    i2s_config_t micConfig = {};
    micConfig.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
    micConfig.sample_rate = SAMPLE_RATE;
    micConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
    micConfig.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    micConfig.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    micConfig.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    micConfig.dma_buf_count = 6;
    micConfig.dma_buf_len = AUDIO_FRAMES;
    micConfig.use_apll = false;
    micConfig.tx_desc_auto_clear = false;
    micConfig.fixed_mclk = 0;

    i2s_pin_config_t micPins = {};
    micPins.bck_io_num = I2S_MIC_SCK_PIN;
    micPins.ws_io_num = I2S_MIC_WS_PIN;
    micPins.data_out_num = I2S_PIN_NO_CHANGE;
    micPins.data_in_num = I2S_MIC_SD_PIN;

    i2s_config_t ampConfig = {};
    ampConfig.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    ampConfig.sample_rate = SAMPLE_RATE;
    ampConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    ampConfig.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    ampConfig.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    ampConfig.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    ampConfig.dma_buf_count = 6;
    ampConfig.dma_buf_len = AUDIO_FRAMES;
    ampConfig.use_apll = false;
    ampConfig.tx_desc_auto_clear = true;
    ampConfig.fixed_mclk = 0;

    i2s_pin_config_t ampPins = {};
    ampPins.bck_io_num = I2S_AMP_BCLK_PIN;
    ampPins.ws_io_num = I2S_AMP_LRC_PIN;
    ampPins.data_out_num = I2S_AMP_DIN_PIN;
    ampPins.data_in_num = I2S_PIN_NO_CHANGE;

    esp_err_t err = i2s_driver_install(MIC_I2S_PORT, &micConfig, 0, nullptr);
    if (err != ESP_OK) {
        LOG_ERROR(name(), "i2s_driver_install micro: %d", err);
        return false;
    }
    err = i2s_set_pin(MIC_I2S_PORT, &micPins);
    if (err != ESP_OK) {
        LOG_ERROR(name(), "i2s_set_pin micro: %d", err);
        return false;
    }

    err = i2s_driver_install(AMP_I2S_PORT, &ampConfig, 0, nullptr);
    if (err != ESP_OK) {
        LOG_ERROR(name(), "i2s_driver_install ampli: %d", err);
        return false;
    }
    err = i2s_set_pin(AMP_I2S_PORT, &ampPins);
    if (err != ESP_OK) {
        LOG_ERROR(name(), "i2s_set_pin ampli: %d", err);
        return false;
    }

    i2s_zero_dma_buffer(MIC_I2S_PORT);
    i2s_zero_dma_buffer(AMP_I2S_PORT);
    return true;
}

void VoiceToyModule::updateControls() {
    updateVolume();
    updateButton(_muteButton);
    updateButton(_prevButton);
    updateButton(_nextButton);
}

void VoiceToyModule::updateVolume() {
    int value = analogRead(POTENTIOMETER_PIN);
    if (_lastPotValue < 0) {
        _lastPotValue = value;
        _volume = value;
        return;
    }

    if (abs(value - _lastPotValue) < POT_HYSTERESIS) {
        return;
    }

    _lastPotValue = value;
    _volume = value;
    refreshDisplay(true);
}

void VoiceToyModule::updateButton(ButtonState& button) {
    bool pressed = digitalRead(button.pin) == LOW;
    unsigned long now = millis();

    if (pressed != button.previous) {
        button.previous = pressed;
        button.changedAt = now;
    }

    if (now - button.changedAt < BUTTON_DEBOUNCE_MS) {
        return;
    }

    if (pressed == button.current) {
        return;
    }

    button.current = pressed;
    if (pressed) {
        button.pressedAt = now;
        button.longHandled = false;
    } else {
        handleButtonRelease(button, now - button.pressedAt);
    }
}

void VoiceToyModule::handleButtonRelease(ButtonState& button, unsigned long pressMs) {
    if (button.longHandled || pressMs < BUTTON_DEBOUNCE_MS) {
        return;
    }

    if (button.pin == _muteButton.pin) {
        _muted = !_muted;
        config.setBool("voice_muted", _muted);
        refreshDisplay(true);
        LOG_INFO(name(), "Sortie audio %s", _muted ? "muette" : "active");
    } else if (button.pin == _prevButton.pin) {
        changeEffect(-1);
    } else if (button.pin == _nextButton.pin) {
        changeEffect(1);
    }
}

void VoiceToyModule::setEffect(uint8_t effect) {
    if (effect >= EFFECT_COUNT) return;
    if (_effect == effect) return;

    _effect = effect;
    config.setInt("voice_effect", _effect);
    refreshDisplay(true);
    LOG_INFO(name(), "Mode %s", effectName(_effect));
}

void VoiceToyModule::changeEffect(int8_t delta) {
    int next = (int)_effect + delta;
    if (next < 0) next = EFFECT_COUNT - 1;
    if (next >= EFFECT_COUNT) next = 0;
    setEffect((uint8_t)next);
}

void VoiceToyModule::processAudio() {
    if (!_audioReady) return;

    int32_t micBuffer[AUDIO_FRAMES] = {};
    int16_t outBuffer[AUDIO_FRAMES * 2] = {};
    size_t bytesRead = 0;

    esp_err_t err = i2s_read(MIC_I2S_PORT, micBuffer, sizeof(micBuffer), &bytesRead, 0);
    if (err != ESP_OK || bytesRead == 0) {
        return;
    }

    size_t frames = bytesRead / sizeof(int32_t);
    for (size_t i = 0; i < frames; ++i) {
        int16_t sample = (int16_t)(micBuffer[i] >> 16);
        sample = _muted ? 0 : applyVolume(applyEffect(sample));
        outBuffer[i * 2] = sample;
        outBuffer[i * 2 + 1] = sample;
    }

    size_t bytesWritten = 0;
    i2s_write(AMP_I2S_PORT, outBuffer, frames * 2 * sizeof(int16_t), &bytesWritten, 0);
}

int16_t VoiceToyModule::applyEffect(int16_t sample) {
    _phase++;
    int32_t s = sample;

    switch (_effect) {
        case EFFECT_ROBOT: {
            int16_t carrier = (_phase % 48) < 24 ? 18000 : -18000;
            s = (s * carrier) / 32768;
            s = (s / 900) * 900;
            break;
        }
        case EFFECT_ALIEN: {
            float wobble = sinf(_phase * 0.045f) * 0.55f + sinf(_phase * 0.013f) * 0.25f;
            s = (int32_t)(s * (0.75f + wobble));
            s += (_lastSample * 3) / 10;
            break;
        }
        case EFFECT_VADOR:
            _lowpass = (_lowpass * 7 + s) / 8;
            s = _lowpass * 2;
            if ((_phase & 1) == 0) s = (s + _lastSample) / 2;
            break;
        case EFFECT_ECHO: {
            size_t tap = (_echoIndex + ECHO_BUFFER_SIZE - 2200) % ECHO_BUFFER_SIZE;
            int16_t delayed = _echoBuffer[tap];
            _echoBuffer[_echoIndex] = constrain(s + delayed / 3, -32768, 32767);
            _echoIndex = (_echoIndex + 1) % ECHO_BUFFER_SIZE;
            s = s + delayed / 2;
            break;
        }
        case EFFECT_CHIPMUNK:
            s = (_phase & 1) ? s : _lastSample;
            s = (s * 3) / 2;
            break;
        case EFFECT_RADIO:
            s = (s / 1400) * 1400;
            s = constrain(s * 3, -18000, 18000);
            break;
        case EFFECT_MEGAPHONE:
            s = constrain(s * 5, -22000, 22000);
            s = (s / 700) * 700;
            break;
        case EFFECT_DARK_CAVE: {
            size_t tapA = (_echoIndex + ECHO_BUFFER_SIZE - 2900) % ECHO_BUFFER_SIZE;
            size_t tapB = (_echoIndex + ECHO_BUFFER_SIZE - 3900) % ECHO_BUFFER_SIZE;
            int16_t delayed = (_echoBuffer[tapA] + _echoBuffer[tapB]) / 2;
            _echoBuffer[_echoIndex] = constrain(s + delayed / 2, -32768, 32767);
            _echoIndex = (_echoIndex + 1) % ECHO_BUFFER_SIZE;
            s = (s / 2) + delayed;
            break;
        }
        case EFFECT_DALEK: {
            int16_t carrier = (_phase % 18) < 9 ? 22000 : -22000;
            s = (s * carrier) / 32768;
            s = constrain(s * 2, -26000, 26000);
            break;
        }
        case EFFECT_DEMON:
            _lowpass = (_lowpass * 5 + s) / 6;
            s = constrain((_lowpass * 3) + (_lastSample / 2), -28000, 28000);
            if ((_phase % 5) == 0) s = -s;
            break;
        default:
            break;
    }

    _lastSample = sample;
    return constrain(s, -32768, 32767);
}

int16_t VoiceToyModule::applyVolume(int16_t sample) const {
    int32_t scaled = ((int32_t)sample * _volume) / 4095;
    return constrain(scaled, -32768, 32767);
}

void VoiceToyModule::writeSilence(size_t frames) {
    int16_t outBuffer[AUDIO_FRAMES * 2] = {};
    size_t bytesWritten = 0;
    i2s_write(AMP_I2S_PORT, outBuffer, min(frames, AUDIO_FRAMES) * 2 * sizeof(int16_t), &bytesWritten, 0);
}

void VoiceToyModule::refreshDisplay(bool force) {
    if (!_displayReady) return;
    unsigned long now = millis();
    if (!force && now - _lastDisplayMs < OLED_REFRESH_MS) return;
    _lastDisplayMs = now;

    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(2);
    _display.setCursor(10, 0);
    _display.print("VOICE TOY");

    _display.setTextSize(1);
    _display.setCursor(0, 24);
    _display.print("Mode:");
    _display.setTextSize(2);
    _display.setCursor(0, 36);
    _display.print(effectName(_effect));

    _display.setTextSize(1);
    _display.setCursor(0, 56);
    _display.print(_muted ? "MUTE " : "VOL ");
    _display.print(map(_volume, 0, 4095, 0, 100));
    _display.print("%");
    _display.setCursor(82, 56);
    _display.print("39< 40>");
    _display.display();
}

const char* VoiceToyModule::effectName(uint8_t effect) const {
    static const char* names[] = {
        "ROBOT",
        "ALIEN",
        "VADOR",
        "ECHO",
        "CHIPMUNK",
        "RADIO",
        "MEGAPHONE",
        "DARK CAVE",
        "DALEK",
        "DEMON"
    };
    return effect < EFFECT_COUNT ? names[effect] : "?";
}

String VoiceToyModule::statusJson() const {
    String json = "{";
    json += "\"effect\":" + String(_effect);
    json += ",\"effectName\":\"" + String(effectName(_effect)) + "\"";
    json += ",\"volume\":" + String(_volume);
    json += ",\"volumePercent\":" + String(map(_volume, 0, 4095, 0, 100));
    json += ",\"muted\":" + String(_muted ? "true" : "false");
    json += ",\"audioReady\":" + String(_audioReady ? "true" : "false");
    json += ",\"displayReady\":" + String(_displayReady ? "true" : "false");
    json += "}";
    return json;
}
