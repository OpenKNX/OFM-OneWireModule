#pragma once
#ifdef WIREMODULE
    #include "OpenKNX.h"
    #include "Sensor.h"
    #include "hardware.h"
    #include "knx.h"
    #include "knxprod.h"
    #include <OneWire.h>

class WireDevice : public OpenKNX::Channel
{
  private:
    uData mData;
    uint8_t mDeviceIndex = 0;

  public:
    WireDevice();
    WireDevice(uint8_t iDeviceIndex, OneWireDS2482 *iBusMaster[]);
    ~WireDevice();

    OneWire *mOneWire = NULL;

    const std::string name() override;
    void setup() override;
    void loop() override;

    // general processing
    void processKOCallback(GroupObject &iKo);
    void processIButtonGroups();
    void processOneWire(bool iForce);
    void processReadRequests();
    bool processNewIdCallback(OneWire *iOneWireSensor);
    void forceSensorRead();

    uint8_t getIndex();
    uint32_t calcParamIndex(uint16_t iParamIndex);
    uint8_t getModelFunction();

    void setValue(uint8_t iValue);
    uint8_t getValue();
    void clearSendDelay();
    // void processOneWire();
    bool processReadRequest();
    bool isIO();
    bool isIButton();
    void setDeviceParameter();
    // void setup(OneWire *iOneWire, uint8_t iModelFunction);
    void processSensor(float iOffsetFactor, uint16_t iParamIndex, uint16_t iKoNumber);
};
#endif
