# OFM-OneWireModule – Agent Instructions

## Project

**OFM-OneWireModule** is an **OpenKNX Function Module** that connects the 1-Wire sensor
stack from OGM-SensorDevices to the KNX bus. It is **not self-contained** — it is always
pulled in as a dependency of a concrete device project (OAM) and requires a matching
ETS application generated from `WireGateway.templ.xml`.

> The entire module is guarded by `#ifdef WIREMODULE` — it is conditionally compiled
> in only when the device project defines this symbol.

- **Language**: C++ (Arduino framework)
- **Experience level**: Advanced — assume knowledge of Arduino, RP2040/ESP32, KNX, and
  OpenKNX fundamentals.
- **Branch convention**: `v1` = stable, `v1dev` = active development.
- **All source files live in `src/`**.

## Architecture

### Key Build Constants

| Constant | Meaning |
|---|---|
| `COUNT_1WIRE_CHANNEL` | Total channel slots = `COUNT_1WIRE_BUSMASTER * 30` |
| `COUNT_1WIRE_BUSMASTER` | Number of DS2482/DS2484 bus masters (1, 2, or 3) |

`COUNT_1WIRE_CHANNEL` is always a multiple of 30 — this is a fixed design constraint, not a
coincidence. The KO range check in `processInputKo` relies on it.

### Module Class: `WireGateway` (`src/WireGateway.h/.cpp`)

Extends `OpenKNX::Module`. Singleton: `openknxWireGateway`.

**Key members:**

| Member | Type | Purpose |
|---|---|---|
| `mDevice[]` | `WireDevice*[COUNT_1WIRE_CHANNEL]` | Per-channel device wrappers (nullptr = not configured) |
| `mBusMaster[]` | `OneWireDS2482*[COUNT_1WIRE_BUSMASTER]` | DS2482/DS2484 bus masters |
| `sUnknownDevice[]` | `WireDevice*[COUNT_1WIRE_CHANNEL]` | Devices found on bus but not in ETS config |
| `sUnknownDeviceLast` | `uint8_t` | End index of sUnknownDevice list |

**Lifecycle:**
- `setup()`: initialises I²C pins (RP2040-specific), calls `OneWireDS2482::setup()` and
  `setupTiming()` for busmaster 0, then `setupChannels()`.
- `setupChannels()`: iterates all channel slots; for each slot with `WIRE_sFamilyCode > 0`,
  allocates a `WireDevice`.
- `loop()`: processes one `WireDevice` per tick (round-robin via `_channelIterator`
  with `openknx.freeLoopTime()` guard), then calls `loop()` on all active bus masters,
  then `processUnknownDevices()`.
- `processInputKo()`: handles KNX group writes to IO device KOs (DS2408/DS2413 only —
  sensor KOs are read-only). **Always null-checks `mDevice[lDeviceIndex]`** before calling
  `isIO()` since not every channel slot is populated.
- `processNewIdCallback()` *(static)*: called by the bus master when a device is found
  on the bus that was not in the ETS config. Adds it to `sUnknownDevice[]`.
- `processUnknownDevices()`: sends new device IDs via KO `WIRE_KoNewId` (DPT String,
  max one per 2 s, then 60 s cycle).

**BusMasterCount safety:**
In `loop()`, `lNumBusmaster` is read from ETS params each tick and clamped to
`[1 .. COUNT_1WIRE_BUSMASTER]` before indexing `mBusMaster[]`.

### Channel Class: `WireDevice` (`src/WireDevice.h/.cpp`)

Extends `OpenKNX::Channel`. One instance per configured ETS channel slot.

**Constructor** `WireDevice(uint8_t iDeviceIndex, OneWireDS2482* iBusMaster[])`:
- Calls `OneWire::factory()` with the 7-byte device ID from ETS params to create or
  reuse the matching sensor object in OGM-SensorDevices.
- Assigns the sensor to the selected bus master via `addSensor()`.
- Calls `setDeviceParameter()` to push model function, IO mask etc. into the sensor.

**`loop()`** → calls `processOneWire(false)`.

**`processOneWire()`**: dispatches by device family:
- DS18B20 / DS18S20 / DS2438 → `processSensor()` (value + send logic)
- DS1990 (iButton) → direct state comparison, sends DPT_Bool
- DS2408 / DS2413 → reads current IO byte, sends DPT_Value_1_Ucount or DPT_Bool

**`processSensor()`**: generic sensor value loop — handles smoothing (div-by-zero guard
on smooth parameter), relative/absolute delta, send cycle. Uses `DPT_Value_Lux` (DPT9)
for all sensor types — this is intentional (DPT9 is assignment-compatible); a full
per-type DPT mapping is deferred to a later refactoring.

**`setValue()`** / **`getValue()`**: delegate to `mOneWire->setValue()` / `->getValue()`.

## ETS Parameter Layout

Parameters are accessed via:
```cpp
mDeviceIndex * WIRE_ParamBlockSize + WIRE_ParamBlockOffset + WIRE_s<ParameterName>
```

Key per-channel parameter offsets (all `WIRE_s*` macros):

| Macro | Content |
|---|---|
| `WIRE_sFamilyCode` | Device family byte (0 = not configured) |
| `WIRE_sDeviceId` | 7-byte ROM ID |
| `WIRE_sModelFunction` | Model function selector (see OneWire `ModelFunction_*` constants) |
| `WIRE_sBusMasterSelect1` | Busmaster assignment (1-based, 0 treated as 1) |
| `WIRE_sIoBitmask0` | IO direction mask (DS2408/DS2413) |
| `WIRE_sIoInvertBitmask0` | IO invert mask |
| `WIRE_sSensorSmooth` | Smoothing divisor (0 = no smoothing, guard in place) |
| `WIRE_sSensorOffset` | Signed offset |
| `WIRE_sSensorDelayBase` | Send cycle base (word, via `paramDelay()`) |
| `WIRE_sSensorDeltaPercent` | Relative delta threshold |
| `WIRE_sSensorDeltaAbs` | Absolute delta threshold |
| `WIRE_siButtonSendStatus` | Send iButton state on startup |

Global parameters: `WIRE_BusMasterCount`, `WIRE_IdSearch`, per-busmaster timing:
`WIRE_Busmaster1RSTL`, `WIRE_Busmaster1MSP`, `WIRE_Busmaster1W0L`, `WIRE_Busmaster1REC0`,
`WIRE_Busmaster1WPU`.

## KO Structure

| KO | Content |
|---|---|
| `WIRE_KoOffset + deviceIndex` | Per-channel sensor value or IO state |
| `WIRE_KoNewId` | New device ID string (DPT String, 14 hex chars) |

## Multi-Busmaster State

`COUNT_1WIRE_BUSMASTER > 1` and `COUNT_1WIRE_BUSMASTER > 2` blocks in `WireGateway.cpp`
are syntactically correct (compile without errors) but **not functionally implemented**.
The second and third busmasters are allocated and their `loop()` is called, but:
- `setupTiming()` is not called for them (they run at DS2482 chip defaults)
- no dedicated ETS timing parameters exist for busmasters 2/3 yet

This is a known open item, not a bug.

## Design Decisions

- **DPT_Value_Lux for all sensor values**: DPT9 is assignment-compatible with DPT_Value_Temp
  etc. Full per-type DPT mapping is planned but deferred.
- **iButton Error state ignored**: In `OneWireDS2482::ProcessIButton()`, the iButton
  search overwrites any Error state unconditionally. This is intentional — transient
  bus errors should not block iButton detection.
- **sUnknownDeviceCount**: declared but always 0 (never incremented); the duplicate-check
  loop in `processNewIdCallback` always starts from index 0. This is a known dead
  variable (minor issue, not a bug).

## Dependencies

- **OGM-Common** (`../OGM-Common`) — module system, KNX stack, logging, flash
- **OGM-SensorDevices** (`../OGM-SensorDevices`) — OneWire device stack
  (`OneWireDS2482`, `OneWire`, `OneWireDS18B20`, `OneWireDS1990`, `OneWireDS2438`,
  `OneWireDS2408`, `OneWireDS2413`, `OneWireDS2438Fromula`, `OneWireSearch`)

## Code Conventions

Identical to OGM-Common (see `../OGM-Common/AGENTS.md`):
- No `delay()` — all timing via `delayCheck(timestamp, ms)`
- Allman-style braces, 4-space indent
- Comments in German or English (mixed is OK)

## References

- [`src/WireGateway.cpp`](src/WireGateway.cpp) — module entry point
- [`src/WireDevice.cpp`](src/WireDevice.cpp) — channel implementation
- [`src/WireGateway.templ.xml`](src/WireGateway.templ.xml) — ETS channel template
- [`src/WireGateway.share.xml`](src/WireGateway.share.xml) — shared ETS parameters
- [`../OGM-SensorDevices/AGENTS.md`](../OGM-SensorDevices/AGENTS.md) — OneWire device stack
- [`../OGM-Common/AGENTS.md`](../OGM-Common/AGENTS.md) — base framework
