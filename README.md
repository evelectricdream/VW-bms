# VW-bms
Can based decoding of VW bms data

## M5Dial UI layer

The `VWBMSV2/M5DialUi.*` module adds a read-only M5Dial display layer on top of the existing VW-BMS decoder state.

### Controls

- Rotary turn:
  - Summary / Temperature screens: select the active module
  - Cell screen: select the active cell (1-13)
- Center button press: cycle screens `Summary -> Cell -> Temp -> Summary`

### Screens

- **Summary**: module address, module voltage, min/max cell voltage, delta, three temperatures, and `LIVE` / `STALE` status
- **Cell detail**: selected cell voltage and a simple threshold bar
- **Temperature**: Temp1 / Temp2 / Temp3 for the active module

### Data source

The UI reads existing decoded values from `BMSModule` through `BMSModuleManager` selection accessors and `BMSModule` value getters:

- `BMSModuleManager::getModule()`
- `BMSModuleManager::getFirstExistingModule()`
- `BMSModuleManager::getNextExistingModule()`
- `BMSModule::getCellVoltage()`
- `BMSModule::getLowCellV()` / `getHighCellV()`
- `BMSModule::getModuleVoltage()`
- `BMSModule::getTemperature()`
- `BMSModule::getAddress()`
- `BMSModule::hasDecodedData()` / `isStale()`
