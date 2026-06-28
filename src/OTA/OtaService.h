#pragma once

class OtaService
{
public:
    void begin();
    void update();

private:
    const char* buildHostname();
};

extern OtaService g_otaService;