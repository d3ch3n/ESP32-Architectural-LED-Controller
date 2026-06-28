#pragma once

class Application
{
public:
    void begin();
    void update();

private:
    void initializeStorage();
    void initializeHardware();
    void initializeNetwork();
};

extern Application g_application;