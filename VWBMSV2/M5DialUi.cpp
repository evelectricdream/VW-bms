#include "M5DialUi.h"

namespace
{
  const uint16_t UI_BACKGROUND = 0x0000;
  const uint16_t UI_TEXT = 0xFFFF;
  const uint16_t UI_DIM = 0x7BEF;
  const uint16_t UI_ACCENT = 0x051D;
  const uint16_t UI_GOOD = 0x07E0;
  const uint16_t UI_WARN = 0xFD20;
  const uint16_t UI_ALERT = 0xF800;
}

M5DialUi::M5DialUi(BMSModuleManager &manager, uint32_t refreshIntervalMs)
  : bms(manager), refreshInterval(refreshIntervalMs), lastRefresh(0), selectedModule(0), selectedCell(0), currentScreen(SummaryScreen), forceRefresh(true)
#if VW_BMS_HAS_M5DIAL_UI
  , lastRenderedScreen(SummaryScreen)
#endif
{
}

void M5DialUi::begin()
{
#if VW_BMS_HAS_M5DIAL_UI
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setRotation(0);
  M5Dial.Display.setTextSize(2);
  M5Dial.Display.setTextColor(UI_TEXT, UI_BACKGROUND);
  M5Dial.Display.setTextWrap(false);
  M5Dial.Display.fillScreen(UI_BACKGROUND);
  M5Dial.Encoder.setCount(0);
  resetLineCache();
  forceRefresh = true;
#endif
}

void M5DialUi::update()
{
#if VW_BMS_HAS_M5DIAL_UI
  M5Dial.update();
#endif

  ensureModuleSelection();
  handleInput();

  if (!forceRefresh && (millis() - lastRefresh) < refreshInterval)
  {
    return;
  }

  lastRefresh = millis();
  render();
  forceRefresh = false;
}

void M5DialUi::ensureModuleSelection()
{
  if (selectedModule > 0)
  {
    BMSModule *module = bms.getModule(selectedModule);
    if (module != nullptr && module->isExisting())
    {
      return;
    }
  }

  selectedModule = bms.getFirstExistingModule();
}

void M5DialUi::handleInput()
{
#if VW_BMS_HAS_M5DIAL_UI
  int32_t delta = M5Dial.Encoder.getCount();
  if (delta != 0)
  {
    M5Dial.Encoder.setCount(0);
    stepSelection(delta);
    forceRefresh = true;
  }

  if (M5Dial.BtnA.wasPressed())
  {
    cycleScreen();
    forceRefresh = true;
  }
#endif
}

void M5DialUi::cycleScreen()
{
  currentScreen = static_cast<Screen>((currentScreen + 1) % 3);
}

void M5DialUi::stepSelection(int delta)
{
  if (delta == 0)
  {
    return;
  }

  if (currentScreen == CellScreen)
  {
    selectedCell = wrapCellIndex(selectedCell + delta);
    return;
  }

  int direction = (delta > 0) ? 1 : -1;
  int steps = abs(delta);
  if (steps > 8)
  {
    steps = 8;
  }

  for (int i = 0; i < steps; i++)
  {
    selectedModule = bms.getNextExistingModule(selectedModule, direction);
    if (selectedModule == 0)
    {
      break;
    }
  }
}

int M5DialUi::wrapCellIndex(int cellIndex)
{
  while (cellIndex < 0)
  {
    cellIndex += CELL_COUNT;
  }
  return cellIndex % CELL_COUNT;
}

BMSModule *M5DialUi::getSelectedModule()
{
  if (selectedModule < 1)
  {
    return nullptr;
  }

  return bms.getModule(selectedModule);
}

bool M5DialUi::moduleHasData(BMSModule *module)
{
  return module != nullptr && module->hasDecodedData();
}

bool M5DialUi::moduleIsStale(BMSModule *module)
{
  return module != nullptr && module->isExisting() && module->isStale();
}

void M5DialUi::render()
{
#if VW_BMS_HAS_M5DIAL_UI
  if (currentScreen != lastRenderedScreen)
  {
    M5Dial.Display.fillScreen(UI_BACKGROUND);
    resetLineCache();
    lastRenderedScreen = currentScreen;
  }

  BMSModule *module = getSelectedModule();
  if (!moduleHasData(module))
  {
    switch (currentScreen)
    {
      case SummaryScreen:
        renderNoData("Summary");
        break;
      case CellScreen:
        renderNoData("Cell");
        break;
      case TemperatureScreen:
        renderNoData("Temp");
        break;
    }
    return;
  }

  switch (currentScreen)
  {
    case SummaryScreen:
      renderSummary(module);
      break;
    case CellScreen:
      renderCell(module);
      break;
    case TemperatureScreen:
      renderTemperatures(module);
      break;
  }
#endif
}

void M5DialUi::renderSummary(BMSModule *module)
{
#if VW_BMS_HAS_M5DIAL_UI
  String line;
  float minCell = module->getLowCellV();
  float maxCell = module->getHighCellV();

  drawHeader("VW BMS Summary");

  line = String("Module ") + String(module->getAddress());
  drawLine(0, line, UI_ACCENT);

  line = String("Volt ") + String(module->getModuleVoltage(), 3) + "V";
  drawLine(1, line);

  line = String("Min  ") + String(minCell, 3) + "V";
  drawLine(2, line);

  line = String("Max  ") + String(maxCell, 3) + "V";
  drawLine(3, line);

  line = String("dV   ") + String((maxCell - minCell) * 1000.0f, 0) + "mV";
  drawLine(4, line, (moduleIsStale(module) ? UI_WARN : UI_TEXT));

  line = String("T ") + String(module->getTemperature(0), 1) + " " + String(module->getTemperature(1), 1) + " " + String(module->getTemperature(2), 1) + "C";
  drawLine(5, line);

  drawLine(6, moduleIsStale(module) ? "STALE" : "LIVE", moduleIsStale(module) ? UI_WARN : UI_GOOD);
  drawFooter("Turn=module  Press=next");
#endif
}

void M5DialUi::renderCell(BMSModule *module)
{
#if VW_BMS_HAS_M5DIAL_UI
  float voltage = module->getCellVoltage(selectedCell);
  drawHeader("Cell Detail");
  drawLine(0, String("Module ") + String(module->getAddress()), UI_ACCENT);
  drawLine(1, String("Cell ") + String(selectedCell + 1) + "/13");
  drawLine(2, String("Volt ") + String(voltage, 3) + "V");
  drawLine(3, moduleIsStale(module) ? "Status STALE" : "Status LIVE", moduleIsStale(module) ? UI_WARN : UI_GOOD);
  drawLine(4, "");
  drawLine(5, "");
  drawLine(6, String("Min ") + String(module->getLowCellV(), 3) + " Max " + String(module->getHighCellV(), 3), UI_DIM);
  drawCellBar(voltage);
  drawFooter("Turn=cell  Press=next");
#endif
}

void M5DialUi::renderTemperatures(BMSModule *module)
{
#if VW_BMS_HAS_M5DIAL_UI
  drawHeader("Temperatures");
  drawLine(0, String("Module ") + String(module->getAddress()), UI_ACCENT);
  drawLine(1, String("Temp1 ") + String(module->getTemperature(0), 1) + "C");
  drawLine(2, String("Temp2 ") + String(module->getTemperature(1), 1) + "C");
  drawLine(3, String("Temp3 ") + String(module->getTemperature(2), 1) + "C");
  drawLine(4, moduleIsStale(module) ? "Status STALE" : "Status LIVE", moduleIsStale(module) ? UI_WARN : UI_GOOD);
  drawLine(5, "");
  drawLine(6, "");
  drawFooter("Turn=module  Press=next");
#endif
}

void M5DialUi::renderNoData(const char *title)
{
#if VW_BMS_HAS_M5DIAL_UI
  drawHeader(title);
  drawLine(0, selectedModule > 0 ? (String("Module ") + String(selectedModule)) : String("No module selected"), UI_ACCENT);
  drawLine(1, "No data yet", UI_WARN);
  drawLine(2, "Waiting for CAN");
  drawLine(3, "");
  drawLine(4, "");
  drawLine(5, "");
  drawLine(6, "");
  M5Dial.Display.fillRect(20, 164, 200, 38, UI_BACKGROUND);
  drawFooter("Turn=select  Press=next");
#endif
}

#if VW_BMS_HAS_M5DIAL_UI
void M5DialUi::resetLineCache()
{
  for (int i = 0; i < LINE_COUNT; i++)
  {
    lineCache[i] = "";
    colorCache[i] = UI_TEXT;
  }
}

void M5DialUi::drawHeader(const char *title)
{
  M5Dial.Display.fillRect(0, 0, 240, 26, UI_BACKGROUND);
  M5Dial.Display.setCursor(12, 6);
  M5Dial.Display.setTextColor(UI_ACCENT, UI_BACKGROUND);
  M5Dial.Display.print(title);
}

void M5DialUi::drawLine(int line, const String &text, uint16_t color)
{
  if (line < 0 || line >= LINE_COUNT)
  {
    return;
  }

  if (!forceRefresh && lineCache[line] == text && colorCache[line] == color)
  {
    return;
  }

  int y = 32 + (line * 18);
  M5Dial.Display.fillRect(0, y, 240, 18, UI_BACKGROUND);
  M5Dial.Display.setCursor(12, y + 2);
  M5Dial.Display.setTextColor(color, UI_BACKGROUND);
  M5Dial.Display.print(text);
  lineCache[line] = text;
  colorCache[line] = color;
}

void M5DialUi::drawFooter(const char *text, uint16_t color)
{
  M5Dial.Display.fillRect(0, 214, 240, 26, UI_BACKGROUND);
  M5Dial.Display.setCursor(12, 220);
  M5Dial.Display.setTextColor(color, UI_BACKGROUND);
  M5Dial.Display.print(text);
}

void M5DialUi::drawCellBar(float voltage)
{
  uint16_t barColor = UI_GOOD;
  if (voltage < 3.20f)
  {
    barColor = UI_ALERT;
  }
  else if (voltage < 3.50f)
  {
    barColor = UI_WARN;
  }

  float clamped = voltage;
  if (clamped < 2.50f) clamped = 2.50f;
  if (clamped > 4.30f) clamped = 4.30f;

  int width = int(((clamped - 2.50f) / 1.80f) * 180.0f);

  M5Dial.Display.drawRect(20, 164, 200, 20, UI_DIM);
  M5Dial.Display.fillRect(21, 165, 198, 18, UI_BACKGROUND);
  M5Dial.Display.fillRect(21, 165, width, 18, barColor);
}
#endif
