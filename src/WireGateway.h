// #pragma once
#ifdef WIREMODULE
    #include "OpenKNX.h"
    #include "WireDevice.h"

typedef bool (*getSensorValue)(MeasureType, float &);

class WireGateway : public OpenKNX::Module
{
  private:
    // unknown device processing
    static uint8_t sUnknownDeviceCount;                     // max. device index to process during runtime
    static uint8_t sUnknownDeviceIndex;                     // iterator for global device processing
    static WireDevice *sUnknownDevice[COUNT_1WIRE_CHANNEL]; // list of all used devices across all BM

    // static uint8_t sUnknownDeviceFirst;
    static uint8_t sUnknownDeviceLast;
    static uint32_t sUnknownDeviceDelay;
    static uint8_t sUnknownDeviceDelaySeconds;

    uint8_t _channelIterator = 0;
    WireDevice *mDevice[COUNT_1WIRE_CHANNEL];
    OneWireDS2482 *mBusMaster[COUNT_1WIRE_BUSMASTER];

    bool mIsRunning = false;
    bool mForceSensorRead = true;

    static bool processNewIdCallback(OneWire *iOneWire);
    static bool measureOneWire(MeasureType iMeasureType, float &eValue);

    // void setupCustomFlash();
    void setupChannels();

  public:
    WireGateway();
    void loop(bool configured) override;
    void setup(bool configured) override;
    const std::string name() override;
    const std::string version() override;
    void processInputKo(GroupObject &ko) override;
    // bool processCommand(const std::string cmd, bool diagnoseKo);
    // void showHelp() override;
};
#endif