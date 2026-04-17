/***************************************************************************//**
 * @file
 * @brief Zigbee-only UI (MemLCD) rendering
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************/

#include <string.h>
#include <stdio.h>

#include "sl_zigbee_ui.h"
#include "sl_zigbee_bitmaps.h"
#include "sl_board_control.h"
#include "glib.h"
#include "dmd.h"
#include "app/framework/include/af.h"    // Zigbee AF APIs/events
#include "zigbee_device_config.h"

// --------- GLIB context (owned here) ----------
static GLIB_Context_t sl_glibContext;

// Hex nibble -> ASCII LUT (local to avoid any global collisions)
static const uint8_t s_ascii_lut[16] =
{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

// ---------- Help text ----------
#define helpmenu_line1           "      **HELP**       "
#define helpmenu_line6           " Press>3s: Leave NWK "
#define helpmenu_line4_light     " No NWK : Form NWK   "
#define helpmenu_line5_light     " NWK    : Permit join"
#define helpmenu_line4_lightSed  " No NWK : Join NWK   "
#define helpmenu_line5_lightSed  " NWK    : Identify   "

#define TMP_STR_LEN   9
#define APP_NAME_LEN  20
#define DEV_NAME_LEN  20

// Permit-join / periodic UI event
static sl_zigbee_af_event_t lcdPermitJoinEvent;
#define permitJoinEvent (&lcdPermitJoinEvent)

// Local UI state
static sl_zigbee_ui_light_state_t light1State = UI_LIGHT_OFF;
static sl_zigbee_ui_light_state_t light2State = UI_LIGHT_OFF;
static bool  helpMenuDisplayed = false;
static bool  panIdDisplayToggle = false;
static bool  blockPanIdDisplay  = false;
static uint8_t eventTimeLeft    = 0;
static char  deviceName[DEV_NAME_LEN] = "Zigbee";

// Forward
static void zigbeeUiUpdateStatus(sl_zigbee_ui_zigbee_network_state_t nwState, bool updateDisplay);
static void zigbeeUiDisplayLogo(void);
static void zigbeeUiDisplayZigbeeLogo(void);
static void zigbeeUiDisplayAppName(const char *device);
static void zigbeeUiRedrawMainScreen(void);
static void lcdPermitJoinEventHandler(sl_zigbee_af_event_t * event);

// ------------------- Small helpers -------------------
static void zigbeeUiDisplayLogo(void)
{
  GLIB_drawBitmap(&sl_glibContext,
                  SILICONLABS_X_POSITION,
                  SILICONLABS_Y_POSITION,
                  SILICONLABS_BITMAP_WIDTH,
                  SILICONLABS_BITMAP_HEIGHT,
                  siliconlabsBitmap);      // header graphic
}

static void zigbeeUiDisplayAppName(const char *device)
{
  char appName[APP_NAME_LEN] = "Zigbee App ";
  const size_t base = strlen("Zigbee App ");
  appName[APP_NAME_LEN - 1] = '\0';

  if (device && device[0]) {
    strncpy(&appName[base], device, (APP_NAME_LEN - 1) - base);
    if (appName[APP_NAME_LEN - 1] != '\0') {
      appName[APP_NAME_LEN - 1] = '\0';
    }
  }

  GLIB_drawString(&sl_glibContext,
                  appName,
                  (int)strlen(appName) + 1,
                  20,
                  SILICONLABS_BITMAP_HEIGHT + 7,
                  0);
}

static void zigbeeUiDisplayZigbeeLogo(void)
{
  GLIB_drawBitmap(&sl_glibContext,
                  ZIGBEE_X_POSITION,
                  ZIGBEE_Y_POSITION,
                  ZIGBEE_BITMAP_WIDTH,
                  ZIGBEE_BITMAP_HEIGHT,
                  zigbeeBitmap);

  if (!blockPanIdDisplay) {
    zigbeeUiUpdateStatus(UI_STATE_UNKNOWN, false);
  }
}

static void zigbeeUiUpdateStatus(sl_zigbee_ui_zigbee_network_state_t nwState,
                                 bool updateDisplay)
{
  char tempStr[TMP_STR_LEN] = { 0 };
  const char *text = tempStr;

  if (helpMenuDisplayed) {
    return;
  }

  sl_802154_pan_id_t panId = sl_zigbee_af_get_pan_id();

  if (nwState == UI_STATE_UNKNOWN) {
    if (panId == 0xFFFF) {
      nwState = UI_NO_NETWORK;
    } else {
      sl_zigbee_network_status_t s = sl_zigbee_af_network_state();
      if (s == SL_ZIGBEE_JOINED_NETWORK) {
        nwState = UI_NETWORK_UP;
      } else if (s == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
        nwState = UI_LOST_NETWORK;
      } else {
        nwState = UI_NO_NETWORK;
      }
    }
  }
  tempStr[TMP_STR_LEN - 1] = '\0';

  switch (nwState) {
    case UI_NO_NETWORK:    text = "No Nwk";   break;
    case UI_LOST_NETWORK:  text = "Lost Nwk"; break;
    case UI_SCANNING:      text = "Scanning"; break;
    case UI_JOINING:       text = "Joining";  break;
    case UI_FORMING:       text = "Forming";  break;
    case UI_DISCOVERING:   text = "Discvrng"; break;

    case UI_NETWORK_UP:
      // Compose PAN:XXXX
      tempStr[0] = 'P'; tempStr[1] = 'A'; tempStr[2] = 'N'; tempStr[3] = ':';
      tempStr[4] = s_ascii_lut[(panId & 0xF000) >> 12];
      tempStr[5] = s_ascii_lut[(panId & 0x0F00) >>  8];
      tempStr[6] = s_ascii_lut[(panId & 0x00F0) >>  4];
      tempStr[7] = s_ascii_lut[panId & 0x000F];
      tempStr[8] = '\0';
      break;

    default: break;
  }

  GLIB_drawString(&sl_glibContext, text,
                  (int)strlen(text) + 1,
                  2,
                  (int)sl_glibContext.pDisplayGeometry->ySize - 10,
                  0);

  if (updateDisplay) {
    DMD_updateDisplay();
  }
}

// ------------------- Public API -------------------
void sl_zigbee_ui_init(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
      sl_zigbee_af_event_init(permitJoinEvent, lcdPermitJoinEventHandler);
      break;

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
      light1State = UI_LIGHT_OFF;
      light2State = UI_LIGHT_OFF;
      deviceName[sizeof(deviceName) - 1] = '\0';
      if (SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE <= SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER) {
        strncpy(deviceName, "Light", sizeof(deviceName));
      } else if (SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE
                 == SLI_ZIGBEE_NETWORK_DEVICE_TYPE_SLEEPY_END_DEVICE) {
        strncpy(deviceName, "LightSed", sizeof(deviceName));
      } else {
        strncpy(deviceName, "Zigbee", sizeof(deviceName));
      }
      if (deviceName[sizeof(deviceName) - 1] != '\0') {
        deviceName[sizeof(deviceName) - 1] = '\0';
      }
      break;

    case SL_ZIGBEE_INIT_LEVEL_DONE: {
      uint32_t st;

      st = sl_board_enable_display();               // power/display mux
      EFM_ASSERT(st == SL_STATUS_OK);

      st = DMD_init(0);                             // LCD controller
      EFM_ASSERT(st == DMD_OK);

      st = GLIB_contextInit(&sl_glibContext);         // GLIB context
      EFM_ASSERT(st == GLIB_OK);

      sl_glibContext.backgroundColor = White;
      sl_glibContext.foregroundColor = Black;
      GLIB_clear(&sl_glibContext);
      GLIB_Font_t fontNarrow6x8 = GLIB_FontNarrow6x8;
      (void)GLIB_setFont(&sl_glibContext, &fontNarrow6x8);

      sl_zigbee_ui_display_help();                 // first screen
      zigbeeUiDisplayZigbeeLogo();
      DMD_updateDisplay();
      break;
    }
    default:
      break;
  }
}

void sl_zigbee_ui_light_on(void)
{
  helpMenuDisplayed = false;
  GLIB_clear(&sl_glibContext);

  light1State = UI_LIGHT_ON;
  GLIB_drawBitmap(&sl_glibContext,
                  LIGHT_X_POSITION, LIGHT_Y_POSITION,
                  LIGHT_BITMAP_WIDTH, LIGHT_BITMAP_HEIGHT,
                  lightOnBitMap);

  zigbeeUiDisplayLogo();
  zigbeeUiDisplayAppName(deviceName);
  zigbeeUiDisplayZigbeeLogo();
  DMD_updateDisplay();
}

void sl_zigbee_ui_light_off(void)
{
  helpMenuDisplayed = false;
  GLIB_clear(&sl_glibContext);

  light1State = UI_LIGHT_OFF;
  GLIB_drawBitmap(&sl_glibContext,
                  LIGHT_X_POSITION, LIGHT_Y_POSITION,
                  LIGHT_BITMAP_WIDTH, LIGHT_BITMAP_HEIGHT,
                  lightOffBitMap);

  zigbeeUiDisplayLogo();
  zigbeeUiDisplayAppName(deviceName);
  zigbeeUiDisplayZigbeeLogo();
  DMD_updateDisplay();
}

void sl_zigbee_ui_light_update(sl_zigbee_ui_light_state_t updateLight1,
                               sl_zigbee_ui_light_state_t updateLight2)
{
  if (updateLight1 != UI_LIGHT_UNCHANGED) {
    light1State = updateLight1;
  }
  if (updateLight2 != UI_LIGHT_UNCHANGED) {
    light2State = updateLight2;
  }

  const uint8_t *pic1 = (light1State == UI_LIGHT_ON) ? lightOnBitMap : lightOffBitMap;
  const uint8_t *pic2 = (light2State == UI_LIGHT_ON) ? lightOnBitMap : lightOffBitMap;

  helpMenuDisplayed = false;
  GLIB_clear(&sl_glibContext);

  GLIB_drawBitmap(&sl_glibContext, 0, LIGHT_Y_POSITION, LIGHT_BITMAP_WIDTH, LIGHT_BITMAP_HEIGHT, pic1);
  GLIB_drawBitmap(&sl_glibContext, 64, LIGHT_Y_POSITION, LIGHT_BITMAP_WIDTH, LIGHT_BITMAP_HEIGHT, pic2);

  zigbeeUiDisplayLogo();
  zigbeeUiDisplayAppName(deviceName);
  zigbeeUiDisplayZigbeeLogo();
  DMD_updateDisplay();
}

void sl_zigbee_ui_display_help(void)
{
  helpMenuDisplayed = true;

  zigbeeUiDisplayLogo();
  zigbeeUiDisplayAppName(deviceName);

  uint8_t y = SILICONLABS_BITMAP_HEIGHT + 20;
  const char *l1 = helpmenu_line1;
  const char *l6 = helpmenu_line6;

  const char *l4 = (SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE
                    <= SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER)
                   ? helpmenu_line4_light
                   : helpmenu_line4_lightSed;

  const char *l5 = (SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE
                    <= SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER)
                   ? helpmenu_line5_light
                   : helpmenu_line5_lightSed;

  GLIB_drawString(&sl_glibContext, l1, (int)strlen(l1) + 1, 2, y +  0, 0);
  GLIB_drawString(&sl_glibContext, l4, (int)strlen(l4) + 1, 2, y + 30, 0);
  GLIB_drawString(&sl_glibContext, l5, (int)strlen(l5) + 1, 2, y + 40, 0);
  GLIB_drawString(&sl_glibContext, l6, (int)strlen(l6) + 1, 2, y + 50, 0);
  DMD_updateDisplay();
}

void sl_zigbee_ui_zigbee_permit_join(bool enable)
{
  eventTimeLeft = enable ? sl_zigbee_get_permit_joining() : 0;
  panIdDisplayToggle = false;
  blockPanIdDisplay  = true;

  if (enable) {
    sl_zigbee_af_event_set_delay_ms(permitJoinEvent, UI_PJOIN_EVENT_DURATION);
  }
}

void sl_zigbee_ui_display_zigbee_state(sl_zigbee_ui_zigbee_network_state_t nwState)
{
  if (helpMenuDisplayed) {
    return;
  }
  bool restore = blockPanIdDisplay;
  blockPanIdDisplay = true;

  zigbeeUiRedrawMainScreen();
  zigbeeUiUpdateStatus(nwState, true);

  blockPanIdDisplay = restore;
}

// ------------------- Internals -------------------
static void zigbeeUiRedrawMainScreen(void)
{
  if (light1State == UI_LIGHT_OFF) {
    sl_zigbee_ui_light_off();
  } else {
    sl_zigbee_ui_light_on();
  }
}

static void lcdPermitJoinEventHandler(sl_zigbee_af_event_t * event)
{
  (void)event;

  if (helpMenuDisplayed) {
    sl_zigbee_af_event_set_delay_ms(permitJoinEvent, UI_PJOIN_EVENT_DURATION);
    return;
  }

  if (SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE <= SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER) {
    eventTimeLeft = sl_zigbee_get_permit_joining();
  }

  zigbeeUiRedrawMainScreen();

  if (panIdDisplayToggle) {
    zigbeeUiUpdateStatus(UI_STATE_UNKNOWN, true);
  }
  panIdDisplayToggle = !panIdDisplayToggle;

  if (eventTimeLeft) {
    sl_zigbee_af_event_set_delay_ms(permitJoinEvent, UI_PJOIN_EVENT_DURATION);
  } else {
    blockPanIdDisplay  = false;
    panIdDisplayToggle = false;
    zigbeeUiUpdateStatus(UI_STATE_UNKNOWN, true);
    sl_zigbee_af_event_set_inactive(permitJoinEvent);
  }
}
