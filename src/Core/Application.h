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
    void initializeServices();
};

extern Application g_application;