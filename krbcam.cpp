#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

#include <cstring>
#include <cmath>
#include <cwchar>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "atmcd32d.h"

// Layout and setup functions
#include "krbcam_layout.hpp"
#include "krbcam_andor.hpp"
#include "krbcam_helpers.hpp"


// Handles to child windows
HWND 	hAcqStatic,
		hAcqDisplay,
		hKinTimeStatic,
		hKinTimeEdit,
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

AndorCapabilities caps;

config_form_input_t gConfig;

BOOL gFlagVerbose = FALSE; // Verbose for printing status on init to the status edit control
BOOL gFlagAcquiring = FALSE;
BOOL gFlagTempOK = FALSE;
int gCounterODSeries = 0;

extern HWND gHandleMain;

KRbFastKinetics AndorCamera;

long* pImageData = NULL;

camInfo_t camInfo;

// Functions for setup and layout
BOOL _CreateWindows(HINSTANCE hInstance, HWND hParent);
int HandleMessagesControl(UINT uMsg, WPARAM wParam, LPARAM lParam);

int InitializeSDK(void);
int SetupAcquisition(void);
int exitGracefully(void);

int getImageData(void);
int saveArray(long* pImage, int data_length, int num_frames, int width, int height);

int processButtons(int ID);
int processTimers(int ID);


BOOL _CreateWindows(HINSTANCE hInstance, HWND hParent) {
	return CreateWindows(hInstance, hParent);
}

int InitializeSDK(void) {
	AndorCamera.Init(gHandleMain, hStatusLogEdit);
	AndorCamera.flagVerbose = TRUE;
	return AndorCamera.iXonInit();
}

int SetupAcquisition(void) {
	camInfo = AndorCamera.GetCamInfo();

	AndorCamera.iXonArm();

	GetConfigFormData(&gConfig);
   	ValidateConfigValues(&gConfig, camInfo);
   	SetConfigFormData(gConfig);

   	if (AndorCamera.iXonSetupAcquisition(gConfig) != 0) {
   		MessageBox(GetActiveWindow(), L"iXonSetupAcquisition error.", L"Error.", MB_OK);
   		return -1;
   	}

	if (AndorCamera.StartAcquisition() == DRV_SUCCESS) {
		gFlagAcquiring = TRUE;
 		SetTimer(gHandleMain, ID_ACQ_TIMER, KRBCAM_TIMER, NULL);
	}
	else {
		gFlagAcquiring = FALSE;
		return -1;
	}

	return 0;
}


int exitGracefully(void) {

	KillTimer(gHandleMain, ID_ACQ_TIMER);

	if (pImageData != NULL) {
		delete pImageData;
	}

	// Do some cooler stuff here eventually!!

	return 0;
}


int HandleMessagesControl(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		case WM_COMMAND:
			{
				switch (HIWORD(wParam))
				{
					case BN_CLICKED:
						processButtons(LOWORD(wParam));
						return 0;
					default:
						return 0;
				}

			}
			return 0;
		case WM_TIMER:
			processTimers(LOWORD(wParam));
			return 0;
		default:
			return 0;
	}
}


int processButtons(int ID) {

	switch (ID) {
		case ID_EMENABLE_BUTTON:
			{
				if (Button_GetCheck(hEmEnableButton) == BST_CHECKED) {
				Edit_SetText(hEmGainEdit, L"1");
				Edit_Enable(hEmGainEdit, TRUE);
				}
				else {
					Edit_SetText(hEmGainEdit, L"");
					Edit_Enable(hEmGainEdit, FALSE);
				}
			}
			break;

		case ID_COOLERENABLE_BUTTON:
			// do something for cooling
			break;
		case ID_COOLERDISABLE_BUTTON:
			// do something for cooling
			break;

		case ID_GO_BUTTON:
			{
				if (!gFlagAcquiring) {
					int err = 0;
					err = SetupAcquisition();
					
					if (err == 0) {
						Button_Enable(hStopButton, TRUE);
						Button_Enable(hGoButton, FALSE);
					}
					else
						return -1;
					gCounterODSeries = 0;
				}
			}
			break;

		case ID_STOP_BUTTON:
			{
				if (gFlagAcquiring) {
					Button_Enable(hGoButton, TRUE);
					Button_Enable(hStopButton, FALSE);
					KillTimer(gHandleMain, ID_ACQ_TIMER);

					gFlagAcquiring = FALSE;
					gCounterODSeries = 0;

					AndorCamera.AbortAcquisition();
				}
			}
			break;
	}

	return 0;
}


int processTimers(int ID) {
	switch (ID) {
		case ID_ACQ_TIMER:	
			{
				BOOL successFlag = FALSE;

				int status;
				int errorVal = AndorCamera.GetStatus(&status);

				if (errorVal != DRV_SUCCESS) {
					KillTimer(gHandleMain, ID_ACQ_TIMER);
					AndorCamera.AbortAcquisition();
				}
				else {
					// If acquiring, just pass until the next timer
					if (status == DRV_ACQUIRING) {
						gFlagAcquiring = TRUE;
						return 0;
					}

					// Otherwise:
					else if (status == DRV_IDLE) {
						KillTimer(gHandleMain, ID_ACQ_TIMER);

						if (gFlagAcquiring) {
							
							gCounterODSeries++;
							getImageData();

							Sleep(100);

							std::wostringstream wss;
							wss << L"Acquired " << gCounterODSeries << L" of " << KRBCAM_OD_SERIES_LENGTH << L" in series.\n";
							appendToEditControl(hStatusLogEdit, wss.str().c_str());

							if (gCounterODSeries < KRBCAM_OD_SERIES_LENGTH) {
								int errorVal = AndorCamera.StartAcquisition();
								
								if (errorVal == DRV_SUCCESS) {
									gFlagAcquiring = TRUE;
									SetTimer(gHandleMain, ID_ACQ_TIMER, KRBCAM_TIMER, NULL);
									return 0;
								}
							}
							else {
								appendToEditControl(hStatusLogEdit, L"Done acquiring.\n");
								successFlag = TRUE;
							}
						}
						else
							appendToEditControl(hStatusLogEdit, L"Didn't start acquiring!\n");
					}
					else if (status != DRV_ACQUIRING)
						appendToEditControl(hStatusLogEdit, L"Error: not acquiring.\n");
				}

				// If not acquiring, the default behavior
				// is to reset the counter, button states, and acquire flag.
				// If the camera is in the middle of an acquisition or OD
				// sequence, then the function will have already returned by this point.
				gCounterODSeries = 0;
				Button_Enable(hGoButton, TRUE);
				Button_Enable(hStopButton, FALSE);
				gFlagAcquiring = FALSE;
				return successFlag ? 0 : -1;
			}

		default:
			return 0;
	}
}


int getImageData(void) {

	unsigned long dataLength = KRBCAM_FK_SERIES_LENGTH * gConfig.width * gConfig.height;
	if (gConfig.binning)
		dataLength /= 4;
	
	if (pImageData != NULL)
		delete pImageData;
	pImageData = new long[dataLength];

	AndorCamera.errorFlag = FALSE;

	long ind1 = 0;
	long ind2 = 0;
	AndorCamera.GetNumberAvailableImages(&ind1, &ind2);
	AndorCamera.GetImages(ind1, ind2, pImageData, dataLength);

	if (!AndorCamera.errorFlag) {
		saveArray(pImageData, dataLength, KRBCAM_FK_SERIES_LENGTH, gConfig.width, gConfig.height);
		return 0;
	}
	else
		return -1;
}



int saveArray(long* pImage, int data_length, int num_frames, int width, int height) {

	if (GetFileAttributes(gConfig.folderPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
		int err = CreateDirectory(gConfig.folderPath.c_str(), NULL);
		if (err == 0) {
			if (GetLastError() == 3)
				MessageBox(GetActiveWindow(), L"Save folder path not found.", L"Error", MB_OK);
		}
		else if (gFlagVerbose) {
			std::wstring ws;
			ws = L"Directory " + gConfig.folderPath + L" created.\n";
			appendToEditControl(hStatusLogEdit, ws.c_str());
		}
	}

	int frame_length = data_length / num_frames;

	int bins = 1;
	if (gConfig.binning) {
		bins = 2;
	}
	int x_limit = width / bins;
	int y_limit = height / bins;

	char kinIndex = 'a';
	for (int i=0; i < num_frames; i++) {
		
		std::ofstream save_file;
		std::wostringstream ss;
		ss << gConfig.folderPath.c_str() << KRBCAM_FILENAME_BASE << gConfig.fileNumber << kinIndex << L".csv";

		if (gCounterODSeries == 1)
			save_file.open(ss.str().c_str(), std::ofstream::trunc); // Make sure the file is empty
		else
			save_file.open(ss.str().c_str(), std::ofstream::app);
		

		for (int j=0; j < y_limit; j++) {
			for (int k=0; k < x_limit; k++) {
				save_file << pImage[i * frame_length + j*x_limit + k];

				if (k != x_limit - 1)
					save_file << ",";
			}

			save_file << "\n";

		}
		save_file.close();
		kinIndex++;
	}

	if (gCounterODSeries == KRBCAM_OD_SERIES_LENGTH) {
		incrementFileNumber(&gConfig);
		SetConfigFormData(gConfig);
	}

	appendToEditControl(hStatusLogEdit, L"Data saved.\n");

	return 0;
}
