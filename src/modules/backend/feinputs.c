
#include "fepch.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#include <dinput.h>
#include "feinputs.h"
#include "cJSON.h"

#define IS4DIRECTIONS 1
#define I_MAX_PLAYER_BINDINGS_COUNT 128
#define I_MAX_EMULATOR_BUTTONS 512
#define I_MAX_PLAYERS 8
#define I_MAX_DEVICES 32

typedef struct feSInputBinding
{
	int	m_iEmuButton;
	int	m_iPadButton;
	int	m_iPadPov;
	
	int	m_iPlayerIndex;

} feSInputBinding;

typedef struct feSDInputDevice
{
	GUID					m_oGuid;
	LPDIRECTINPUTDEVICEA	m_pDevice;
	DIJOYSTATE2				m_oState;
} feSDInputDevice;

typedef struct feSPlayerInputs
{
	GUID					m_oDeviceGuid;
	int						m_iBindingsCount;
	feSInputBinding			m_aBindings[I_MAX_PLAYER_BINDINGS_COUNT];
	int						m_iDeviceIndex;

	int						m_bIsPlugged;
} feSPlayerInputs;

LPDIRECTINPUT				g_pDinput = 0;
int							g_pDinput_version = 0;
int							g_iDevicesCount = 0;
feSDInputDevice				g_aDInputDevices[I_MAX_DEVICES];
feSPlayerInputs				g_aPlayersInputs[I_MAX_PLAYERS];
feParseEmuButtonCallback	g_pOnParseEmuButton;

int padComputePov(int iPov, int iValue)
{
	switch (iValue)
	{
		case 8: return iPov == 0		|| (IS4DIRECTIONS && (iPov == 31500 || iPov == 4500));	//N or 4DIR NW NE
		case 2: return iPov == 18000	|| (IS4DIRECTIONS && (iPov == 22500 || iPov == 13500)); //S or 4DIR SW SE
		case 4: return iPov == 27000	|| (IS4DIRECTIONS && (iPov == 31500 || iPov == 22500)); //W or 4DIR NW SW
		case 6: return iPov == 9000		|| (IS4DIRECTIONS && (iPov == 4500 || iPov == 13500));	//E or 4DIR NE SE

		case 7: return iPov == 31500;	//NW		
		case 9: return iPov == 4500;	//NE		
		case 3: return iPov == 13500;	//SE
		case 1: return iPov == 22500;	//SW
	
		default: break;
	}

	return 0;
}
BOOL CALLBACK padEnumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
	HRESULT hr;
	LPDIRECTINPUTDEVICEA Pad;

	hr = g_pDinput->lpVtbl->CreateDevice(g_pDinput, &instance->guidInstance, &Pad, NULL);

	if (FAILED(hr))
		return DIENUM_CONTINUE;
	
	g_aDInputDevices[g_iDevicesCount].m_pDevice	= Pad;
	g_aDInputDevices[g_iDevicesCount].m_oGuid		=instance->guidInstance;

	g_iDevicesCount++;

	FE_PRINT("\t\tAdded intput pad %d (%s)", g_iDevicesCount, instance->tszProductName);

	if (g_iDevicesCount == I_MAX_DEVICES)
		return DIENUM_STOP;
	else
		return DIENUM_CONTINUE;
}
HRESULT padGetState(LPDIRECTINPUTDEVICEA _pPad, DIJOYSTATE2 *_pState)
{
	HRESULT hr = S_OK;

	hr = _pPad->lpVtbl->GetDeviceState(_pPad, sizeof(DIJOYSTATE2), _pState);

	if (FAILED(hr)) {
		hr = _pPad->lpVtbl->Acquire(_pPad);
		while (hr == DIERR_INPUTLOST)
		{
			hr = _pPad->lpVtbl->Acquire(_pPad);
		}

		// If we encounter a fatal error, return failure.
		if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
			return E_FAIL;
		}

		// If another application has control of this device, return successfully.
		// We'll just have to wait our turn to use the Pad.
		if (hr == DIERR_OTHERAPPHASPRIO) {
			return S_OK;
		}
	}

	return S_OK;
}

feSInputBinding* playerGetBindedButton(feSPlayerInputs* _pPlayerInputs, int _iEmuButton)
{
	feSInputBinding* pBinding = 0;

	for (int i = 0; i < I_MAX_PLAYER_BINDINGS_COUNT; ++i)
	{
		if (_pPlayerInputs->m_aBindings[i].m_iEmuButton == _iEmuButton)
		{
			pBinding = &(_pPlayerInputs->m_aBindings[i]);
			break;
		}
	}

	return pBinding;
}
void playerInputsClear(feSPlayerInputs* _pPlayer)
{
	_pPlayer->m_bIsPlugged = false;
	_pPlayer->m_iDeviceIndex = -1;
	
	for (int i = 0; i < I_MAX_PLAYER_BINDINGS_COUNT; ++i)
	{
		_pPlayer->m_aBindings[i].m_iEmuButton	= -1;
		_pPlayer->m_aBindings[i].m_iPadButton	= -1;
		_pPlayer->m_aBindings[i].m_iPadPov		= -1;
	}
}

feresult feInputsUpdate()
{
	for (int i = 0; i < g_iDevicesCount; ++i)
	{
		LPDIRECTINPUTDEVICEA Pad = g_aDInputDevices[i].m_pDevice;
		DIJOYSTATE2* pJoyState = &g_aDInputDevices[i].m_oState;

		if (FAILED(padGetState(Pad, pJoyState)))
		{
			//FE_PRINT_ERROR("feInputsupdate failed polling Pad %d", i);
			return FE_RESULT_ERROR;
		}
	}
	return FE_RESULT_OK;
}
void feComputeActualPlayerIndex(int* _pPlayerIdx, int _iEmuButton)
{
	int bFound = false;

	for (int i = 0; i < I_MAX_PLAYERS; ++i)
	{
		feSPlayerInputs* pPlayer = &g_aPlayersInputs[i];

		if (pPlayer->m_bIsPlugged)
		{
			for (int iBindIdx = 0; iBindIdx < pPlayer->m_iBindingsCount; ++iBindIdx)
			{
				feSInputBinding* pBind = &pPlayer->m_aBindings[iBindIdx];

				if (pBind->m_iEmuButton == _iEmuButton && pBind->m_iPlayerIndex != -1)
				{
					*_pPlayerIdx = pBind->m_iPlayerIndex;
					bFound = true;
				}

				if (bFound)
					break;
			}
		}
	}
}
int feInputsPoll(int _iPlayerIdx, int _iEmuButton)
{
	feComputeActualPlayerIndex(&_iPlayerIdx, _iEmuButton);

	int iState = false;
	feSPlayerInputs* pPlayer = &g_aPlayersInputs[_iPlayerIdx];
	
	if (pPlayer->m_bIsPlugged)
	{
		feSDInputDevice* pDevice = &g_aDInputDevices[pPlayer->m_iDeviceIndex];
		feSInputBinding* pButtonBinding = playerGetBindedButton(pPlayer, _iEmuButton);

		if (pButtonBinding)
		{
			if (pButtonBinding->m_iPadButton != -1)
				iState = pDevice->m_oState.rgbButtons[pButtonBinding->m_iPadButton] > 0;
			else
				iState = padComputePov(pDevice->m_oState.rgdwPOV[0], pButtonBinding->m_iPadPov);
		}

	}
	return iState;
}
int getDInputDeviceIndexFromGuid(GUID _oGuid)
{
	int iDeviceIndex = -1;

	for (int i = 0; i < g_iDevicesCount; ++i)
	{
		const feSDInputDevice* pDInputDevice = &g_aDInputDevices[i];

		if (pDInputDevice->m_oGuid.Data1 == _oGuid.Data1 &&
			pDInputDevice->m_oGuid.Data2 == _oGuid.Data2 &&
			pDInputDevice->m_oGuid.Data3 == _oGuid.Data3 &&
			(strcmp(pDInputDevice->m_oGuid.Data4, _oGuid.Data4) == 0)
			)
		{
			iDeviceIndex = i;
			break;
		}
	}
	
	return iDeviceIndex;
}

void parseGuid(GUID* pOut, const char* _szGuid)
{
	unsigned long p0;
	int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;

	int err = sscanf_s(_szGuid, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);

	pOut->Data1 = p0;
	pOut->Data2 = p1;
	pOut->Data3 = p2;
	pOut->Data4[0] = p3;
	pOut->Data4[1] = p4;
	pOut->Data4[2] = p5;
	pOut->Data4[3] = p6;
	pOut->Data4[4] = p7;
	pOut->Data4[5] = p8;
	pOut->Data4[6] = p9;
	pOut->Data4[7] = p10;
}

void parsePlayerInputs(feSPlayerInputs* pOut, const char* _szJson)
{
	cJSON* pJsonRoot = cJSON_Parse(_szJson);

	const char* szGuid = cJSON_GetObjectItem(pJsonRoot, "DeviceGuid")->valuestring;
	
	cJSON* pJsonBindings = cJSON_GetObjectItem(pJsonRoot, "Bindings");

	int iBindingsCount = cJSON_GetArraySize(pJsonBindings);
	
	for (int i = 0; i < iBindingsCount; ++i)
	{
		cJSON* pJsonBinding = cJSON_GetArrayItem(pJsonBindings, i);
		
		const char* szEmuButton = cJSON_GetObjectItem(pJsonBinding, "EmuButton")->valuestring;
		
		int iEmuButton = g_pOnParseEmuButton(szEmuButton);
		int iPlayerIdx = cJSON_GetObjectItem(pJsonBinding, "Player")->valueint;
		int iPadButton = cJSON_GetObjectItem(pJsonBinding, "PadButton")->valueint;
		int iPovButton = cJSON_GetObjectItem(pJsonBinding, "PovButton")->valueint;

		pOut->m_aBindings[i].m_iPlayerIndex = iPlayerIdx;
		pOut->m_aBindings[i].m_iEmuButton	= iEmuButton;
		pOut->m_aBindings[i].m_iPadButton	= iPadButton;
		pOut->m_aBindings[i].m_iPadPov		= iPovButton;
	}

	pOut->m_iBindingsCount = iBindingsCount;
	parseGuid(&pOut->m_oDeviceGuid, szGuid);

}
feresult feInputsLoadPlayerBindings(int iPlayer, const char* szConfig)
{
	feSPlayerInputs* pPlayerToSet = &g_aPlayersInputs[iPlayer];
	parsePlayerInputs(pPlayerToSet, szConfig);

	pPlayerToSet->m_iDeviceIndex = getDInputDeviceIndexFromGuid(pPlayerToSet->m_oDeviceGuid);
	
	if (pPlayerToSet->m_iDeviceIndex != -1)
	{
		pPlayerToSet->m_bIsPlugged = true;
	}

	return FE_RESULT_OK;
}

feresult feInputsInitialize(void* iWindowHwnd, feParseEmuButtonCallback _pParseEumuButtonCallback)
{
	FE_PRINT("Initialize inputs...");

	g_pOnParseEmuButton = _pParseEumuButtonCallback;
	
	// clear devices
	memset(&g_aDInputDevices, 0, sizeof(feSDInputDevice)*I_MAX_DEVICES);
	
	// clear players
	for (int i = 0; i < I_MAX_PLAYERS; ++i)
		playerInputsClear(&g_aPlayersInputs[i]);
	
	// setup direct input library
	HRESULT hr = S_OK;
	int didevtype_keyboard = DI8DEVCLASS_KEYBOARD;
	int didevtype_mouse = DI8DEVCLASS_POINTER;
	int didevtype_pad = DI8DEVCLASS_GAMECTRL;
	HWND hWnd = (HWND)iWindowHwnd;

	g_pDinput_version = DIRECTINPUT_VERSION;
	HINSTANCE iWndInstance = GetModuleHandle(NULL);

	hr = DirectInput8Create(iWndInstance, g_pDinput_version, &IID_IDirectInput8, (void **)&g_pDinput, NULL);
	if (FAILED(hr))
	{
		FE_PRINT_ERROR("inputs DirectInput8Create failed !");
		return FE_RESULT_ERROR;
	}

	// fetch plugged devices
	hr = g_pDinput->lpVtbl->EnumDevices(g_pDinput, DI8DEVCLASS_GAMECTRL, padEnumCallback, NULL, DIEDFL_ATTACHEDONLY);
	if (FAILED(hr))
	{
		FE_PRINT_ERROR("inputs DInput EnumDevices failed !");
		return FE_RESULT_ERROR;
	}

	for (int i = 0; i < g_iDevicesCount; ++i)
	{
		//DIDEVCAPS capabilities;
		LPDIRECTINPUTDEVICEA Pad = g_aDInputDevices[i].m_pDevice;

		// Set the data format
		hr = Pad->lpVtbl->SetDataFormat(Pad, &c_dfDIJoystick2);

		if (!FAILED(hr))
			FE_PRINT_ERROR("inputs DInput SetDataFormat failed !")

		if (hWnd != 0)
		{
			hr = Pad->lpVtbl->SetCooperativeLevel(Pad, hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

			if (FAILED(hr))
				FE_PRINT_ERROR("inputs DInput SetCooperativeLevel failed !")
		}
		// Determine how many axis the Pad has (so we don't error out setting
		// properties for unavailable axis)
		//capabilities.dwSize = sizeof(DIDEVCAPS);

		//if (FAILED(hr = Pad->GetCapabilities(&capabilities))) {
		//	return FE_RESULT_ERROR;
		//}

		//if (FAILED(hr = Pad->EnumObjects(enumAxesCallback, NULL, DIDFT_AXIS))) {
		//	return FE_RESULT_ERROR;
		//}
	}

	FE_PRINT("\t\tFound %d gamepads.", g_iDevicesCount)

	if (g_iDevicesCount == 0) 
	{
		FE_PRINT_ERROR("inputs no devices found !")
		return FE_RESULT_ERROR;
	}

	return FE_RESULT_OK;

}
//BOOL CALLBACK feInputspadEnumAxisCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context)
//{
//	HWND hDlg = (HWND)context;
//
//	DIPROPRANGE propRange;
//	propRange.diph.dwSize = sizeof(DIPROPRANGE);
//	propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
//	propRange.diph.dwHow = DIPH_BYID;
//	propRange.diph.dwObj = instance->dwType;
//	propRange.lMin = -1000;
//	propRange.lMax = +1000;
//
//	// Set the range for the axis
//	if (FAILED(Pad->SetProperty(DIPROP_RANGE, &propRange.diph))) {
//		return DIENUM_STOP;
//	}
//
//	return DIENUM_CONTINUE;
//}
