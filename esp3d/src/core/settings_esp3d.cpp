/*
  settings_esp3d.cpp -  settings esp3d functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "../include/esp3d_config.h"
#if defined(ESP_SAVE_SETTINGS)
#include "esp3doutput.h"
#include "settings_esp3d.h"

#define STRINGIFY(x) #x
#define STRING(x) STRINGIFY(x)

#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
#include <EEPROM.h>
// EEPROM SIZE (Up to 4096)
#define EEPROM_SIZE 2048  // max is 2048
#endif                    // SETTINGS_IN_EEPROM

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../modules/network/netconfig.h"
#if defined(WIFI_FEATURE)
#include "../modules/wifi/wificonfig.h"
#endif  // WIFI_FEATURE
#endif  // WIFI_FEATURE || ETH_FEATURE

#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
#include <Preferences.h>
#define NAMESPACE "ESP3D"
#endif  // SETTINGS_IN_PREFERENCES

// Current Settings Version
#define CURRENT_SETTINGS_VERSION "ESP3D04"

// boundaries
#define MAX_SENSOR_INTERVAL 60000
#define MIN_SENSOR_INTERVAL 0
#define MAX_LOCAL_PASSWORD_LENGTH 20
#define MIN_LOCAL_PASSWORD_LENGTH 1
#define MAX_VERSION_LENGTH 7  // ESP3DXX
#define MAX_BOOT_DELAY 40000
#define MIN_BOOT_DELAY 0
#define MIN_NOTIFICATION_TOKEN_LENGTH 0
#define MIN_NOTIFICATION_SETTINGS_LENGTH 0
#define MAX_NOTIFICATION_TOKEN_LENGTH 63
#define MAX_NOTIFICATION_SETTINGS_LENGTH 128
#define MAX_SERVER_ADDRESS_LENGTH 128
#define MAX_TIME_ZONE_LENGTH 7
#define MIN_SERVER_ADDRESS_LENGTH 0

// default byte values
#ifdef WIFI_FEATURE
#define DEFAULT_STA_FALLBACK_MODE STRING(ESP_AP_SETUP)
#if defined(STATION_WIFI_SSID) && defined(STATION_WIFI_PASSWORD)
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_WIFI_STA)
#else
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_AP_SETUP)
#endif  // STATION_WIFI_SSID && STATION_WIFI_PASSWORD
#else   // WIFI_FEATURE
#define DEFAULT_STA_FALLBACK_MODE STRING(ESP_NO_NETWORK)
#ifdef BLUETOOTH_FEATURE
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_BT)
#else  // BLUETOOTH_FEATURE
#ifdef ETH_FEATURE
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_ETH_STA)
#else  // BLUETOOTH_FEATURE
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_NO_NETWORK)
#endif  // ETH_FEATURE
#endif  // BLUETOOTH_FEATURE
#endif  // WIFI_FEATURE

#define DEFAULT_BUZZER_STATE "1"
#define DEFAULT_INTERNET_TIME "0"
#define DEFAULT_SETUP "0"
#define DEFAULT_VERBOSE_BOOT "0"
#define DEFAULT_STA_IP_MODE "1"
#define DEFAULT_AP_CHANNEL "11"
#define DEFAULT_SSID_VISIBLE "1"
#define DEFAULT_OUTPUT_FLAG "255"
#define DEFAULT_SD_SPI_DIV "4"
#ifndef DEFAULT_FW
#define DEFAULT_FW UNKNOWN_FW
#endif  // DEFAULT_FW
#define DEFAULT_TIME_ZONE "+00:00"
#define DEFAULT_TIME_DST "0"
#define DEFAULT_SD_MOUNT "1"
#define DEFAULT_SD_CHECK_UPDATE_AT_BOOT "1"
#define DEFAULT_SENSOR_TYPE "0"
#define DEFAULT_HTTP_ON "1"
#define DEFAULT_FTP_ON "1"
#define DEFAULT_SERIAL_BRIDGE_ON "1"
#define DEFAULT_WEBDAV_ON "1"
#define DEFAULT_TELNET_ON "1"
#define DEFAULT_WEBSOCKET_ON "1"
#define DEFAULT_NOTIFICATION_TYPE "0"
#define DEFAULT_NOTIFICATION_TOKEN1 ""
#define DEFAULT_NOTIFICATION_TOKEN2 ""
#define DEFAULT_NOTIFICATION_SETTINGS ""
#define DEFAULT_AUTO_NOTIFICATION_STATE "1"
#define DEFAULT_SECURE_SERIAL "1"
#define DEFAULT_BOOT_RADIO_STATE "1"

// default int values
#define DEFAULT_ESP_INT "0"
#define DEFAULT_BAUD_RATE "115200"
#define DEFAULT_SERIAL_BRIDGE_BAUD_RATE "115200"
#define DEFAULT_HTTP_PORT "80"
#define DEFAULT_FTP_CTRL_PORT "21"
#define DEFAULT_FTP_ACTIVE_PORT "20"
#define DEFAULT_FTP_PASSIVE_PORT "55600"
#define DEFAULT_WEBSOCKET_PORT "8282"
#define DEFAULT_WEBDAV_PORT "8181"
#define DEFAULT_TELNET_PORT "23"
#define DEFAULT_SENSOR_INTERVAL "30000"
#define DEFAULT_BOOT_DELAY "10000"
#define DEFAULT_CALIBRATION_VALUE "0"
#define DEFAULT_CALIBRATION_DONE "0"
#define DEFAULT_SESSION_TIMEOUT "3"

// Default flags
#define DEFAULT_SERIAL_OUTPUT_FLAG "1"
#define DEFAULT_REMOTE_SCREEN_FLAG "1"
#define DEFAULT_WEBSOCKET_FLAG "1"
#define DEFAULT_TELNET_FLAG "1"
#define DEFAULT_BT_FLAG "1"
#define DEFAULT_SCREEN_FLAG "1"
#define DEFAULT_SERIAL_BRIDGE_FLAG "1"

#ifdef WIFI_FEATURE
// default string values
#define DEFAULT_AP_SSID "ESP3D"
#define DEFAULT_AP_PASSWORD "12345678"
#if defined(STATION_WIFI_SSID) && defined(STATION_WIFI_PASSWORD)
#define DEFAULT_STA_SSID STATION_WIFI_SSID
#define DEFAULT_STA_PASSWORD STATION_WIFI_PASSWORD
#else
#define DEFAULT_STA_SSID "NETWORK_SSID"
#define DEFAULT_STA_PASSWORD "12345678"
#endif  // STATION_WIFI_SSID && STATION_WIFI_PASSWORD
#endif  // WIFI_FEATURE
#if defined(BLUETOOTH_FEATURE) || defined(WIFI_FEATURE) || defined(ETH_FEATURE)
#define DEFAULT_HOSTNAME "esp3d"
#endif  // BLUETOOTH_FEATURE ||WIFI_FEATURE || ETH_FEATURE

#define DEFAULT_ADMIN_PWD "admin"
#define DEFAULT_USER_PWD "user"

#define DEFAULT_TIME_SERVER1 "time.windows.com"
#define DEFAULT_TIME_SERVER2 "time.google.com"
#define DEFAULT_TIME_SERVER3 "0.pool.ntp.org"

#define DEFAULT_SETTINGS_VERSION "ESP3D"

// default IP values
#define DEFAULT_STA_IP_VALUE "192.168.0.254"
#define DEFAULT_STA_GATEWAY_VALUE DEFAULT_STA_IP_VALUE
#define DEFAULT_STA_MASK_VALUE "255.255.255.0"
#define DEFAULT_STA_DNS_VALUE DEFAULT_STA_IP_VALUE

#define DEFAULT_AP_IP_VALUE "192.1680.0.1"
#define DEFAULT_AP_GATEWAY_VALUE DEFAULT_AP_IP_VALUE
#define DEFAULT_AP_MASK_VALUE "255.255.255.0"
#define DEFAULT_AP_DNS_VALUE DEFAULT_AP_IP_VALUE

uint8_t Settings_ESP3D::_FirmwareTarget = 0;
bool Settings_ESP3D::_isverboseboot = false;

uint16_t ESP3DSettingsData[] = {ESP_SETTINGS_VERSION,
                                ESP_RADIO_MODE,
                                ESP_NOTIFICATION_TYPE,
                                ESP_CALIBRATION,
                                ESP_AP_CHANNEL,
                                ESP_BUZZER,
                                ESP_INTERNET_TIME,
                                ESP_SERIAL_FLAG,
                                ESP_HTTP_ON,
                                ESP_TELNET_ON,
                                ESP_WEBSOCKET_ON,
                                ESP_SD_SPEED_DIV,
                                ESP_SENSOR_TYPE,
                                ESP_TARGET_FW,
                                ESP_TIME_IS_DST,
                                ESP_REMOTE_SCREEN_FLAG,
                                ESP_SD_MOUNT,
                                ESP_SESSION_TIMEOUT,
                                ESP_WEBSOCKET_FLAG,
                                ESP_SD_CHECK_UPDATE_AT_BOOT,
                                ESP_SETUP,
                                ESP_TELNET_FLAG,
                                ESP_BT_FLAG,
                                ESP_SCREEN_FLAG,
                                ESP_FTP_ON,
                                ESP_AUTO_NOTIFICATION,
                                ESP_VERBOSE_BOOT,
                                ESP_WEBDAV_ON,
                                ESP_SECURE_SERIAL,
                                ESP_BOOT_RADIO_STATE,
                                ESP_STA_FALLBACK_MODE,
                                ESP_SERIAL_BRIDGE_FLAG,
                                ESP_SERIAL_BRIDGE_ON,
                                ESP_HOSTNAME,
                                ESP_STA_PASSWORD,
                                ESP_STA_SSID,
                                ESP_ADMIN_PWD,
                                ESP_USER_PWD,
                                ESP_AP_SSID,
                                ESP_AP_PASSWORD,
                                ESP_NOTIFICATION_TOKEN1,
                                ESP_NOTIFICATION_TOKEN2,
                                ESP_NOTIFICATION_SETTINGS,
                                ESP_TIME_SERVER1,
                                ESP_TIME_SERVER2,
                                ESP_TIME_SERVER3,
                                ESP_TIME_ZONE,
                                ESP_STA_IP_VALUE,
                                ESP_STA_GATEWAY_VALUE,
                                ESP_STA_MASK_VALUE,
                                ESP_STA_DNS_VALUE,
                                ESP_AP_IP_VALUE,
                                ESP_BAUD_RATE,
                                ESP_HTTP_PORT,
                                ESP_TELNET_PORT,
                                ESP_SENSOR_INTERVAL,
                                ESP_BOOT_DELAY,
                                ESP_WEBSOCKET_PORT,
                                ESP_CALIBRATION_1,
                                ESP_CALIBRATION_2,
                                ESP_CALIBRATION_3,
                                ESP_CALIBRATION_4,
                                ESP_CALIBRATION_5,
                                ESP_FTP_CTRL_PORT,
                                ESP_FTP_DATA_ACTIVE_PORT,
                                ESP_FTP_DATA_PASSIVE_PORT,
                                ESP_WEBDAV_PORT,
                                ESP_SERIAL_BRIDGE_BAUD

};

bool Settings_ESP3D::begin() {
  if (GetSettingsVersion() == -1) {
    return false;
  }
  // get target FW
  Settings_ESP3D::GetFirmwareTarget(true);
  Settings_ESP3D::isVerboseBoot(true);
  return true;
}

bool Settings_ESP3D::isVerboseBoot(bool fromsettings) {
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
  if (fromsettings) {
    _isverboseboot = read_byte(ESP_VERBOSE_BOOT);
  }
#else
  _isverboseboot = false;
#endif  // #if COMMUNICATION_PROTOCOL == MKS_SERIAL
  return _isverboseboot;
}

uint8_t Settings_ESP3D::GetFirmwareTarget(bool fromsettings) {
#if defined(FIXED_FW_TARGET)
  (void)fromsettings;
  _FirmwareTarget = FIXED_FW_TARGET;
#else
  if (fromsettings) {
    _FirmwareTarget = read_byte(ESP_TARGET_FW);
  }
#endif  // #if defined( FIXED_FW_TARGET )
  return _FirmwareTarget;
}

uint8_t Settings_ESP3D::GetSDDevice() {
#ifdef SD_DEVICE
  return SD_DEVICE_CONNECTION;
#else   // !SD_DEVICE
  return ESP_NO_SD;
#endif  // SD_DEVICE
}

const char *Settings_ESP3D::GetFirmwareTargetShortName() {
  static String response;

  if (_FirmwareTarget == REPETIER) {
    response = F("repetier");
  } else if (_FirmwareTarget == MARLIN) {
    response = F("marlin");
  } else if (_FirmwareTarget == MARLIN_EMBEDDED) {
    response = F("marlin");
  } else if (_FirmwareTarget == SMOOTHIEWARE) {
    response = F("smoothieware");
  } else if (_FirmwareTarget == GRBL) {
    response = F("grbl");
  } else {
    response = F("unknown");
  }
  return response.c_str();
}
/*
// Max value for a int32 setting
uint32_t Settings_ESP3D::get_max_int32_value(int pos) {
  uint32_t res;
  switch (pos) {
    case ESP_BOOT_DELAY:
      res = MAX_BOOT_DELAY;
      break;
#ifdef FTP_FEATURE
    case ESP_FTP_CTRL_PORT:
    case ESP_FTP_DATA_ACTIVE_PORT:
    case ESP_FTP_DATA_PASSIVE_PORT:
      res = MAX_FTP_PORT;
      break;
#endif  // FTP_FEATURE
#ifdef HTTP_FEATURE
    case ESP_HTTP_PORT:
      res = MAX_HTTP_PORT;
      break;
#endif  // HTTP_FEATURE
#ifdef TELNET_FEATURE
    case ESP_TELNET_PORT:
      res = MAX_TELNET_PORT;
      break;
#endif  // TELNET_FEATURE
#ifdef WEBDAV_FEATURE
    case ESP_WEBDAV_PORT:
      res = MAX_WEBDAV_PORT;
      break;
#endif  // WEBDAV_FEATURE
#ifdef WS_DATA_FEATURE
    case ESP_WEBSOCKET_PORT:
      res = MAX_WEBSOCKET_PORT;
      break;
#endif  // WS_DATA_FEATURE
#if defined(SENSOR_DEVICE)
    case ESP_SENSOR_INTERVAL:
      res = MAX_SENSOR_INTERVAL;
      break;
#endif  // SENSOR_DEVICE
    default:
      res = DEFAULT_ESP_INT;
  }
  return res;
}

// Min value for a int32 setting
uint32_t Settings_ESP3D::get_min_int32_value(int pos) {
  uint32_t res;
  switch (pos) {
    case ESP_BOOT_DELAY:
      res = MIN_BOOT_DELAY;
      break;
#ifdef FTP_FEATURE
    case ESP_FTP_CTRL_PORT:
    case ESP_FTP_DATA_ACTIVE_PORT:
    case ESP_FTP_DATA_PASSIVE_PORT:
      res = MIN_FTP_PORT;
      break;
#endif  // FTP_FEATURE
#ifdef HTTP_FEATURE
    case ESP_HTTP_PORT:
      res = MIN_HTTP_PORT;
      break;
#endif  // HTTP_FEATURE
#ifdef TELNET_FEATURE
    case ESP_TELNET_PORT:
      res = MIN_TELNET_PORT;
      break;
#endif  // TELNET_FEATURE
#ifdef WS_DATA_FEATURE
    case ESP_WEBSOCKET_PORT:
      res = MIN_WEBSOCKET_PORT;
      break;
#endif  // WS_DATA_FEATURE
#ifdef WEBDAV_FEATURE
    case ESP_WEBDAV_PORT:
      res = MIN_WEBDAV_PORT;
      break;
#endif  // WEBDAV_FEATURE
#if defined(SENSOR_DEVICE)
    case ESP_SENSOR_INTERVAL:
      res = MIN_SENSOR_INTERVAL;
      break;
#endif  // SENSOR_DEVICE
    default:
      res = DEFAULT_ESP_INT;
  }
  return res;
}

uint8_t Settings_ESP3D::get_max_byte(int pos) {
  uint8_t res;
  switch (pos) {
#if defined(WIFI_FEATURE)
    case ESP_AP_CHANNEL:
      res = MAX_CHANNEL;
      break;
#endif  // WIFI_FEATURE
    default:
      res = 255;
  }
  return res;
}

int8_t Settings_ESP3D::get_min_byte(int pos) {
  uint8_t res;
  switch (pos) {
#if defined(WIFI_FEATURE)
    case ESP_AP_CHANNEL:
      res = MIN_CHANNEL;
      break;
#endif  // WIFI_FEATURE
    default:
      res = 0;
  }
  return res;
}

*/

/*
// Max size of for a string setting
uint8_t Settings_ESP3D::get_max_string_size(int pos) {
  uint8_t res;
  switch (pos) {
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BLUETOOTH_FEATURE)
    case ESP_HOSTNAME:
      res = MAX_HOSTNAME_LENGTH;
      break;
#endif  // WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
#ifdef TIMESTAMP_FEATURE
    case ESP_TIME_ZONE:
      res = MAX_TIME_ZONE_LENGTH;
      break;
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
      res = MAX_SERVER_ADDRESS_LENGTH;
      break;
#endif  // TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
      res = MAX_NOTIFICATION_TOKEN_LENGTH;
      break;
    case ESP_NOTIFICATION_SETTINGS:
      res = MAX_NOTIFICATION_SETTINGS_LENGTH;
      break;
#endif  // NOTIFICATION_FEATURE
#if defined(WIFI_FEATURE)
    case ESP_STA_SSID:
    case ESP_AP_SSID:
      res = MAX_SSID_LENGTH;
      break;
    case ESP_STA_PASSWORD:
    case ESP_AP_PASSWORD:
      res = MAX_PASSWORD_LENGTH;
      break;
#endif  // WIFI_FEATURE
#ifdef AUTHENTICATION_FEATURE
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
      res = MAX_LOCAL_PASSWORD_LENGTH;
      break;
#endif  // AUTHENTICATION_FEATURE
    case ESP_SETTINGS_VERSION:
      res = MAX_VERSION_LENGTH;
      break;
    default:
      res = DEFAULT_ESP_STRING_SIZE;
  }
  return res;
}

// Min size of for a string setting
uint8_t Settings_ESP3D::get_min_string_size(int pos) {
  uint8_t res;
  switch (pos) {
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BLUETOOTH_FEATURE)
    case ESP_HOSTNAME:
      res = MIN_HOSTNAME_LENGTH;
      break;
#endif  // WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
#ifdef NOTIFICATION_FEATURE
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
      res = MIN_NOTIFICATION_TOKEN_LENGTH;
      break;
    case ESP_NOTIFICATION_SETTINGS:
      res = MIN_NOTIFICATION_SETTINGS_LENGTH;
      break;
#endif  // NOTIFICATION_FEATURE
#ifdef TIMESTAMP_FEATURE
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
      res = MIN_SERVER_ADDRESS_LENGTH;
      break;
#endif  // TIMESTAMP_FEATURE
#if defined(WIFI_FEATURE)
    case ESP_STA_SSID:
    case ESP_AP_SSID:
      res = MIN_SSID_LENGTH;
      break;
    case ESP_STA_PASSWORD:
    case ESP_AP_PASSWORD:
      res = MIN_PASSWORD_LENGTH;
      break;
#endif  // WIFI_FEATURE
#ifdef AUTHENTICATION_FEATURE
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
      res = MIN_LOCAL_PASSWORD_LENGTH;
      break;
#endif  // AUTHENTICATION_FEATURE
    default:
      res = DEFAULT_ESP_STRING_SIZE;
  }
  return res;
}
*/

uint8_t Settings_ESP3D::read_byte(int pos, bool *haserror) {
  if (haserror) {
    *haserror = true;
  }
  uint8_t value = getDefaultByteSetting(pos);
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  // check if parameters are acceptable
  if ((pos + 1 > EEPROM_SIZE)) {
    log_esp3d_e("Error read byte %d", pos);
    return value;
  }
  // read byte
  EEPROM.begin(EEPROM_SIZE);
  value = EEPROM.read(pos);
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, true)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return value;
  }
  String p = "P_" + String(pos);
  if (prefs.isKey(p.c_str())) {
    value = prefs.getChar(p.c_str(), getDefaultByteSetting(pos));
  } else {
    value = getDefaultByteSetting(pos);
  }
  prefs.end();
#endif  // SETTINGS_IN_PREFERENCES
  if (haserror) {
    *haserror = false;
  }
  return value;
}

// write a flag / byte
bool Settings_ESP3D::write_byte(int pos, const uint8_t value) {
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  // check if parameters are acceptable
  if (pos + 1 > EEPROM_SIZE) {
    log_esp3d_e("Error read byte %d", pos);
    return false;
  }
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(pos, value);
  if (!EEPROM.commit()) {
    log_esp3d_e("Error commit %d", pos);
    return false;
  }
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, false)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return false;
  }
  String p = "P_" + String(pos);
  uint8_t r = prefs.putChar(p.c_str(), value);
  prefs.end();
  if (r == 0) {
    log_esp3d_e("Error commit %s", p.c_str());
    return false;
  }
#endif  // SETTINGS_IN_PREFERENCES
  return true;
}

bool Settings_ESP3D::is_string(const char *s, uint len) {
  for (uint p = 0; p < len; p++) {
    if (!isPrintable(char(s[p]))) {
      return false;
    }
  }
  return true;
}

// read a string
// a string is multibyte + \0, this is won't work if 1 char is multibyte like
// chinese char
const char *Settings_ESP3D::read_string(int pos, bool *haserror) {
  const ESP3DSettingDescription *query = getSettingPtr(pos);
  if (haserror) {
    *haserror = true;
  }
  if (!query) {
    log_esp3d_e("Error unknow entry %d", pos);
    return "";
  }
  uint8_t size_max = query->size;

#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  static char *byte_buffer = NULL;
  size_max++;  // do not forget the 0x0 for the end
  if (byte_buffer) {
    free(byte_buffer);
    byte_buffer = NULL;
  }
  // check if parameters are acceptable
  if (pos + size_max + 1 > EEPROM_SIZE) {
    log_esp3d_e("Error read string %d", pos);
    return "";
  }
  byte_buffer = (char *)malloc(size_max + 1);
  if (!byte_buffer) {
    log_esp3d_e("Error mem read string %d", pos);
    return "";
  }
  EEPROM.begin(EEPROM_SIZE);
  byte b = 1;  // non zero for the while loop below
  int i = 0;

  // read until max size is reached or \0 is found
  while (i < size_max && b != 0) {
    b = EEPROM.read(pos + i);
    byte_buffer[i] = isPrintable(char(b)) ? b : 0;
    i++;
  }

  // Be sure there is a 0 at the end.
  if (b != 0) {
    byte_buffer[i - 1] = 0x00;
  }
  EEPROM.end();

  if (haserror) {
    *haserror = false;
  }
  return byte_buffer;

#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  static String res;

  if (!prefs.begin(NAMESPACE, true)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return "";
  }
  String p = "P_" + String(pos);
  if (prefs.isKey(p.c_str())) {
    res = prefs.getString(p.c_str(), getDefaultStringSetting(pos));
  } else {
    res = getDefaultStringSetting(pos);
  }
  prefs.end();

  if (res.length() > size_max) {
    log_esp3d_e("String too long %d vs %d", res.length(), size_max);
    res = res.substring(0, size_max - 1);
  }

  if (haserror) {
    *haserror = false;
  }
  return res.c_str();

#endif  // SETTINGS_IN_PREFERENCES
}

// write a string (array of byte with a 0x00  at the end)
bool Settings_ESP3D::write_string(int pos, const char *byte_buffer) {
  int size_buffer = strlen(byte_buffer);
  const ESP3DSettingDescription *query = getSettingPtr(pos);
  if (!query) {
    log_esp3d_e("Error unknow entry %d", pos);
    return false;
  }
  uint8_t size_max = query->size;

  // check if parameters are acceptable
  if (size_max == 0) {
    log_esp3d_e("Error unknow entry %d", pos);
    return false;
  }
  if (size_max < size_buffer) {
    log_esp3d_e("Error string too long %d, %d", pos, size_buffer);
    return false;
  }
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  if (pos + size_buffer + 1 > EEPROM_SIZE || byte_buffer == NULL) {
    log_esp3d_e("Error write string %d", pos);
    return false;
  }
  // copy the value(s)
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < size_buffer; i++) {
    EEPROM.write(pos + i, byte_buffer[i]);
  }
  // 0 terminal
  EEPROM.write(pos + size_buffer, 0x00);
  if (!EEPROM.commit()) {
    log_esp3d_e("Error commit %d", pos);
    return false;
  }
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, false)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return false;
  }
  String p = "P_" + String(pos);
  uint8_t r = prefs.putString(p.c_str(), byte_buffer);
  prefs.end();
  if (r != size_buffer) {
    log_esp3d_e("Error commit %s", p.c_str());
    return false;
  }
#endif  // SETTINGS_IN_PREFERENCES
  return true;
}

// read a uint32
uint32_t Settings_ESP3D::read_uint32(int pos, bool *haserror) {
  if (haserror) {
    *haserror = true;
  }
  uint32_t res = getDefaultIntegerSetting(pos);
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  // check if parameters are acceptable
  uint8_t size_buffer = sizeof(uint32_t);
  if (pos + size_buffer > EEPROM_SIZE) {
    log_esp3d_e("Error read int %d", pos);
    return res;
  }
  uint8_t i = 0;
  EEPROM.begin(EEPROM_SIZE);
  // read until max size is reached
  while (i < size_buffer) {
    ((uint8_t *)(&res))[i] = EEPROM.read(pos + i);
    i++;
  }
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, true)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return res;
  }
  String p = "P_" + String(pos);
  if (prefs.isKey(p.c_str())) {
    res = prefs.getUInt(p.c_str(), res);
  } else {
    res = getDefaultIntegerSetting(pos);
  }
  prefs.end();
#endif  // SETTINGS_IN_PREFERENCES
  if (haserror) {
    *haserror = false;
  }
  return res;
}

// read an IP
uint32_t Settings_ESP3D::read_IP(int pos, bool *haserror) {
  return read_uint32(pos, haserror);
}

// read an IP
String Settings_ESP3D::read_IP_String(int pos, bool *haserror) {
  return IPtoString(read_uint32(pos, haserror));
}

// write a uint32
bool Settings_ESP3D::write_uint32(int pos, const uint32_t value) {
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  uint8_t size_buffer = sizeof(uint32_t);
  // check if parameters are acceptable
  if (pos + size_buffer > EEPROM_SIZE) {
    log_esp3d_e("Error invalid entry %d", pos);
    return false;
  }
  EEPROM.begin(EEPROM_SIZE);
  // copy the value(s)
  for (int i = 0; i < size_buffer; i++) {
    EEPROM.write(pos + i, ((uint8_t *)(&value))[i]);
  }
  if (!EEPROM.commit()) {
    log_esp3d_e("Error commit %d", pos);
    return false;
  }
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, false)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return false;
  }
  String p = "P_" + String(pos);
  uint8_t r = prefs.putUInt(p.c_str(), value);
  prefs.end();
  if (r == 0) {
    log_esp3d_e("Error commit %s", p.c_str());
    return false;
  }
#endif  // SETTINGS_IN_PREFERENCES
  return true;
}

// write a IP
bool Settings_ESP3D::write_IP(int pos, const uint32_t value) {
  return write_uint32(pos, value);
}

// clear all entries
bool Settings_ESP3D::reset(bool networkonly) {
  // radio mode
  Settings_ESP3D::write_byte(
      ESP_RADIO_MODE, Settings_ESP3D::getDefaultByteSetting(ESP_RADIO_MODE));
  Settings_ESP3D::write_byte(
      ESP_BOOT_RADIO_STATE,
      Settings_ESP3D::getDefaultByteSetting(ESP_BOOT_RADIO_STATE));
  Settings_ESP3D::write_byte(
      ESP_STA_FALLBACK_MODE,
      Settings_ESP3D::getDefaultByteSetting(ESP_STA_FALLBACK_MODE));
#if defined(WIFI_FEATURE)
  // STA SSID
  Settings_ESP3D::write_string(
      ESP_STA_SSID, Settings_ESP3D::getDefaultStringSetting(ESP_STA_SSID));
  // STA pwd
  Settings_ESP3D::write_string(
      ESP_STA_PASSWORD,
      Settings_ESP3D::getDefaultStringSetting(ESP_STA_PASSWORD));
  // AP SSID
  Settings_ESP3D::write_string(
      ESP_AP_SSID, Settings_ESP3D::getDefaultStringSetting(ESP_AP_SSID));
  // AP password
  Settings_ESP3D::write_string(
      ESP_AP_PASSWORD,
      Settings_ESP3D::getDefaultStringSetting(ESP_AP_PASSWORD));
  // AP static IP
  Settings_ESP3D::write_IP(
      ESP_AP_IP_VALUE,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_AP_IP_VALUE));
  // AP Channel
  Settings_ESP3D::write_byte(
      ESP_AP_CHANNEL, Settings_ESP3D::getDefaultByteSetting(ESP_AP_CHANNEL));

#endif  // WIFI_FEATURE

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
  // STA IP mode
  Settings_ESP3D::write_byte(
      ESP_STA_IP_MODE, Settings_ESP3D::getDefaultByteSetting(ESP_STA_IP_MODE));
  // STA static IP
  Settings_ESP3D::write_IP(
      ESP_STA_IP_VALUE,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_STA_IP_VALUE));
  // STA static Gateway
  Settings_ESP3D::write_IP(
      ESP_STA_GATEWAY_VALUE,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_STA_GATEWAY_VALUE));
  // STA static Mask
  Settings_ESP3D::write_IP(
      ESP_STA_MASK_VALUE,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_STA_MASK_VALUE));
  // STA static DNS
  Settings_ESP3D::write_IP(
      ESP_STA_DNS_VALUE,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_STA_DNS_VALUE));
#endif  // WIFI_FEATURE || ETH_FEATURE
  if (networkonly) {
    return true;
  }

  bool res = true;
  log_esp3d("Reset Settings");
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  log_esp3d("clear preferences");
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, false)) {
    return false;
  }
  res = prefs.clear();
  prefs.end();
#endif  // SETTINGS_IN_PREFERENCES

// for EEPROM need to overwrite all settings
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  log_esp3d("clear EEPROM");

  // Setup done (internal only)
  Settings_ESP3D::write_byte(ESP_SETUP,
                             Settings_ESP3D::getDefaultByteSetting(ESP_SETUP));
  // Verbose boot
  Settings_ESP3D::write_byte(
      ESP_VERBOSE_BOOT,
      Settings_ESP3D::getDefaultByteSetting(ESP_VERBOSE_BOOT));
  // Secure Serial
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
  Settings_ESP3D::write_byte(
      ESP_SECURE_SERIAL,
      Settings_ESP3D::getDefaultByteSetting(ESP_SECURE_SERIAL));
#endif  // COMMUNICATION PROTOCOL
#if defined(DISPLAY_DEVICE) && defined(DISPLAY_TOUCH_DRIVER)
  // Calibration done (internal only)
  Settings_ESP3D::write_byte(
      ESP_CALIBRATION, Settings_ESP3D::getDefaultByteSetting(ESP_CALIBRATION));
  // Calibration data (internal only)
  Settings_ESP3D::write_uint32(
      ESP_CALIBRATION_1,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_CALIBRATION_1));
  Settings_ESP3D::write_uint32(
      ESP_CALIBRATION_2,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_CALIBRATION_2));
  Settings_ESP3D::write_uint32(
      ESP_CALIBRATION_3,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_CALIBRATION_3));
  Settings_ESP3D::write_uint32(
      ESP_CALIBRATION_4,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_CALIBRATION_4));
  Settings_ESP3D::write_uint32(
      ESP_CALIBRATION_5,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_CALIBRATION_5));
#endif  // DISPLAY_DEVICE && DISPLAY_TOUCH_DRIVER
#ifdef BUZZER_DEVICE
  // Buzzer state
  Settings_ESP3D::write_byte(ESP_BUZZER,
                             Settings_ESP3D::getDefaultByteSetting(ESP_BUZZER));
#endif  // BUZZER_DEVICE
#if defined(WIFI_FEATURE) || defined(BLUETOOTH_FEATURE) || defined(ETH_FEATURE)
  // Hostname
  Settings_ESP3D::write_string(
      ESP_HOSTNAME, Settings_ESP3D::getDefaultStringSetting(ESP_HOSTNAME));
#endif  // WIFI_FEATURE ||  BLUETOOTH_FEATURE || ETH_FEATURE
#ifdef NOTIFICATION_FEATURE
  // Auto Notification
  Settings_ESP3D::write_byte(
      ESP_AUTO_NOTIFICATION,
      Settings_ESP3D::getDefaultByteSetting(ESP_AUTO_NOTIFICATION));
  // Notification Type
  Settings_ESP3D::write_byte(
      ESP_NOTIFICATION_TYPE,
      Settings_ESP3D::getDefaultByteSetting(ESP_NOTIFICATION_TYPE));
  // Notification Token1
  Settings_ESP3D::write_string(
      ESP_NOTIFICATION_TOKEN1,
      Settings_ESP3D::getDefaultStringSetting(ESP_NOTIFICATION_TOKEN1));
  // Notification Token2
  Settings_ESP3D::write_string(
      ESP_NOTIFICATION_TOKEN2,
      Settings_ESP3D::getDefaultStringSetting(ESP_NOTIFICATION_TOKEN2));
  // Notification Settings
  Settings_ESP3D::write_string(
      ESP_NOTIFICATION_SETTINGS,
      Settings_ESP3D::getDefaultStringSetting(ESP_NOTIFICATION_SETTINGS));
#endif  // NOTIFICATION_FEATURE
  // radio mode
  Settings_ESP3D::write_byte(
      ESP_RADIO_MODE, Settings_ESP3D::getDefaultByteSetting(ESP_RADIO_MODE));

#ifdef FTP_FEATURE
  // FTP On
  Settings_ESP3D::write_byte(ESP_FTP_ON,
                             Settings_ESP3D::getDefaultByteSetting(ESP_FTP_ON));
  // FTP Ctrl Port
  Settings_ESP3D::write_uint32(
      ESP_FTP_CTRL_PORT,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_FTP_CTRL_PORT));
  // FTP Active data Port
  Settings_ESP3D::write_uint32(
      ESP_FTP_DATA_ACTIVE_PORT,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_FTP_DATA_ACTIVE_PORT));
  // FTP Pasive data Port
  Settings_ESP3D::write_uint32(
      ESP_FTP_DATA_PASSIVE_PORT,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_FTP_DATA_PASSIVE_PORT));
#endif  // FTP_FEATURE

#ifdef HTTP_FEATURE
  // HTTP On
  Settings_ESP3D::write_byte(
      ESP_HTTP_ON, Settings_ESP3D::getDefaultByteSetting(ESP_HTTP_ON));
  // HTTP Port
  Settings_ESP3D::write_uint32(
      ESP_HTTP_PORT, Settings_ESP3D::getDefaultIntegerSetting(ESP_HTTP_PORT));
#endif  // HTTP_FEATURE

#ifdef TELNET_FEATURE
  // TELNET On
  Settings_ESP3D::write_byte(
      ESP_TELNET_ON, Settings_ESP3D::getDefaultByteSetting(ESP_TELNET_ON));
  // TELNET Port
  Settings_ESP3D::write_uint32(
      ESP_TELNET_PORT,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_TELNET_PORT));
#endif  // TELNET
#ifdef WS_DATA_FEATURE
  // Websocket On
  Settings_ESP3D::write_byte(
      ESP_WEBSOCKET_ON,
      Settings_ESP3D::getDefaultByteSetting(ESP_WEBSOCKET_ON));
  // Websocket Port
  Settings_ESP3D::write_uint32(
      ESP_WEBSOCKET_PORT,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_WEBSOCKET_PORT));
#endif  // WS_DATA_FEATURE
#ifdef WEBDAV_FEATURE
  // WebDav On
  Settings_ESP3D::write_byte(
      ESP_WEBDAV_ON, Settings_ESP3D::getDefaultByteSetting(ESP_WEBDAV_ON));
  // WebDav Port
  Settings_ESP3D::write_uint32(
      ESP_WEBDAV_PORT,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_WEBDAV_PORT));
#endif  // WEBDAV_FEATURE
#ifdef AUTHENTICATION_FEATURE
  // Admin password
  Settings_ESP3D::write_string(
      ESP_ADMIN_PWD, Settings_ESP3D::getDefaultStringSetting(ESP_ADMIN_PWD));
  // User password
  Settings_ESP3D::write_string(
      ESP_USER_PWD, Settings_ESP3D::getDefaultStringSetting(ESP_USER_PWD));
  // Session timeout
  Settings_ESP3D::write_byte(
      ESP_SESSION_TIMEOUT,
      Settings_ESP3D::getDefaultByteSetting(ESP_SESSION_TIMEOUT));
#endif  // AUTHENTICATION_FEATURE
  // Target FW
  Settings_ESP3D::write_byte(
      ESP_TARGET_FW, Settings_ESP3D::getDefaultByteSetting(ESP_TARGET_FW));
  // Output flags
  Settings_ESP3D::write_byte(
      ESP_SERIAL_FLAG, Settings_ESP3D::getDefaultByteSetting(ESP_SERIAL_FLAG));
  Settings_ESP3D::write_byte(
      ESP_SERIAL_BRIDGE_FLAG,
      Settings_ESP3D::getDefaultByteSetting(ESP_SERIAL_BRIDGE_FLAG));
  Settings_ESP3D::write_byte(
      ESP_REMOTE_SCREEN_FLAG,
      Settings_ESP3D::getDefaultByteSetting(ESP_REMOTE_SCREEN_FLAG));
  Settings_ESP3D::write_byte(
      ESP_WEBSOCKET_FLAG,
      Settings_ESP3D::getDefaultByteSetting(ESP_WEBSOCKET_FLAG));
  Settings_ESP3D::write_byte(
      ESP_TELNET_FLAG, Settings_ESP3D::getDefaultByteSetting(ESP_TELNET_FLAG));
  Settings_ESP3D::write_byte(
      ESP_BT_FLAG, Settings_ESP3D::getDefaultByteSetting(ESP_BT_FLAG));
  Settings_ESP3D::write_byte(
      ESP_SCREEN_FLAG, Settings_ESP3D::getDefaultByteSetting(ESP_SCREEN_FLAG));
#ifdef SD_DEVICE
  // SPI SD Divider
  Settings_ESP3D::write_byte(
      ESP_SD_SPEED_DIV,
      Settings_ESP3D::getDefaultByteSetting(ESP_SD_SPEED_DIV));
#ifdef SD_UPDATE_FEATURE
  // SD Update feature
  Settings_ESP3D::write_byte(
      ESP_SD_CHECK_UPDATE_AT_BOOT,
      Settings_ESP3D::getDefaultByteSetting(ESP_SD_CHECK_UPDATE_AT_BOOT));
#endif  // SD_UPDATE_FEATURE
#endif  // SD_DEVICE

#ifdef TIMESTAMP_FEATURE
  // Internet time
  Settings_ESP3D::write_byte(
      ESP_INTERNET_TIME,
      Settings_ESP3D::getDefaultByteSetting(ESP_INTERNET_TIME));
  // Time Zone
  Settings_ESP3D::write_string(
      ESP_TIME_ZONE, Settings_ESP3D::getDefaultStringSetting(ESP_TIME_ZONE));
  // Is DST Time Zone
  Settings_ESP3D::write_byte(
      ESP_TIME_IS_DST, Settings_ESP3D::getDefaultByteSetting(ESP_TIME_IS_DST));
  // Time Server 1 address
  Settings_ESP3D::write_string(
      ESP_TIME_SERVER1,
      Settings_ESP3D::getDefaultStringSetting(ESP_TIME_SERVER1));
  // Time Server 2 address
  Settings_ESP3D::write_string(
      ESP_TIME_SERVER2,
      Settings_ESP3D::getDefaultStringSetting(ESP_TIME_SERVER2));
  // Time Server 3 address
  Settings_ESP3D::write_string(
      ESP_TIME_SERVER3,
      Settings_ESP3D::getDefaultStringSetting(ESP_TIME_SERVER3));
#endif  // TIMESTAMP_FEATURE
#ifdef SENSOR_DEVICE
  // Sensor device
  Settings_ESP3D::write_byte(
      ESP_SENSOR_TYPE, Settings_ESP3D::getDefaultByteSetting(ESP_SENSOR_TYPE));
  // Sensor query interval
  Settings_ESP3D::write_uint32(
      ESP_SENSOR_INTERVAL,
      Settings_ESP3D::getDefaultIntegerSetting(ESP_SENSOR_INTERVAL));
#endif  // SENSOR_DEVICE
  // Start Delay
  Settings_ESP3D::write_uint32(
      ESP_BOOT_DELAY, Settings_ESP3D::getDefaultIntegerSetting(ESP_BOOT_DELAY));
#endif  // SETTINGS_IN_EEPROM
  // set version in settings
  if (res) {
    log_esp3d("Reset Setting Version");
    // Settings version (internal only)
    res = Settings_ESP3D::write_string(ESP_SETTINGS_VERSION,
                                       CURRENT_SETTINGS_VERSION);
  }
  return res;
}

// Get Settings Version
//  * -1 means no version detected
//  * 00 / 01 Not used
//  * 03 and up is version
int8_t Settings_ESP3D::GetSettingsVersion() {
  int8_t v = -1;
  String version = Settings_ESP3D::read_string(ESP_SETTINGS_VERSION);
  if ((version == "ESP3D") || (version.length() != 7) ||
      (version.indexOf("ESP3D") != 0)) {
    log_esp3d_e("Invalid Settings Version %s", version.c_str());
    return v;
  }
  v = version.substring(5).toInt();
  log_esp3d("Settings Version %d", v);
  return v;
}

// write a IP from string
bool Settings_ESP3D::write_IP_String(int pos, const char *value) {
  return write_uint32(pos, StringtoIP(value));
}

// Helper to convert  IP string to int
uint32_t Settings_ESP3D::StringtoIP(const char *s) {
  uint32_t ip_int = 0;
  IPAddress ipaddr;
  if (ipaddr.fromString(s)) {
    ip_int = ipaddr;
  }
  return ip_int;
}

// Helper to convert int to IP string
String Settings_ESP3D::IPtoString(uint32_t ip_int) {
  static IPAddress ipaddr;
  ipaddr = ip_int;
  return ipaddr.toString();
}

const char *Settings_ESP3D::TargetBoard() {
#ifdef ARDUINO_ARCH_ESP32
#if CONFIG_IDF_TARGET_ESP32
#define TYPE_BOARD "ESP32"
#elif CONFIG_IDF_TARGET_ESP32S2
#define TYPE_BOARD "ESP32-S2"
#elif CONFIG_IDF_TARGET_ESP32S3
#define TYPE_BOARD "ESP32-S3"
#elif CONFIG_IDF_TARGET_ESP32C3
#define TYPE_BOARD "ESP32-C3"
#endif
#ifdef BOARD_HAS_PSRAM
#define IS_PSRAM " (PSRAM)"
#else
#define IS_PSRAM ""
#endif  // BOARD_HAS_PSRAM
  return TYPE_BOARD IS_PSRAM;
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  return "ESP82XX";
#endif  // ARDUINO_ARCH_ESP8266
}

bool Settings_ESP3D::isLocalPasswordValid(const char *password) {
  char c;
  // limited size
  if ((strlen(password) > MAX_LOCAL_PASSWORD_LENGTH) ||
      (strlen(password) <= MIN_LOCAL_PASSWORD_LENGTH)) {
    return false;
  }
  // no space allowed
  for (uint8_t i = 0; i < strlen(password); i++) {
    c = password[i];
    if (c == ' ') {
      return false;
    }
  }
  return true;
}
#endif  // ESP_SAVE_SETTINGS

bool Settings_ESP3D::isValidIPStringSetting(const char *value,
                                            ESP3DSettingIndex settingElement) {
  switch (settingElement) {
    case ESP_STA_IP_VALUE:
    case ESP_STA_GATEWAY_VALUE:
    case ESP_STA_MASK_VALUE:
    case ESP_STA_DNS_VALUE:
    case ESP_AP_IP_VALUE:
      break;
    default:
      return false;
  }

  return true;
}
bool Settings_ESP3D::isValidStringSetting(const char *value,
                                          ESP3DSettingIndex settingElement) {
  switch (settingElement) {
    case ESP_SETTINGS_VERSION:
    case ESP_HOSTNAME:
    case ESP_STA_PASSWORD:
    case ESP_STA_SSID:
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
    case ESP_AP_SSID:
    case ESP_AP_PASSWORD:
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
    case ESP_NOTIFICATION_SETTINGS:
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
    case ESP_TIME_ZONE:
      break;
    default:
      return false;
  }
  return true;
}
bool Settings_ESP3D::isValidIntegerSetting(uint32_t value,
                                           ESP3DSettingIndex settingElement) {
  switch (settingElement) {
    case ESP_BAUD_RATE:
    case ESP_HTTP_PORT:
    case ESP_TELNET_PORT:
    case ESP_SENSOR_INTERVAL:
    case ESP_BOOT_DELAY:
    case ESP_WEBSOCKET_PORT:
    case ESP_CALIBRATION_1:
    case ESP_CALIBRATION_2:
    case ESP_CALIBRATION_3:
    case ESP_CALIBRATION_4:
    case ESP_CALIBRATION_5:
    case ESP_FTP_CTRL_PORT:
    case ESP_FTP_DATA_ACTIVE_PORT:
    case ESP_FTP_DATA_PASSIVE_PORT:
    case ESP_WEBDAV_PORT:
    case ESP_SERIAL_BRIDGE_BAUD:
      break;
    default:
      return false;
  }
  return true;
}
bool Settings_ESP3D::isValidByteSetting(uint8_t value,
                                        ESP3DSettingIndex settingElement) {
  switch (settingElement) {
    case ESP_RADIO_MODE:
    case ESP_NOTIFICATION_TYPE:
    case ESP_CALIBRATION:
    case ESP_AP_CHANNEL:
    case ESP_BUZZER:
    case ESP_INTERNET_TIME:
    case ESP_SERIAL_FLAG:
    case ESP_HTTP_ON:
    case ESP_TELNET_ON:
    case ESP_WEBSOCKET_ON:
    case ESP_SD_SPEED_DIV:
    case ESP_SENSOR_TYPE:
    case ESP_TARGET_FW:
    case ESP_TIME_IS_DST:
    case ESP_REMOTE_SCREEN_FLAG:
    case ESP_SD_MOUNT:
    case ESP_SESSION_TIMEOUT:
    case ESP_WEBSOCKET_FLAG:
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
    case ESP_SETUP:
    case ESP_TELNET_FLAG:
    case ESP_BT_FLAG:
    case ESP_SCREEN_FLAG:
    case ESP_FTP_ON:
    case ESP_AUTO_NOTIFICATION:
    case ESP_VERBOSE_BOOT:
    case ESP_WEBDAV_ON:
    case ESP_SECURE_SERIAL:
    case ESP_BOOT_RADIO_STATE:
    case ESP_STA_FALLBACK_MODE:
    case ESP_SERIAL_BRIDGE_FLAG:
    case ESP_SERIAL_BRIDGE_ON:
      break;
    default:
      return false;
  }

  return true;
}

uint32_t Settings_ESP3D::getDefaultIntegerSetting(
    ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *query = getSettingPtr(settingElement);
  if (getSettingPtr) {
    if (query->type == ESP3DSettingType::ip_t) {
      return StringtoIP(query->default_val);
    }
    if (query->type == ESP3DSettingType::integer_t)
      return (uint32_t)strtoul(query->default_val, NULL, 0);
    else {
      log_esp3d_e("Error invalid type %d for %d", query->type, settingElement);
    }
  }
  return 0;
}
const char *Settings_ESP3D::getDefaultStringSetting(
    ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *query = getSettingPtr(settingElement);
  if (getSettingPtr) {
    if (query->type == ESP3DSettingType::string_t ||
        query->type == ESP3DSettingType::ip_t)
      return query->default_val;
    else {
      log_esp3d_e("Error invalid type %d for %d", query->type, settingElement);
    }
  }
  return NULL;
}
uint8_t Settings_ESP3D::getDefaultByteSetting(
    ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *query = getSettingPtr(settingElement);
  if (getSettingPtr) {
    if (query->type == ESP3DSettingType::byte_t)
      return (uint8_t)strtoul(query->default_val, NULL, 0);
    else {
      log_esp3d_e("Error invalid type %d for %d", query->type, settingElement);
    }
  }
  return 0;
}

// get the description of a setting
// Unlike esp32 esp8266 does not have lot of memory so we hard code the
// settings and just generate one setting on demand
const ESP3DSettingDescription *Settings_ESP3D::getSettingPtr(
    const ESP3DSettingIndex index) {
  static ESP3DSettingDescription setting;
  memset(&setting, 0, sizeof(setting));
  // index of setting
  setting.index = index;
  // type of setting
  switch (index) {
    case ESP_RADIO_MODE:
    case ESP_NOTIFICATION_TYPE:
    case ESP_CALIBRATION:
    case ESP_AP_CHANNEL:
    case ESP_BUZZER:
    case ESP_INTERNET_TIME:
    case ESP_SERIAL_FLAG:
    case ESP_HTTP_ON:
    case ESP_TELNET_ON:
    case ESP_WEBSOCKET_ON:
    case ESP_SD_SPEED_DIV:
    case ESP_SENSOR_TYPE:
    case ESP_TARGET_FW:
    case ESP_TIME_IS_DST:
    case ESP_REMOTE_SCREEN_FLAG:
    case ESP_SD_MOUNT:
    case ESP_SESSION_TIMEOUT:
    case ESP_WEBSOCKET_FLAG:
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
    case ESP_SETUP:
    case ESP_TELNET_FLAG:
    case ESP_BT_FLAG:
    case ESP_SCREEN_FLAG:
    case ESP_FTP_ON:
    case ESP_AUTO_NOTIFICATION:
    case ESP_VERBOSE_BOOT:
    case ESP_WEBDAV_ON:
    case ESP_SECURE_SERIAL:
    case ESP_BOOT_RADIO_STATE:
    case ESP_STA_FALLBACK_MODE:
    case ESP_SERIAL_BRIDGE_FLAG:
    case ESP_SERIAL_BRIDGE_ON:
      setting.type = ESP3DSettingType::byte_t;  // byte
      break;

    case ESP_SETTINGS_VERSION:
    case ESP_HOSTNAME:
    case ESP_STA_PASSWORD:
    case ESP_STA_SSID:
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
    case ESP_AP_SSID:
    case ESP_AP_PASSWORD:
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
    case ESP_NOTIFICATION_SETTINGS:
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
    case ESP_TIME_ZONE:
      setting.type = ESP3DSettingType::string_t;  // string
      break;

    case ESP_STA_IP_VALUE:
    case ESP_STA_GATEWAY_VALUE:
    case ESP_STA_MASK_VALUE:
    case ESP_STA_DNS_VALUE:
    case ESP_AP_IP_VALUE:

      setting.type = ESP3DSettingType::ip_t;  // ip = 4 bytes
      break;

    case ESP_BAUD_RATE:
    case ESP_HTTP_PORT:
    case ESP_TELNET_PORT:
    case ESP_SENSOR_INTERVAL:
    case ESP_BOOT_DELAY:
    case ESP_WEBSOCKET_PORT:
    case ESP_CALIBRATION_1:
    case ESP_CALIBRATION_2:
    case ESP_CALIBRATION_3:
    case ESP_CALIBRATION_4:
    case ESP_CALIBRATION_5:
    case ESP_FTP_CTRL_PORT:
    case ESP_FTP_DATA_ACTIVE_PORT:
    case ESP_FTP_DATA_PASSIVE_PORT:
    case ESP_WEBDAV_PORT:
    case ESP_SERIAL_BRIDGE_BAUD:

      setting.type = ESP3DSettingType::integer_t;  // integer = 4 bytes
      break;
    default:
      setting.type = ESP3DSettingType::unknown_t;
      break;
  }

  // size of setting
  switch (index) {
    case ESP_RADIO_MODE:
    case ESP_NOTIFICATION_TYPE:
    case ESP_CALIBRATION:
    case ESP_AP_CHANNEL:
    case ESP_BUZZER:
    case ESP_INTERNET_TIME:
    case ESP_SERIAL_FLAG:
    case ESP_HTTP_ON:
    case ESP_TELNET_ON:
    case ESP_WEBSOCKET_ON:
    case ESP_SD_SPEED_DIV:
    case ESP_SENSOR_TYPE:
    case ESP_TARGET_FW:
    case ESP_TIME_IS_DST:
    case ESP_REMOTE_SCREEN_FLAG:
    case ESP_SD_MOUNT:
    case ESP_SESSION_TIMEOUT:
    case ESP_WEBSOCKET_FLAG:
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
    case ESP_SETUP:
    case ESP_TELNET_FLAG:
    case ESP_BT_FLAG:
    case ESP_SCREEN_FLAG:
    case ESP_FTP_ON:
    case ESP_AUTO_NOTIFICATION:
    case ESP_VERBOSE_BOOT:
    case ESP_WEBDAV_ON:
    case ESP_SECURE_SERIAL:
    case ESP_BOOT_RADIO_STATE:
    case ESP_STA_FALLBACK_MODE:
    case ESP_SERIAL_BRIDGE_FLAG:
    case ESP_SERIAL_BRIDGE_ON:
      setting.size = 1;  // 1 byte
      break;
    case ESP_STA_IP_VALUE:
    case ESP_STA_GATEWAY_VALUE:
    case ESP_STA_MASK_VALUE:
    case ESP_STA_DNS_VALUE:
    case ESP_AP_IP_VALUE:
    case ESP_BAUD_RATE:
    case ESP_HTTP_PORT:
    case ESP_TELNET_PORT:
    case ESP_SENSOR_INTERVAL:
    case ESP_BOOT_DELAY:
    case ESP_WEBSOCKET_PORT:
    case ESP_CALIBRATION_1:
    case ESP_CALIBRATION_2:
    case ESP_CALIBRATION_3:
    case ESP_CALIBRATION_4:
    case ESP_CALIBRATION_5:
    case ESP_FTP_CTRL_PORT:
    case ESP_FTP_DATA_ACTIVE_PORT:
    case ESP_FTP_DATA_PASSIVE_PORT:
    case ESP_WEBDAV_PORT:
    case ESP_SERIAL_BRIDGE_BAUD:
      setting.size = 4;  // 4 bytes
      break;
    // Note for string size is the max size of the string, in EEPROM it use
    // the size + 1 for the ending 0x00
    case ESP_STA_SSID:
      setting.size = 32;  // 32 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_SETTINGS_VERSION:
      setting.size = 7;  // 7 bytes
      break;
    case ESP_HOSTNAME:
      setting.size = 32;  // 32 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_STA_PASSWORD:
      setting.size = 64;  // 64 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
      setting.size = 20;  // 20 bytes
      break;
    case ESP_AP_SSID:
      setting.size = 32;  // 32 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_AP_PASSWORD:
      setting.size = 64;  // 64 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
      setting.size = 63;  // 63 bytes
      break;
    case ESP_NOTIFICATION_SETTINGS:
      setting.size = 128;  // 128 bytes
      break;
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
      setting.size = 128;  // 128 bytes
      break;
    case ESP_TIME_ZONE:
      setting.size = 6;  // 6 bytes
      break;

    default:
      break;
  }

  // default value of setting in string
  switch (index) {
    case ESP_RADIO_MODE:
      setting.default_val = DEFAULT_ESP_RADIO_MODE;
      break;
    case ESP_STA_SSID:
      setting.default_val = DEFAULT_STA_SSID;
      break;
    case ESP_NOTIFICATION_TYPE:
      setting.default_val = DEFAULT_NOTIFICATION_TYPE;
      break;
    case ESP_CALIBRATION:
      setting.default_val = DEFAULT_CALIBRATION_DONE;
      break;
    case ESP_AP_CHANNEL:
      setting.default_val = DEFAULT_AP_CHANNEL;
      break;
    case ESP_BUZZER:
      setting.default_val = DEFAULT_BUZZER_STATE;
      break;
    case ESP_INTERNET_TIME:
      setting.default_val = DEFAULT_INTERNET_TIME;
      break;
    case ESP_SERIAL_FLAG:
      setting.default_val = DEFAULT_SERIAL_OUTPUT_FLAG;
      break;
    case ESP_HTTP_ON:
      setting.default_val = DEFAULT_HTTP_ON;
      break;
    case ESP_TELNET_ON:
      setting.default_val = DEFAULT_TELNET_ON;
      break;
    case ESP_WEBSOCKET_ON:
      setting.default_val = DEFAULT_WEBSOCKET_ON;
      break;
    case ESP_SD_SPEED_DIV:
      setting.default_val = DEFAULT_SD_SPI_DIV;
      break;
    case ESP_SENSOR_TYPE:
      setting.default_val = DEFAULT_SENSOR_TYPE;
      break;
    case ESP_TARGET_FW:
      setting.default_val = DEFAULT_FW;
      break;
    case ESP_TIME_IS_DST:
      setting.default_val = DEFAULT_TIME_DST;
      break;
    case ESP_REMOTE_SCREEN_FLAG:
      setting.default_val = DEFAULT_REMOTE_SCREEN_FLAG;
      break;
    case ESP_SD_MOUNT:
      setting.default_val = DEFAULT_SD_MOUNT;
      break;
    case ESP_SESSION_TIMEOUT:
      setting.default_val = DEFAULT_SESSION_TIMEOUT;
      break;
    case ESP_WEBSOCKET_FLAG:
      setting.default_val = DEFAULT_WEBSOCKET_FLAG;
      break;
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
      setting.default_val = DEFAULT_SD_CHECK_UPDATE_AT_BOOT;
      break;
    case ESP_SETUP:
      setting.default_val = DEFAULT_SETUP;
      break;
    case ESP_TELNET_FLAG:
      setting.default_val = DEFAULT_TELNET_FLAG;
      break;
    case ESP_BT_FLAG:
      setting.default_val = DEFAULT_BT_FLAG;
      break;
    case ESP_SCREEN_FLAG:
      setting.default_val = DEFAULT_SCREEN_FLAG;
      break;
    case ESP_FTP_ON:
      setting.default_val = DEFAULT_FTP_ON;
      break;
    case ESP_AUTO_NOTIFICATION:
      setting.default_val = DEFAULT_AUTO_NOTIFICATION_STATE;
      break;
    case ESP_VERBOSE_BOOT:
      setting.default_val = DEFAULT_VERBOSE_BOOT;
      break;
    case ESP_WEBDAV_ON:
      setting.default_val = DEFAULT_WEBDAV_ON;
      break;
    case ESP_SECURE_SERIAL:
      setting.default_val = DEFAULT_SECURE_SERIAL;
      break;
    case ESP_BOOT_RADIO_STATE:
      setting.default_val = DEFAULT_BOOT_RADIO_STATE;
      break;
    case ESP_STA_FALLBACK_MODE:
      setting.default_val = DEFAULT_STA_FALLBACK_MODE;
      break;
    case ESP_SERIAL_BRIDGE_FLAG:
      setting.default_val = DEFAULT_SERIAL_BRIDGE_FLAG;
      break;
    case ESP_SERIAL_BRIDGE_ON:
      setting.default_val = DEFAULT_SERIAL_BRIDGE_ON;
      break;
    case ESP_SERIAL_BRIDGE_BAUD:
      setting.default_val = DEFAULT_SERIAL_BRIDGE_BAUD_RATE;
      break;
    case ESP_SETTINGS_VERSION:
      setting.default_val = DEFAULT_SETTINGS_VERSION;
      break;
    case ESP_HOSTNAME:
      setting.default_val = DEFAULT_HOSTNAME;
      break;
    case ESP_STA_PASSWORD:
      setting.default_val = DEFAULT_STA_PASSWORD;
      break;
    case ESP_ADMIN_PWD:
      setting.default_val = DEFAULT_ADMIN_PWD;
      break;
    case ESP_USER_PWD:
      setting.default_val = DEFAULT_USER_PWD;
      break;
    case ESP_AP_SSID:
      setting.default_val = DEFAULT_AP_SSID;
      break;
    case ESP_AP_PASSWORD:
      setting.default_val = DEFAULT_AP_PASSWORD;
      break;
    case ESP_NOTIFICATION_TOKEN1:
      setting.default_val = DEFAULT_NOTIFICATION_TOKEN1;
      break;
    case ESP_NOTIFICATION_TOKEN2:
      setting.default_val = DEFAULT_NOTIFICATION_TOKEN2;
      break;
    case ESP_NOTIFICATION_SETTINGS:
      setting.default_val = DEFAULT_NOTIFICATION_SETTINGS;
      break;
    case ESP_TIME_SERVER1:
      setting.default_val = DEFAULT_TIME_SERVER1;
      break;
    case ESP_TIME_SERVER2:
      setting.default_val = DEFAULT_TIME_SERVER2;
      break;
    case ESP_TIME_SERVER3:
      setting.default_val = DEFAULT_TIME_SERVER3;
      break;
    case ESP_TIME_ZONE:
      setting.default_val = DEFAULT_TIME_ZONE;
      break;
    case ESP_STA_IP_VALUE:
      setting.default_val = DEFAULT_STA_IP_VALUE;
      break;
    case ESP_STA_GATEWAY_VALUE:
      setting.default_val = DEFAULT_STA_GATEWAY_VALUE;
      break;
    case ESP_STA_MASK_VALUE:
      setting.default_val = DEFAULT_STA_MASK_VALUE;
      break;
    case ESP_STA_DNS_VALUE:
      setting.default_val = DEFAULT_STA_DNS_VALUE;
      break;
    case ESP_AP_IP_VALUE:
      setting.default_val = DEFAULT_AP_IP_VALUE;
      break;
    case ESP_BAUD_RATE:
      setting.default_val = DEFAULT_BAUD_RATE;
      break;
    case ESP_HTTP_PORT:
      setting.default_val = DEFAULT_HTTP_PORT;
      break;
    case ESP_TELNET_PORT:
      setting.default_val = DEFAULT_TELNET_PORT;
      break;
    case ESP_SENSOR_INTERVAL:
      setting.default_val = DEFAULT_SENSOR_INTERVAL;
      break;
    case ESP_BOOT_DELAY:
      setting.default_val = DEFAULT_BOOT_DELAY;
      break;
    case ESP_WEBSOCKET_PORT:
      setting.default_val = DEFAULT_WEBSOCKET_PORT;
      break;
    case ESP_CALIBRATION_1:
    case ESP_CALIBRATION_2:
    case ESP_CALIBRATION_3:
    case ESP_CALIBRATION_4:
    case ESP_CALIBRATION_5:
      setting.default_val = DEFAULT_CALIBRATION_VALUE;
      break;
    case ESP_FTP_CTRL_PORT:
      setting.default_val = DEFAULT_FTP_CTRL_PORT;
      break;
    case ESP_FTP_DATA_ACTIVE_PORT:
      setting.default_val = DEFAULT_FTP_ACTIVE_PORT;
      break;
    case ESP_FTP_DATA_PASSIVE_PORT:
      setting.default_val = DEFAULT_FTP_PASSIVE_PORT;
      break;
    case ESP_WEBDAV_PORT:
      setting.default_val = DEFAULT_WEBDAV_PORT;
      break;
    default:
      log_esp3d_e("Invalid setting %d", index);
      return NULL;
      break;
  }

  return &setting;
}