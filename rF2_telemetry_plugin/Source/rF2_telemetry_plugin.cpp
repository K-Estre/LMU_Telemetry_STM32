// ###########################################################################
// #                                                                         #
// # Module: rF2 Telemetry Plugin Source File                                #
// #                                                                         #
// # Description: Declarations for the rF2 telemetry plugin                  #
// #                                                                         #
// ###########################################################################

#include "rF2_telemetry_plugin.hpp"

#include <math.h>
#include <stdio.h>

namespace {

constexpr unsigned char kFrameHeader1 = 0xAA;
constexpr unsigned char kFrameHeader2 = 0x55;
constexpr unsigned long kBaudRate = CBR_115200;
constexpr unsigned long long kReconnectIntervalMs = 1000;
constexpr char kSerialPortName[] = "\\\\.\\COM3";

unsigned char ClampGear(long gear) {
  if (gear < 0) {
    return 255;
  }

  if (gear == 0) {
    return 0;
  }

  if (gear > 255) {
    return 255;
  }

  return static_cast<unsigned char>(gear);
}

unsigned short ClampUnsignedShort(long value) {
  if (value <= 0) {
    return 0;
  }

  if (value > 65535) {
    return 65535;
  }

  return static_cast<unsigned short>(value);
}

unsigned short ClampRpmPctX10(double rpm, double max_rpm) {
  long pct_x10;

  if (max_rpm <= 0.0) {
    return 0;
  }

  pct_x10 = static_cast<long>(((rpm / max_rpm) * 1000.0) + 0.5);

  if (pct_x10 < 0) {
    return 0;
  }

  if (pct_x10 > 1000) {
    return 1000;
  }

  return static_cast<unsigned short>(pct_x10);
}

unsigned char ClampTireTemp(double kelvin) {
  long celsius = static_cast<long>((kelvin - 273.15) + 0.5);

  if (celsius < 0) {
    return 0;
  }

  if (celsius > 99) {
    return 99;
  }

  return static_cast<unsigned char>(celsius);
}

unsigned char ClampPercent(double value) {
  long percent = static_cast<long>((value * 100.0) + 0.5);

  if (percent < 0) {
    return 0;
  }

  if (percent > 100) {
    return 100;
  }

  return static_cast<unsigned char>(percent);
}

unsigned char ClampTemperatureC(double value) {
  long temp = static_cast<long>(value + 0.5);

  if (temp < 0) {
    return 0;
  }

  if (temp > 255) {
    return 255;
  }

  return static_cast<unsigned char>(temp);
}

unsigned char ClampFuelLiters(double liters) {
  long rounded = static_cast<long>(liters + 0.5);

  if (rounded < 0) {
    return 0;
  }

  if (rounded > 255) {
    return 255;
  }

  return static_cast<unsigned char>(rounded);
}

unsigned long ClampLapMilliseconds(double seconds) {
  long milliseconds;

  if (seconds <= 0.0) {
    return 0;
  }

  milliseconds = static_cast<long>((seconds * 1000.0) + 0.5);

  if (milliseconds < 0) {
    return 0;
  }

  if (milliseconds > 0x7FFFFFFF) {
    return 0x7FFFFFFF;
  }

  return static_cast<unsigned long>(milliseconds);
}

}  // namespace

extern "C" __declspec(dllexport) const char* __cdecl GetPluginName() {
  return ("rF2 Telemetry Plugin");
}

extern "C" __declspec(dllexport) PluginObjectType __cdecl GetPluginType() {
  return (PO_INTERNALS);
}

extern "C" __declspec(dllexport) int __cdecl GetPluginVersion() { return (1); }

extern "C" __declspec(dllexport) PluginObject* __cdecl CreatePluginObject() {
  return ((PluginObject*)new RF2TelemetryInternalsPlugin);
}

extern "C" __declspec(dllexport) void __cdecl DestroyPluginObject(
    PluginObject* obj) {
  delete ((RF2TelemetryInternalsPlugin*)obj);
}

void RF2TelemetryInternalsPlugin::WriteToLogFile(const char* const openStr,
                                                 const char* const msg) {
  FILE* fo = fopen("LMU_Telemetry_Output.txt", openStr);
  if (fo != NULL) {
    fprintf(fo, "%s\n", msg);
    fclose(fo);
  }
}

void RF2TelemetryInternalsPlugin::OpenSerialPort() {
  DCB dcb = {};
  COMMTIMEOUTS timeouts = {};
  char msg[128];

  if (mSerialHandle != INVALID_HANDLE_VALUE) {
    return;
  }

  mSerialHandle =
      CreateFileA(kSerialPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (mSerialHandle == INVALID_HANDLE_VALUE) {
    if (!mSerialErrorLogged) {
      sprintf(msg, "Serial open failed on %s, err=%lu", kSerialPortName,
              GetLastError());
      WriteToLogFile("a", msg);
      mSerialErrorLogged = true;
    }
    return;
  }

  SetupComm(mSerialHandle, 4096, 4096);
  PurgeComm(mSerialHandle,
            PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

  dcb.DCBlength = sizeof(dcb);
  if (!GetCommState(mSerialHandle, &dcb)) {
    sprintf(msg, "GetCommState failed, err=%lu", GetLastError());
    WriteToLogFile("a", msg);
    CloseSerialPort();
    return;
  }

  dcb.BaudRate = kBaudRate;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;

  if (!SetCommState(mSerialHandle, &dcb)) {
    sprintf(msg, "SetCommState failed, err=%lu", GetLastError());
    WriteToLogFile("a", msg);
    CloseSerialPort();
    return;
  }

  timeouts.ReadIntervalTimeout = 20;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = 20;
  timeouts.WriteTotalTimeoutMultiplier = 5;
  timeouts.WriteTotalTimeoutConstant = 20;
  if (!SetCommTimeouts(mSerialHandle, &timeouts)) {
    sprintf(msg, "SetCommTimeouts failed, err=%lu", GetLastError());
    WriteToLogFile("a", msg);
    CloseSerialPort();
    return;
  }

  mSerialErrorLogged = false;
  WriteToLogFile("a", "Serial COM3 opened");
}

void RF2TelemetryInternalsPlugin::CloseSerialPort() {
  if (mSerialHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(mSerialHandle);
    mSerialHandle = INVALID_HANDLE_VALUE;
  }
}

void RF2TelemetryInternalsPlugin::TryReconnectSerial() {
  unsigned long long now = GetTickCount64();

  if (mSerialHandle != INVALID_HANDLE_VALUE) {
    return;
  }

  if ((now - mLastReconnectTick) < kReconnectIntervalMs) {
    return;
  }

  mLastReconnectTick = now;
  OpenSerialPort();
}

static void RF2TelemetrySendFrame(HANDLE serial_handle, unsigned char gear,
                                  unsigned short speed, unsigned short rpm,
                                  unsigned char tire_temp_fl,
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
                                  unsigned char brake_pct,
                                  bool* serial_error_logged) {
  DWORD written = 0;
  unsigned char frame[35];
  char msg[128];

  frame[0] = kFrameHeader1;
  frame[1] = kFrameHeader2;
  frame[2] = gear;
  frame[3] = static_cast<unsigned char>(speed & 0xFF);
  frame[4] = static_cast<unsigned char>((speed >> 8) & 0xFF);
  frame[5] = static_cast<unsigned char>(rpm & 0xFF);
  frame[6] = static_cast<unsigned char>((rpm >> 8) & 0xFF);
  frame[7] = tire_temp_fl;
  frame[8] = tire_temp_fr;
  frame[9] = tire_temp_rl;
  frame[10] = tire_temp_rr;
  frame[11] = static_cast<unsigned char>(brake_temp_fl & 0xFF);
  frame[12] = static_cast<unsigned char>((brake_temp_fl >> 8) & 0xFF);
  frame[13] = static_cast<unsigned char>(brake_temp_fr & 0xFF);
  frame[14] = static_cast<unsigned char>((brake_temp_fr >> 8) & 0xFF);
  frame[15] = static_cast<unsigned char>(brake_temp_rl & 0xFF);
  frame[16] = static_cast<unsigned char>((brake_temp_rl >> 8) & 0xFF);
  frame[17] = static_cast<unsigned char>(brake_temp_rr & 0xFF);
  frame[18] = static_cast<unsigned char>((brake_temp_rr >> 8) & 0xFF);
  frame[19] = water_temp;
  frame[20] = oil_temp;
  frame[21] = static_cast<unsigned char>(best_lap_ms & 0xFF);
  frame[22] = static_cast<unsigned char>((best_lap_ms >> 8) & 0xFF);
  frame[23] = static_cast<unsigned char>((best_lap_ms >> 16) & 0xFF);
  frame[24] = static_cast<unsigned char>((best_lap_ms >> 24) & 0xFF);
  frame[25] = static_cast<unsigned char>(current_lap_ms & 0xFF);
  frame[26] = static_cast<unsigned char>((current_lap_ms >> 8) & 0xFF);
  frame[27] = static_cast<unsigned char>((current_lap_ms >> 16) & 0xFF);
  frame[28] = static_cast<unsigned char>((current_lap_ms >> 24) & 0xFF);
  frame[29] = static_cast<unsigned char>(rpm_pct_x10 & 0xFF);
  frame[30] = static_cast<unsigned char>((rpm_pct_x10 >> 8) & 0xFF);
  frame[31] = fuel_liters;
  frame[32] = fuel_pct;
  frame[33] = throttle_pct;
  frame[34] = brake_pct;

  if (!WriteFile(serial_handle, frame, sizeof(frame), &written, NULL) ||
      (written != sizeof(frame))) {
    sprintf(msg, "Serial write failed, err=%lu", GetLastError());
    FILE* fo = fopen("LMU_Telemetry_Output.txt", "a");
    if (fo != NULL) {
      fprintf(fo, "%s\n", msg);
      fclose(fo);
    }
    *serial_error_logged = true;
  }
}

void RF2TelemetryInternalsPlugin::SendTelemetryFrame(unsigned char gear,
                                                     unsigned short speed,
                                                     unsigned short rpm,
                                                     unsigned char tire_temp_fl,
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
                                                     unsigned char brake_pct) {
  TryReconnectSerial();
  if (mSerialHandle == INVALID_HANDLE_VALUE) {
    return;
  }

  RF2TelemetrySendFrame(mSerialHandle, gear, speed, rpm, tire_temp_fl,
                        tire_temp_fr, tire_temp_rl, tire_temp_rr,
                        brake_temp_fl, brake_temp_fr, brake_temp_rl,
                        brake_temp_rr, water_temp, oil_temp, best_lap_ms,
                        current_lap_ms, rpm_pct_x10, fuel_liters, fuel_pct,
                        throttle_pct, brake_pct,
                        &mSerialErrorLogged);

  if (mSerialErrorLogged) {
    WriteToLogFile("a", "Serial write failed");
    CloseSerialPort();
  }
}

void RF2TelemetryInternalsPlugin::Startup(long version) {
  char temp[128];
  sprintf(temp, "-STARTUP- (version %.3f)", (float)version / 1000.0f);
  WriteToLogFile("w", temp);

  mEnabled = true;
  mET = 0.0;
  mLastReconnectTick = 0;
  mSerialErrorLogged = false;
  OpenSerialPort();
}

void RF2TelemetryInternalsPlugin::Shutdown() {
  CloseSerialPort();
  WriteToLogFile("a", "-SHUTDOWN-");
}

void RF2TelemetryInternalsPlugin::StartSession() {
  WriteToLogFile("a", "--STARTSESSION--");
}

void RF2TelemetryInternalsPlugin::EndSession() {
  WriteToLogFile("a", "--ENDSESSION--");
}

void RF2TelemetryInternalsPlugin::EnterRealtime() {
  mET = 0.0;
  WriteToLogFile("a", "---ENTERREALTIME---");
}

void RF2TelemetryInternalsPlugin::ExitRealtime() {
  WriteToLogFile("a", "---EXITREALTIME---");
}

void RF2TelemetryInternalsPlugin::UpdateTelemetry(const TelemInfoV01& info) {
  long speed_kph = 0;
  long rpm = 0;
  unsigned char gear = 0;
  unsigned char tire_temp_fl = 0;
  unsigned char tire_temp_fr = 0;
  unsigned char tire_temp_rl = 0;
  unsigned char tire_temp_rr = 0;
  unsigned short brake_temp_fl = 0;
  unsigned short brake_temp_fr = 0;
  unsigned short brake_temp_rl = 0;
  unsigned short brake_temp_rr = 0;
  unsigned char water_temp = 0;
  unsigned char oil_temp = 0;
  unsigned long best_lap_ms = 0;
  unsigned long current_lap_ms = 0;
  unsigned short rpm_pct_x10 = 0;
  unsigned char fuel_liters = 0;
  unsigned char fuel_pct = 0;
  unsigned char throttle_pct = 0;
  unsigned char brake_pct = 0;

  speed_kph = static_cast<long>(
      sqrt((info.mLocalVel.x * info.mLocalVel.x) +
           (info.mLocalVel.y * info.mLocalVel.y) +
           (info.mLocalVel.z * info.mLocalVel.z)) *
          3.6 +
      0.5);
  rpm = static_cast<long>(info.mEngineRPM + 0.5);
  gear = ClampGear(info.mGear);
  tire_temp_fl = ClampTireTemp(info.mWheel[0].mTireInnerLayerTemperature[2]);
  tire_temp_fr = ClampTireTemp(info.mWheel[1].mTireInnerLayerTemperature[2]);
  tire_temp_rl = ClampTireTemp(info.mWheel[2].mTireInnerLayerTemperature[2]);
  tire_temp_rr = ClampTireTemp(info.mWheel[3].mTireInnerLayerTemperature[2]);
  brake_temp_fl =
      ClampUnsignedShort(static_cast<long>(info.mWheel[0].mBrakeTemp + 0.5));
  brake_temp_fr =
      ClampUnsignedShort(static_cast<long>(info.mWheel[1].mBrakeTemp + 0.5));
  brake_temp_rl =
      ClampUnsignedShort(static_cast<long>(info.mWheel[2].mBrakeTemp + 0.5));
  brake_temp_rr =
      ClampUnsignedShort(static_cast<long>(info.mWheel[3].mBrakeTemp + 0.5));
  water_temp = ClampTemperatureC(info.mEngineWaterTemp);
  oil_temp = ClampTemperatureC(info.mEngineOilTemp);
  best_lap_ms = mBestLapMilliseconds;
  current_lap_ms = ClampLapMilliseconds(info.mElapsedTime - info.mLapStartET);
  rpm_pct_x10 = ClampRpmPctX10(info.mEngineRPM, info.mEngineMaxRPM);
  fuel_liters = ClampFuelLiters(info.mFuel);
  if (info.mFuelCapacity > 0.0) {
    fuel_pct = ClampPercent(info.mFuel / info.mFuelCapacity);
  }
  throttle_pct = ClampPercent(info.mUnfilteredThrottle);
  brake_pct = ClampPercent(info.mUnfilteredBrake);

  SendTelemetryFrame(gear, ClampUnsignedShort(speed_kph),
                     ClampUnsignedShort(rpm), tire_temp_fl, tire_temp_fr,
                     tire_temp_rl, tire_temp_rr, brake_temp_fl,
                     brake_temp_fr, brake_temp_rl, brake_temp_rr,
                     water_temp, oil_temp, best_lap_ms, current_lap_ms,
                     rpm_pct_x10,
                     fuel_liters, fuel_pct,
                     throttle_pct, brake_pct);
}

void RF2TelemetryInternalsPlugin::UpdateGraphics(const GraphicsInfoV01& info) {
  FILE* fo = fopen("ExampleInternalsGraphicsOutput.txt", "a");
  if (fo != NULL) {
    fprintf(fo, "CamPos=(%.1f,%.1f,%.1f)\n", info.mCamPos.x, info.mCamPos.y,
            info.mCamPos.z);
    fprintf(fo, "CamOri[0]=(%.1f,%.1f,%.1f)\n", info.mCamOri[0].x,
            info.mCamOri[0].y, info.mCamOri[0].z);
    fprintf(fo, "CamOri[1]=(%.1f,%.1f,%.1f)\n", info.mCamOri[1].x,
            info.mCamOri[1].y, info.mCamOri[1].z);
    fprintf(fo, "CamOri[2]=(%.1f,%.1f,%.1f)\n", info.mCamOri[2].x,
            info.mCamOri[2].y, info.mCamOri[2].z);
    fprintf(fo, "HWND=%p\n", info.mHWND);
    fprintf(fo, "Ambient Color=(%.1f,%.1f,%.1f)\n\n", info.mAmbientRed,
            info.mAmbientGreen, info.mAmbientBlue);
    fclose(fo);
  }
}

bool RF2TelemetryInternalsPlugin::CheckHWControl(const char* const controlName,
                                                 double& fRetVal) {
  if (!mEnabled) return (false);

  if (_stricmp(controlName, "LookLeft") == 0) {
    const double headSwitcheroo = fmod(mET, 2.0);
    fRetVal = (headSwitcheroo < 0.5) ? 1.0 : 0.0;
    return (true);
  } else if (_stricmp(controlName, "LookRight") == 0) {
    const double headSwitcheroo = fmod(mET, 2.0);
    fRetVal = ((headSwitcheroo > 1.0) && (headSwitcheroo < 1.5)) ? 1.0 : 0.0;
    return (true);
  }

  return (false);
}

bool RF2TelemetryInternalsPlugin::ForceFeedback(double& forceValue) {
  return (false);
}

void RF2TelemetryInternalsPlugin::UpdateScoring(const ScoringInfoV01& info) {
  long i;

  mBestLapMilliseconds = 0;

  for (i = 0; i < info.mNumVehicles; i++) {
    const VehicleScoringInfoV01& vehicle = info.mVehicle[i];

    if (!vehicle.mIsPlayer) {
      continue;
    }

    mBestLapMilliseconds = ClampLapMilliseconds(vehicle.mBestLapTime);
    break;
  }
}

bool RF2TelemetryInternalsPlugin::RequestCommentary(
    CommentaryRequestInfoV01& info) {
  return (false);
}
