#ifdef WIREMODULE
    #include "WireGateway.h"
    #include "KnxHelper.h"
    // #include "Logic.h"
    #include "ModuleVersionCheck.h"
    #include "OneWire.h"
    #include "Sensor.h"
    #include <OneWireDS2482.h>

uint8_t WireGateway::sUnknownDeviceCount = 0;
uint8_t WireGateway::sUnknownDeviceIndex = 0;
WireDevice *WireGateway::sUnknownDevice[COUNT_1WIRE_CHANNEL] = {0};

// uint8_t WireGateway::sUnknownDeviceFirst = 0;
uint8_t WireGateway::sUnknownDeviceLast = 0;
uint32_t WireGateway::sUnknownDeviceDelay = 0;
uint8_t WireGateway::sUnknownDeviceDelaySeconds = 60;

WireGateway openknxWireGateway;

// bool WireGateway::sForceSensorRead = false;
// uint32_t WireGateway::sKnxLoopCallbackDelay = 0;

WireGateway::WireGateway() {}

const std::string WireGateway::name()
{
    return "OneWireGateway";
}

const std::string WireGateway::version()
{
    return MODULE_OneWireModule_Version;
}

void WireGateway::setup(bool configured)
{
    if (configured && ParamBASE_ModuleEnabled_WIRE)
    {
        // should we search for new devices?
        bool lSearchNewDevices = knx.paramByte(WIRE_IdSearch) & WIRE_IdSearchMask;
        // are there iButtons?
        // uint8_t lIsIButton = 0;
        #ifdef ARDUINO_ARCH_RP2040
            #ifdef ONEWIRE_5V_ENABLE
                pinMode(ONEWIRE_5V_ENABLE, OUTPUT);
                digitalWrite(ONEWIRE_5V_ENABLE, HIGH);
            #endif    
            #ifdef OPENKNX_HTA_1WIRE_SLPZ_PIN
                pinMode(OPENKNX_HTA_1WIRE_SLPZ_PIN, OUTPUT);
                digitalWrite(OPENKNX_HTA_1WIRE_SLPZ_PIN, HIGH);
            #endif
            #ifndef I2C_BUS_1WIRE
                #define I2C_BUS_1WIRE Wire
            #endif
            #ifdef I2C_SDA_1WIRE_PIN
                #ifdef I2C_SCL_1WIRE_PIN
                I2C_BUS_1WIRE.setSDA(I2C_SDA_1WIRE_PIN);
                I2C_BUS_1WIRE.setSCL(I2C_SCL_1WIRE_PIN);
                #endif
            #endif
        #endif
        
        I2C_BUS_1WIRE.setClock(400000);
        I2C_BUS_1WIRE.begin();

        mBusMaster[0] = new OneWireDS2482(WireGateway::processNewIdCallback, nullptr);
        mBusMaster[0]->setup(0, 0, lSearchNewDevices, I2C_BUS_1WIRE);
    #if COUNT_1WIRE_BUSMASTER > 1
        uint8_t lNumBusmaster = (knx.paramByte(WIRE_BusMasterCount) & WIRE_BusMasterCountMask) >> WIRE_BusMasterCountShift;
        if (lNumBusmaster > 1)
        {
            gBusMaster[1] = new OneWireDS2482(WireDevice::processNewIdCallback, WireDevice::knxLoopCallback);
            gBusMaster[1]->setup(1, 3, lSearchNewDevices);
        #if COUNT_1WIRE_BUSMASTER > 2
            if (lNumBusmaster > 2)
            {
                gBusMaster[2] = new OneWireDS2482(WireDevice::processNewIdCallback, WireDevice::knxLoopCallback);
                gBusMaster[2]->setup(2, 2, lSearchNewDevices);
            }
        }
        #endif
    #endif
        setupChannels();
    }
}

void WireGateway::setupChannels()
{
    // initialize all known 1-Wire-sensors from application data
    for (uint8_t lDeviceIndex = 0; lDeviceIndex < COUNT_1WIRE_CHANNEL; lDeviceIndex++)
    {
        // check for family information
        uint8_t lFamily = knx.paramByte(lDeviceIndex * WIRE_ParamBlockSize + WIRE_ParamBlockOffset + WIRE_sFamilyCode);
        if (lFamily > 0)
        {
            // WireDevice *lDevice = new WireDevice(lDeviceIndex, gBusMaster);
            mDevice[lDeviceIndex] = new WireDevice(lDeviceIndex, mBusMaster);
        }
    }
}

void WireGateway::loop(bool configured)
{
    if (!configured)
        return;

    if (!ParamBASE_ModuleEnabled_WIRE)
        return;

    mIsRunning = true;

    // at this point startup-delay is done
    // we process heartbeat
    // ProcessReadRequests();
    // counter for processed channels in one loop
    uint8_t channelProcessed = 0;

    // skip when the free time has been used up or all channels have already been processed once.
    while (channelProcessed < COUNT_1WIRE_CHANNEL && openknx.freeLoopTime())
    {
        if (_channelIterator >= WIRE_ChannelCount) _channelIterator = 0;
            if (mDevice[_channelIterator] != nullptr)
                mDevice[_channelIterator]->loop();
        _channelIterator++;
        channelProcessed++;
    }

    uint8_t lNumBusmaster = (knx.paramByte(WIRE_BusMasterCount) & WIRE_BusMasterCountMask) >> WIRE_BusMasterCountShift;
    if (lNumBusmaster == 0) lNumBusmaster = 1; // at least one busmaster is always there
    for (uint8_t lBusmasterIndex = 0; lBusmasterIndex < lNumBusmaster; lBusmasterIndex++)
    {
        mBusMaster[lBusmasterIndex]->loop();
    }
    processUnknownDevices();	
    // at Startup, we want to send all values immediately
    // ProcessSensors(gRuntimeData.forceSensorRead);
    mForceSensorRead = false;
}

void WireGateway::processInputKo(GroupObject &iKo)
{
    if (!ParamBASE_ModuleEnabled_WIRE)
        return;
        
    // check for 1-Wire-KO
    if (iKo.asap() >= WIRE_KoOffset && iKo.asap() < ((knx.paramByte(WIRE_BusMasterCount) & WIRE_BusMasterCountMask) >> WIRE_BusMasterCountShift) * 30 + WIRE_KoOffset)
    {
        uint8_t lDeviceIndex = iKo.asap() - WIRE_KoOffset;
        // has to be an input KO (for an 1W output device)
        WireDevice *lDevice = mDevice[lDeviceIndex];
        // we have to check this (in case someone writes on a KO of a sensor device)
        if (lDevice->isIO())
        {
            // find correct DPT for KO
            if (lDevice->getModelFunction() == ModelFunction_IoByte)
                lDevice->setValue(iKo.value(getDPT(VAL_DPT_5)));
            else
                lDevice->setValue(iKo.value(getDPT(VAL_DPT_1)));
        }
    }
}

void ProcessReadRequests()
{
    // this method is called after startup delay and executes read requests, which should just happen once after startup
    static bool sCalledProcessReadRequests = false;
    if (!sCalledProcessReadRequests)
    {
        // we go through all IO devices defined as outputs and check for initial read requests
        // WireDevice::processReadRequests();
        sCalledProcessReadRequests = true;
    }
}

// jeder neue Sensor, der erstmals bei der 1W-Suche erkannt wird,
// wird über diesen Callback der Applikation mitgeteilt.
// Hier wird gecheckt, ob er schon in der Liste der neuen Sensoren ist.
// Wenn nicht, wird er dieser Liste zugefügt. Diese Liste
// wird dann jede Minute ausgegeben.
// static
bool WireGateway::processNewIdCallback(OneWire *iOneWire)
{
    bool lResult = false;
    if (sUnknownDeviceLast < sUnknownDeviceCount)
        sUnknownDeviceLast = sUnknownDeviceCount;
    for (uint8_t lIndex = sUnknownDeviceCount; lIndex < sUnknownDeviceLast; lIndex++)
    {
        OneWire *lSensor = sUnknownDevice[lIndex]->mOneWire;
        if (equalId(iOneWire->Id(), lSensor->Id()))
        {
            lResult = true;
            break;
        }
    }
    if (!lResult && sUnknownDeviceLast < COUNT_1WIRE_CHANNEL)
    {
        // new sensor found, we add it to unknown device list
        sUnknownDevice[sUnknownDeviceLast] = new WireDevice();
        sUnknownDevice[sUnknownDeviceLast++]->mOneWire = iOneWire;
        // trigger send new sensor info
        sUnknownDeviceDelay = millis() - 58000; // start output in 2 seconds
    }
    return lResult;
}

void WireGateway::processUnknownDevices()
{
    bool lForce = sUnknownDeviceDelay == 0;

    if (lForce || delayCheck(sUnknownDeviceDelay, sUnknownDeviceDelaySeconds * 1000))
    {
        // if (sUnknownDeviceIndex < mDeviceCount)
        //     sUnknownDeviceIndex = sDeviceCount;
        if (sUnknownDeviceIndex < sUnknownDeviceLast)
        {
            OneWire *lSensor = sUnknownDevice[sUnknownDeviceIndex++]->mOneWire;
            if (lSensor->Mode() == OneWire::New)
            {
                // output is 1 new ID in 2 Seconds at max
                char lBuffer[15] = {0};
                sprintf(lBuffer, "%02X%02X%02X%02X%02X%02X%02X", lSensor->Id()[0], lSensor->Id()[1], lSensor->Id()[2], lSensor->Id()[3], lSensor->Id()[4], lSensor->Id()[5], lSensor->Id()[6]);
                logDebugP("KO%d sendet Wert: %s", WIRE_KoNewId, lBuffer);
                knx.getGroupObject(WIRE_KoNewId).value(lBuffer, getDPT(VAL_DPT_16));
                sUnknownDeviceDelaySeconds = 2; // check in 2 Seconds for next new ID
            }
        }
        if (sUnknownDeviceIndex >= sUnknownDeviceLast)
        {
            sUnknownDeviceIndex = 0;
            sUnknownDeviceDelaySeconds = 60; // next output of all IDs in a minute
        }
        sUnknownDeviceDelay = delayTimerInit();
    }
}


// static - this is not perfect, but it works
bool WireGateway::measureOneWire(MeasureType iMeasureType, float &eValue)
{
    // eValue = mDevice[sDeviceIndex]->getValue();
    return true;
}

#endif
