#pragma once

HEADER_TOP

#define MAX_CONTROLLERS 4  // XInput handles up to 4 controllers 
#define INPUT_DEADZONE  ( 0.24f * FLOAT(0x7FFF) )  // Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.
typedef int(*feParseEmuButtonCallback)(const char* szEmuButtonName);

feresult feInputsInitialize(void* iWindowHwnd, feParseEmuButtonCallback _pParseEumuButtonCallback);
int feInputsPoll(int iPlayerIdx, int _iEmuButton);
feresult feInputsLoadPlayerBindings(int iPlayer, const char* szConfig);
feresult feInputsUpdate();

HEADER_BOTTOM