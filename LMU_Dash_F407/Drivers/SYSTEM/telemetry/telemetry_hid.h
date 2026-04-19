#ifndef __TELEMETRY_HID_H
#define __TELEMETRY_HID_H

#include "stdint.h"

void telemetry_hid_apply_report(const uint8_t *report, uint16_t report_len);

#endif
