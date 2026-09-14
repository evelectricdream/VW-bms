#pragma once

#include <Arduino.h>
#include "BMSModuleManager.h"

#if defined(__has_include)
#if __has_include(<M5Dial.h>)
#include <M5Dial.h>
#define VW_BMS_HAS_M5DIAL_UI 1
#else
#define VW_BMS_HAS_M5DIAL_UI 0
#endif
#else
#define VW_BMS_HAS_M5DIAL_UI 0
#endif

class M5DialUi
{
  public:
    explicit M5DialUi(BMSModuleManager &manager, uint32_t refreshIntervalMs = 150);
    void begin();
    void update();

  private:
    enum Screen
    {
      SummaryScreen = 0,
      CellScreen = 1,
      TemperatureScreen = 2
    };

    static const int CELL_COUNT = 13;
    static const int LINE_COUNT = 7;

    BMSModuleManager &bms;
    uint32_t refreshInterval;
    uint32_t lastRefresh;
    int selectedModule;
    int selectedCell;
    Screen currentScreen;
    bool forceRefresh;

#if VW_BMS_HAS_M5DIAL_UI
    Screen lastRenderedScreen;
    String lineCache[LINE_COUNT];
    uint16_t colorCache[LINE_COUNT];
#endif

    void ensureModuleSelection();
    void handleInput();
    void render();
    void renderSummary(BMSModule *module);
    void renderCell(BMSModule *module);
    void renderTemperatures(BMSModule *module);
    void renderNoData(const char *title);
    BMSModule *getSelectedModule();
    bool moduleHasData(BMSModule *module);
    bool moduleIsStale(BMSModule *module);
    void cycleScreen();
    void stepSelection(int delta);
    int wrapCellIndex(int cellIndex);

#if VW_BMS_HAS_M5DIAL_UI
    void resetLineCache();
    void drawHeader(const char *title);
    void drawLine(int line, const String &text, uint16_t color = 0xFFFF);
    void drawFooter(const char *text, uint16_t color = 0x7BEF);
    void drawCellBar(float voltage);
#endif
};
