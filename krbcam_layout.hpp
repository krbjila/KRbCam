#ifndef KRBCAM_LAYOUT_HPP
#define KRBCAM_LAYOUT_HPP

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

#include <cstring>
#include <cstdlib>
#include <cwchar>
#include <ctime>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

// Struct defs and macros
// and a few helper functions
#include "krbcam_helpers.hpp"

#define ID_KINTIME_EDIT			0x01
#define ID_EXP_EDIT 			0x02
#define ID_ROIX_EDIT			0x03
#define ID_ROIY_EDIT			0x04
#define ID_ROIWIDTH_EDIT		0x05
#define ID_ROIHEIGHT_EDIT		0x06
#define ID_EMENABLE_BUTTON		0x07
#define ID_EMGAIN_EDIT			0x08
#define ID_SAVEPATH_EDIT		0x09
#define ID_STATUSLOG_EDIT		0x0A
#define ID_GO_BUTTON			0x0B
#define ID_STOP_BUTTON			0x0C
#define ID_COOLERENABLE_BUTTON	0x0D
#define ID_COOLERDISABLE_BUTTON	0x0E
#define ID_BINNING_BUTTON		0x0F

#define ID_ACQ_TIMER			0x10

#define KRBCAM_DEFAULT_SAVEPATH	L"C:\\Users\\KRbG2\\Desktop\\Kyle"

struct window_layout_values_t {
	int originX;
	int originY;
	int width;
	int height;
	int spacingX;
	int spacingY;
};

window_layout_values_t layout = {20, 20, 100, 20, 5, 5};


HFONT hWindowFont = CreateFont(
						16, // Height
						0, // Width (0 is default)
						0, // Escapement angle
						0, // Orientation angle
						FW_NORMAL, // Normal weight
						FALSE, // Italics?
						FALSE, // Underline?
						FALSE, // Strikeout?
						ANSI_CHARSET, // Character set
						OUT_DEFAULT_PRECIS, // Output precision
						CLIP_DEFAULT_PRECIS, // Clipping behavior
						DEFAULT_QUALITY, // Font quality
						DEFAULT_PITCH | FF_DONTCARE, // Font pitch and family
						TEXT("Sergoe UI") 
						);


// Following externs are defined in KRBCAM.cpp
// Handles to child windows
extern HWND 			hAcqStatic,
						hAcqDisplay,
						hExpStatic,
						hExpEdit,
						hRoiXStatic,
						hRoiXEdit,
						hRoiYStatic,
						hRoiYEdit,
						hRoiWidthStatic,
						hRoiWidthEdit,
						hRoiHeightStatic,
						hRoiHeightEdit,
						hBinningStatic,
						hBinningButton,
						hTriggerStatic,
						hTriggerDisplay,
						hEmEnableStatic,
						hEmEnableButton,
						hEmGainStatic,
						hEmGainEdit,
						hSavePathStatic,
						hSavePathEdit,
						hFileNumberStatic,
						hFileNumberEdit,
						hStatusLabelStatic,
						hStatusLogEdit,
						hGoButton,
						hStopButton,
						hCoolerLabelStatic,
						hCoolerStatusStatic,
						hTempLabelStatic,
						hTempStatusStatic,
						hCoolerGroupBox,
						hCoolerEnableButton,
						hCoolerDisableButton;

BOOL CreateWindows(HINSTANCE hInstance, HWND hParent);
BOOL SetDefaultValues(void);
inline int WindowCoords(int xIndex, int yIndex, int coord);
void appendToEditControl(HWND handle, LPCWSTR str);

void GetConfigFormData(config_form_input_t* out);
void SetConfigFormData(config_form_input_t data);


// Vector of handles to all child windows
std::vector<HWND> handlesVector;

/*
* Create Windows.
*
*
*
*/
BOOL CreateWindows(HINSTANCE hInstance, HWND hParent) {
	// Make something

	hAcqStatic = CreateWindow(
		L"STATIC", L"Acq. Mode",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,0,0), WindowCoords(0,0,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hAcqStatic);

	hAcqDisplay = CreateWindow(
		L"STATIC", L"Fast Kinetics",
		WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER,
		WindowCoords(1,0,0), WindowCoords(1,0,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hAcqDisplay);

	hExpStatic = CreateWindow(
		L"STATIC", L"Exposure (ms)",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,1,0), WindowCoords(0,1,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hExpStatic);

	hExpEdit = CreateWindow(
		L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER,
		WindowCoords(1,1,0), WindowCoords(1,1,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_EXP_EDIT, hInstance, NULL
		);
	handlesVector.push_back(hExpEdit);

	hRoiXStatic = CreateWindow(
		L"STATIC", L"X offset (px)",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,3,0), WindowCoords(0,3,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hRoiXStatic);

	hRoiXEdit = CreateWindow(
		L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | WS_BORDER,
		WindowCoords(1,3,0), WindowCoords(1,3,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_ROIX_EDIT, hInstance, NULL
		);
	handlesVector.push_back(hRoiXEdit);

	hRoiYStatic = CreateWindow(
		L"STATIC", L"Y offset (px)",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,4,0), WindowCoords(0,4,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hRoiYStatic);

	hRoiYEdit = CreateWindow(
		L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | WS_BORDER,
		WindowCoords(1,4,0), WindowCoords(1,4,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_ROIY_EDIT, hInstance, NULL
		);
	handlesVector.push_back(hRoiYEdit);

	hRoiWidthStatic = CreateWindow(
		L"STATIC", L"Width (px)",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,5,0), WindowCoords(0,5,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hRoiWidthStatic);

	hRoiWidthEdit = CreateWindow(
		L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | WS_BORDER,
		WindowCoords(1,5,0), WindowCoords(1,5,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_ROIWIDTH_EDIT, hInstance, NULL
		);
	handlesVector.push_back(hRoiWidthEdit);

	hRoiHeightStatic = CreateWindow(
		L"STATIC", L"Height (px)",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,6,0), WindowCoords(0,6,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hRoiHeightStatic);

	hRoiHeightEdit = CreateWindow(
		L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | WS_BORDER,
		WindowCoords(1,6,0), WindowCoords(1,6,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_ROIHEIGHT_EDIT, hInstance, NULL
		);
	handlesVector.push_back(hRoiHeightEdit);

	hBinningStatic = CreateWindow(
		L"STATIC", L"Bin 2x2?",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,7,0), WindowCoords(0,7,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hBinningStatic);

	hBinningButton = CreateWindow(
		L"BUTTON", L"",
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_BORDER,
		WindowCoords(1,7,0), WindowCoords(1,7,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_BINNING_BUTTON, hInstance, NULL
		);
	handlesVector.push_back(hBinningButton);

	hTriggerStatic = CreateWindow(
		L"STATIC", L"Trig. Mode",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,9,0), WindowCoords(0,9,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hTriggerStatic);

	hTriggerDisplay = CreateWindow(
		L"STATIC", L"External",
		WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER,
		WindowCoords(1,9,0), WindowCoords(1,9,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hTriggerDisplay);

	hEmEnableStatic = CreateWindow(
		L"STATIC", L"EM Enable?",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,10,0), WindowCoords(0,10,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hEmEnableStatic);

	hEmEnableButton = CreateWindow(
		L"BUTTON", L"",
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_BORDER,
		WindowCoords(1,10,0), WindowCoords(1,10,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_EMENABLE_BUTTON, hInstance, NULL
		);
	handlesVector.push_back(hEmEnableButton);

	hEmGainStatic = CreateWindow(
		L"STATIC", L"EM Gain",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,11,0), WindowCoords(0,11,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hEmGainStatic);

	hEmGainEdit = CreateWindow(
		L"Edit", L"",
		WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NUMBER | WS_BORDER,
		WindowCoords(1,11,0), WindowCoords(1,11,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_EMGAIN_EDIT, hInstance, NULL
		);
	handlesVector.push_back(hEmGainEdit);

	hSavePathStatic = CreateWindow(
		L"STATIC", L"Save to:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,13,0), WindowCoords(0,13,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hSavePathStatic);

	hSavePathEdit = CreateWindow(
		L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL,
		WindowCoords(1,13,0), WindowCoords(1,13,1),
		3 * layout.width + layout.spacingX, layout.height,
		hParent, (HMENU)ID_SAVEPATH_EDIT, hInstance, NULL
		);
	Edit_LimitText(hSavePathEdit, 60);
	handlesVector.push_back(hSavePathEdit);

	hFileNumberStatic = CreateWindow(
		L"STATIC", L"File number:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,14,0), WindowCoords(0,14,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hFileNumberStatic);

	hFileNumberEdit = CreateWindow(
		L"EDIT", L"0",
		WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_NUMBER,
		WindowCoords(1,14,0), WindowCoords(1,14,1),
		layout.width + layout.spacingX, layout.height,
		hParent, (HMENU)ID_SAVEPATH_EDIT, hInstance, NULL
		);
	handlesVector.push_back(hFileNumberEdit);

	hGoButton = CreateWindow(
		L"BUTTON", L"Update parameters and acquire",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER,
		WindowCoords(0, 16, 0), WindowCoords(0, 16, 1),
		4 * layout.width + layout.spacingX, layout.height,
		hParent, (HMENU)ID_GO_BUTTON, hInstance, NULL
		);
	handlesVector.push_back(hGoButton);

	hStopButton = CreateWindow(
		L"BUTTON", L"Stop acquiring",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER,
		WindowCoords(0, 17, 0), WindowCoords(0, 17, 1),
		4 * layout.width + layout.spacingX, layout.height,
		hParent, (HMENU)ID_STOP_BUTTON, hInstance, NULL
		);
	handlesVector.push_back(hStopButton);

	hStatusLabelStatic = CreateWindow(
		L"STATIC", L"Status log:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(0,19,0), WindowCoords(0,19,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hStatusLabelStatic);

	hStatusLogEdit = CreateWindow(
		L"EDIT", L"Application initialized...\n",
		WS_VISIBLE | WS_CHILD | SS_LEFT | ES_LEFT | ES_READONLY | ES_AUTOVSCROLL | ES_MULTILINE | WS_BORDER,
		WindowCoords(0,20,0), WindowCoords(0,20,1),
		4 * layout.width + layout.spacingX, 4*layout.height,
		hParent, (HMENU)ID_STATUSLOG_EDIT, hInstance, NULL
		);
	handlesVector.push_back(hStatusLogEdit);

	hCoolerGroupBox = CreateWindow(
		L"Button", L"Cooler control:",
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		WindowCoords(2,0,0), WindowCoords(2,0,1),
		2 * (layout.width + layout.spacingX), 2 * (layout.height + layout.spacingY),
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hCoolerGroupBox);

	hCoolerEnableButton = CreateWindow(
		L"Button", L"ON:",
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
		WindowCoords(2,1,0) + layout.spacingX, WindowCoords(2,1,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_COOLERENABLE_BUTTON, hInstance, NULL
		);
	handlesVector.push_back(hCoolerEnableButton);

	hCoolerDisableButton = CreateWindow(
		L"Button", L"OFF:",
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		WindowCoords(3,1,0) + layout.spacingX, WindowCoords(3,1,1),
		layout.width, layout.height,
		hParent, (HMENU)ID_COOLERDISABLE_BUTTON, hInstance, NULL
		);
	handlesVector.push_back(hCoolerDisableButton);

	hCoolerLabelStatic = CreateWindow(
		L"STATIC", L"Cooler status:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(2,3,0), WindowCoords(2,3,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hCoolerLabelStatic);

	hCoolerStatusStatic = CreateWindow(
		L"STATIC", L"OFF",
		WS_VISIBLE | WS_CHILD | SS_LEFT| WS_BORDER,
		WindowCoords(3,3,0), WindowCoords(3,3,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hCoolerStatusStatic);

	hTempLabelStatic = CreateWindow(
		L"STATIC", L"CCD Temp:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		WindowCoords(2,4,0), WindowCoords(2,4,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hTempLabelStatic);

	hTempStatusStatic = CreateWindow(
		L"STATIC", L" \u00B0 C",
		WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER,
		WindowCoords(3,4,0), WindowCoords(3,4,1),
		layout.width, layout.height,
		hParent, NULL, hInstance, NULL
		);
	handlesVector.push_back(hTempStatusStatic);

	for (int j = 0; j < handlesVector.size(); j++ ) {
		SendMessage(handlesVector[j], WM_SETFONT, (WPARAM)hWindowFont, TRUE);
	}

	SetDefaultValues();

	return TRUE;
}

BOOL SetDefaultValues(void) {

	using namespace std;
	wifstream inFile;
	inFile.open("default_config.txt");

	if (!inFile) {
		Edit_SetText(hExpEdit, L"40");
		Edit_SetText(hRoiXEdit, L"0");
		Edit_SetText(hRoiYEdit, L"0");
		Edit_SetText(hRoiWidthEdit, L"500");
		Edit_SetText(hRoiHeightEdit, L"500");
		Edit_SetText(hSavePathEdit, KRBCAM_DEFAULT_SAVEPATH);

		MessageBox(GetActiveWindow(), L"Couldn't open default_config.txt. Loading dummy values...", L"Warning", MB_OK);
	}
	else {
		for (wstring line; getline(inFile, line); ) {
			wstring key = line.substr(0, line.find(' '));
			wstring val = line.substr(line.find(' ') + 1);

			if (key == L"expTime")
				Edit_SetText(hExpEdit, val.c_str());
			else if (key == L"x_off")
				Edit_SetText(hRoiXEdit, val.c_str());
			else if (key == L"y_off")
				Edit_SetText(hRoiYEdit, val.c_str());
			else if (key == L"width")
				Edit_SetText(hRoiWidthEdit, val.c_str());
			else if (key == L"height")
				Edit_SetText(hRoiHeightEdit, val.c_str());
			else if (key == L"savePath") {
				char buffer[MAX_PATH];
				time_t rawtime;
				time(&rawtime);
				struct tm* timeinfo = localtime(&rawtime); 
				strftime(buffer, MAX_PATH, "%Y%m%d", timeinfo);

				std::wostringstream wss;
				if (val.compare(val.length() - 1, 1, L"\\") != 0) {
					val += L"\\";
				}
				wss << val.c_str() << buffer << L"\\";

				Edit_SetText(hSavePathEdit, wss.str().c_str());
			}
			else if (key == L"binning") {
				std::stringstream ss;
				ss << val.c_str();

				if (atoi(ss.str().c_str())) {
					Button_SetCheck(hBinningButton, TRUE);
				}
				else
					Button_SetCheck(hBinningButton, FALSE);
			}
		}

		inFile.close();
	}
	
	wchar_t buffer[MAX_PATH];
	Edit_GetText(hSavePathEdit, buffer, MAX_PATH);
	if (GetFileAttributes(buffer) == INVALID_FILE_ATTRIBUTES) {
		Edit_SetText(hFileNumberEdit, L"0");
	}
	else {
		std::wstring wstr = buffer;
		wstr += KRBCAM_FILENAME_BASE + std::wstring(L"*");
		HANDLE fileHandle;
		WIN32_FIND_DATA fileData;

		fileHandle = FindFirstFile(wstr.c_str(), &fileData);
		std::wstring temp(L"");
		int l1 = std::wstring(KRBCAM_FILENAME_BASE).length();
		int index = 0;
		while (FindNextFile(fileHandle, &fileData)) {
			temp = fileData.cFileName;
			int l2 = temp.length();
			l2 -= temp.find(L".csv"); 
			temp = temp.substr(l1, l2);
			int temp_index = wcstol(temp.c_str(), NULL, 10);

			if (temp_index > index)
				index = temp_index;
		}

		std::wostringstream wss;
		wss << index + 1;
		Edit_SetText(hFileNumberEdit, wss.str().c_str());
	}


	Button_SetCheck(hEmEnableButton, BST_UNCHECKED);
	Edit_SetText(hEmGainEdit, L"1");
	Edit_Enable(hEmGainEdit, FALSE);

	Button_SetCheck(hCoolerDisableButton, BST_CHECKED);

//	Button_Enable(hGoButton, FALSE);
	Button_Enable(hStopButton, FALSE);

	return TRUE;
}


void appendToEditControl(HWND handle, LPCWSTR str) {
	int currLength = GetWindowTextLength(handle);
	BOOL flag = FALSE;

	if (GetWindowLong(handle, GWL_STYLE) == ES_READONLY) {
		flag = TRUE;
		Edit_SetReadOnly(handle, FALSE);
	}
	Edit_SetSel(handle, currLength, currLength);
	Edit_ReplaceSel(handle, str);
	SendMessage(handle, WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);
	if (flag) {
		Edit_SetReadOnly(handle, TRUE);
	}
}

// Assuming a grid layout, get the x,y positions for subwindows on
// different positions on the grid.
inline int WindowCoords(int xIndex, int yIndex, int coord) {
	int x = layout.originX + xIndex * (layout.width);
	if (x > 0 && x % 2 == 0)
		x += (xIndex / 2) * layout.spacingX;
	int y = layout.originY + yIndex * (layout.height + layout.spacingY);

	return (coord == 0 ? x : y); 
}


void GetConfigFormData(config_form_input_t* out) {
	wchar_t buffer[100];
	
	Edit_GetText(hExpEdit, buffer, 100);
	std::wstring ws = buffer;
	out->expTime = wcstod(ws.c_str(), NULL);

	Edit_GetText(hRoiXEdit, buffer, 100);
	ws = buffer;
	out->xOffset = (int)wcstol(ws.c_str(), NULL, 10);

	Edit_GetText(hRoiYEdit, buffer, 100);
	ws = buffer;
	out->yOffset = (int)wcstol(ws.c_str(), NULL, 10);

	Edit_GetText(hRoiWidthEdit, buffer, 100);
	ws = buffer;
	out->width = (int)wcstol(ws.c_str(), NULL, 10);

	Edit_GetText(hRoiHeightEdit, buffer, 100);
	ws = buffer;
	out->height = (int)wcstol(ws.c_str(), NULL, 10);


	if (Button_GetCheck(hEmEnableButton) == BST_CHECKED) {
		out->emEnable = TRUE;
		Edit_GetText(hEmGainEdit, buffer, 100);
		ws = buffer;;
		out->emGain = (int)wcstol(ws.c_str(), NULL, 10);
	}
	else {
		out->emEnable = FALSE;
		out->emGain = 1;
	}

	if (Button_GetCheck(hBinningButton) == BST_CHECKED) {
		out->binning = TRUE;
	}
	else
		out->binning = FALSE;

	Edit_GetText(hSavePathEdit, buffer, 100);
	out->folderPath = std::wstring(buffer);

	Edit_GetText(hFileNumberEdit, buffer, 100);
	ws = buffer;
	out->fileNumber = (int)wcstol(ws.c_str(), NULL, 10);
}


void SetConfigFormData(config_form_input_t data) {
	std::wostringstream ss;

	ss.str(L"");
	ss << data.expTime;
	Edit_SetText(hExpEdit, (LPCWSTR)ss.str().c_str());
	
	ss.str(L"");
	ss << data.xOffset;
	Edit_SetText(hRoiXEdit, (LPCWSTR)ss.str().c_str());

	ss.str(L"");
	ss << data.yOffset;
	Edit_SetText(hRoiYEdit, (LPCWSTR)ss.str().c_str());

	ss.str(L"");
	ss << data.width;
	Edit_SetText(hRoiWidthEdit, (LPCWSTR)ss.str().c_str());

	ss.str(L"");
	ss << data.height;
	Edit_SetText(hRoiHeightEdit, (LPCWSTR)ss.str().c_str());

	if (data.emEnable) {
		ss.str(L"");
		ss << data.emGain;
		Edit_SetText(hEmGainEdit, (LPCWSTR)ss.str().c_str());
		Button_SetCheck(hEmEnableButton, BST_CHECKED);
	}
	else {
		Edit_SetText(hEmGainEdit, L"1");
		Button_SetCheck(hEmEnableButton, BST_UNCHECKED);
	}

	if (data.binning) 
		Button_SetCheck(hBinningButton, BST_CHECKED);
	else
		Button_SetCheck(hBinningButton, BST_UNCHECKED);

	ss.str(L"");
	ss << data.fileNumber;
	Edit_SetText(hFileNumberEdit, (LPCWSTR)ss.str().c_str());

}




#endif