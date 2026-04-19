// ###########################################################################
// #                                                                         #
// # Module: rF2 Telemetry Plugin Header File                                #
// #                                                                         #
// # Description: Declarations for the rF2 telemetry plugin                  #
// #                                                                         #
// #                                                                         #
// # This source code module, and all information, data, and algorithms      #
// # associated with it, are part of CUBE technology (tm).                   #
// #                 PROPRIETARY AND CONFIDENTIAL                            #
// # Copyright (c) 2017 Studio 397 B.V.  All rights reserved.               #
// #                                                                         #
// #                                                                         #
// # Change history:                                                         #
// #   tag.2005.11.30: created                                               #
// #                                                                         #
// ###########################################################################

#ifndef _RF2_TELEMETRY_PLUGIN_H
#define _RF2_TELEMETRY_PLUGIN_H

#include "InternalsPlugin.hpp"
#include <windows.h>

// This is used for the app to use the plugin for its intended purpose
class RF2TelemetryInternalsPlugin
    : public InternalsPluginV01  // REMINDER: exported function
                                 // GetPluginVersion() should return 1 if you
                                 // are deriving from this InternalsPluginV01, 2
                                 // for InternalsPluginV02, etc.
{
 public:
  // Constructor/destructor
  RF2TelemetryInternalsPlugin() {}
  ~RF2TelemetryInternalsPlugin() {}

  // These are the functions derived from base class InternalsPlugin
  // that can be implemented.
  void Startup(long version);  // game startup
  void Shutdown();             // game shutdown

  void EnterRealtime();  // entering realtime
  void ExitRealtime();   // exiting realtime

  void StartSession();  // session has started
  void EndSession();    // session has ended

  // GAME OUTPUT
  long WantsTelemetryUpdates() {
    return (1);
  }  // Return 1 to enable telemetry updates.
  void UpdateTelemetry(const TelemInfoV01& info);

  bool WantsGraphicsUpdates() {
    return (false);
  }  // Return true to enable graphics updates.
  void UpdateGraphics(const GraphicsInfoV01& info);

  // GAME INPUT
  bool HasHardwareInputs() {
    return (false);
  }  // Return true to enable hardware input updates.
  void UpdateHardware(const double fDT) {
    mET += fDT;
  }  // update the hardware with the time between frames
  void EnableHardware() {
    mEnabled = true;
  }  // message from game to enable hardware
  void DisableHardware() {
    mEnabled = false;
  }  // message from game to disable hardware

  // See if the plugin wants to take over a hardware control.  If the plugin
  // takes over the control, this method returns true and sets the value of the
  // double pointed to by the second arg.  Otherwise, it returns false and
  // leaves the double unmodified.
  bool CheckHWControl(const char* const controlName, double& fRetVal);

  bool ForceFeedback(
      double& forceValue);  // SEE FUNCTION BODY TO ENABLE FORCE EXAMPLE

  // SCORING OUTPUT
  bool WantsScoringUpdates() {
    return (true);
  }
  void UpdateScoring(const ScoringInfoV01& info);

  // COMMENTARY INPUT
  bool RequestCommentary(
      CommentaryRequestInfoV01&
          info);  // SEE FUNCTION BODY TO ENABLE COMMENTARY EXAMPLE

 private:
  void WriteToLogFile(const char* const openStr, const char* const msg);
  void OpenSerialPort();
  void CloseSerialPort();
  void TryReconnectSerial();
  void OpenHidDevice();
  void CloseHidDevice();
  void TryReconnectHid();
  void SendTelemetryFrame(unsigned char gear, unsigned short speed,
                          unsigned short rpm, unsigned char tire_temp_fl,
                          unsigned char tire_temp_fr,
                          unsigned char tire_temp_rl,
                          unsigned char tire_temp_rr,
                          unsigned short brake_temp_fl,
                          unsigned short brake_temp_fr,
                          unsigned short brake_temp_rl,
                          unsigned short brake_temp_rr,
                          unsigned char water_temp,
                          unsigned char oil_temp,
                          unsigned long best_lap_ms,
                          unsigned long current_lap_ms,
                          unsigned short rpm_pct_x10,
                          unsigned char fuel_liters,
                          unsigned char fuel_pct,
                          unsigned char throttle_pct,
                          unsigned char brake_pct);
  double mET;     // needed for the hardware example
  bool mEnabled;  // needed for the hardware example
  HANDLE mSerialHandle = INVALID_HANDLE_VALUE;
  HANDLE mHidHandle = INVALID_HANDLE_VALUE;
  unsigned long long mLastReconnectTick = 0;
  bool mSerialErrorLogged = false;
  bool mHidErrorLogged = false;
  unsigned long mBestLapMilliseconds = 0;
  unsigned long mCurrentLapMilliseconds = 0;
};

#endif  // _RF2_TELEMETRY_PLUGIN_H
