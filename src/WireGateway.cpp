#ifdef WIREMODULE
    #include "WireGateway.h"
    #include "KnxHelper.h"
    // #include "Logic.h"
    #include "ModuleVersionCheck.h"
    #include "OneWire.h"
    #include "Sensor.h"
    #include <OneWireDS2482.h>

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
    if (configured)
    {
        // should we search for new devices?
        bool lSearchNewDevices = knx.paramByte(WIRE_IdSearch) & WIRE_IdSearchMask;
        // are there iButtons?
        // uint8_t lIsIButton = 0;

        Wire.setClock(400000);

        mBusMaster[0] = new OneWireDS2482(WireGateway::processNewIdCallback, nullptr);
        mBusMaster[0]->setup(0, 1, lSearchNewDevices, Wire);
        uint8_t lNumBusmaster = (knx.paramByte(WIRE_BusMasterCount) & WIRE_BusMasterCountMask) >> WIRE_BusMasterCountShift;
    #if COUNT_1WIRE_BUSMASTER > 1
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

    mIsRunning = true;

    // at this point startup-delay is done
    // we process heartbeat
    ProcessReadRequests();
    // counter for processed channels in one loop
    uint8_t channelProcessed = 0;

    // skip when the free time has been used up or all channels have already been processed once.
    while (channelProcessed < COUNT_1WIRE_CHANNEL && openknx.freeLoopTime())
    {
        if (_channelIterator >= WIRE_ChannelCount) _channelIterator = 0;

        mDevice[_channelIterator]->loop();
        _channelIterator++;
        channelProcessed++;
    }

    uint8_t lNumBusmaster = (knx.paramByte(WIRE_BusMasterCount) & WIRE_BusMasterCountMask) >> WIRE_BusMasterCountShift;
    for (uint8_t lBusmasterIndex = 0; lBusmasterIndex < lNumBusmaster; lBusmasterIndex++)
    {
        mBusMaster[lBusmasterIndex]->loop();
    }

    // at Startup, we want to send all values immediately
    // ProcessSensors(gRuntimeData.forceSensorRead);
    mForceSensorRead = false;
}

void WireGateway::processInputKo(GroupObject &iKo)
{
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

#endif
