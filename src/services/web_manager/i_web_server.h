#pragma once
class IWebServer {
public:
    virtual ~IWebServer() = default;
    virtual void begin() = 0;
    virtual void loop() = 0;
};
