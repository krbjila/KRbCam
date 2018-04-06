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


#define KRbCAM_ACQ_MODE 				4 // 2 for accumulate (use this for frame transfer), 4 for fast kinetics
#define KRbCAM_FT 						TRUE
#define KRbCAM_READ_MODE 				4 // 4 for image
#define KRbCAM_USE_INTERNAL_SHUTTER 	FALSE // controls whether the internal shutter is used.
#define KRbCAM_TRIGGER_MODE 			0 // 1 for External Trigger Mode; 0 for Internal mode
#define KRbCAM_EM_MODE 					0 // 0 is normal EM mode
#define KRbCAM_EXPOSED_ROWS 			512 // Number of exposed rows for kinetics imaging
#define KRbCAM_FK_SERIES_LENGTH 		2 // number of shots in fast kinetics series
#define KRbCAM_OD_SERIES_LENGTH			3 // number of fast kinetics series to obtain an OD image
#define	KRbCAM_FK_BINNING_MODE 			4 // 4 for image mode
#define KRbCAM_N_ACC					1 // Number of accumulations: 1

#define KRbCAM_TIMER					100 // ms for acquisition loop

#define KRbCAM_LOOP_ACQ					FALSE // Loop acquisitions?

#ifndef KRbCAM_FILENAME_BASE
#define KRbCAM_FILENAME_BASE			L"iXon_img"
#endif

// #define KRbCAM_STATUS_IDLE				0 // Idle
// #define KRbCAM_STATUS_ACQ				1 // Acquiring


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

std::vector<HWND> handlesVector;

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
   	
	if (AndorCamera.iXonSetupAcquisition(gConfig) == 0) {
   		if (AndorCamera._StartAcquisition() == DRV_SUCCESS) {
   			gFlagAcquiring = TRUE;
	 		SetTimer(gHandleMain, ID_ACQ_TIMER, KRbCAM_TIMER, NULL);
   		}
   		else {
   			gFlagAcquiring = FALSE;
   			return -1;
   		}
   	}
	return 0;
}


int getImageData(void) {

	unsigned long dataLength = KRbCAM_FK_SERIES_LENGTH * gConfig.width * gConfig.height;
	if (gConfig.binning)
		dataLength /= 4;
	
	if (pImageData != NULL)
		delete pImageData;
	pImageData = new long[dataLength];

	AndorCamera.errorFlag = FALSE;

	long ind1 = 0;
	long ind2 = 0;
	AndorCamera._GetNumberAvailableImages(&ind1, &ind2);
	AndorCamera._GetImages(ind1, ind2, pImageData, dataLength);

	if (!AndorCamera.errorFlag) {
		saveArray(pImageData, dataLength, KRbCAM_FK_SERIES_LENGTH, gConfig.width, gConfig.height);
		return 0;
	}
	else
		return -1;
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
						{
							if (LOWORD(wParam) == ID_EMENABLE_BUTTON) {
								if (Button_GetCheck(hEmEnableButton) == BST_CHECKED) {
									Edit_SetText(hEmGainEdit, L"1");
									Edit_Enable(hEmGainEdit, TRUE);
								}
								else {
									Edit_SetText(hEmGainEdit, L"");
									Edit_Enable(hEmGainEdit, FALSE);
								}
							}
							else if (LOWORD(wParam) == ID_COOLERENABLE_BUTTON || LOWORD(wParam) == ID_COOLERDISABLE_BUTTON) {
								// DO SOMETHING FOR COOLING
							}

							else if (LOWORD(wParam) == ID_GO_BUTTON) {
								if (!gFlagAcquiring) {
									int err = 0;
									err = SetupAcquisition();
									
									if (err == 0) {
										Button_Enable(hStopButton, TRUE);
										Button_Enable(hGoButton, FALSE);
									}
									gCounterODSeries = 0;
								}
							}
							else if (LOWORD(wParam) == ID_STOP_BUTTON) {
								if (gFlagAcquiring) {
									Button_Enable(hGoButton, TRUE);
									Button_Enable(hStopButton, FALSE);
									KillTimer(gHandleMain, ID_ACQ_TIMER);

									int errorVal = AbortAcquisition();
									if (errorVal == DRV_SUCCESS)
										appendToEditControl(hStatusLogEdit, L"Abort successful.\n");
									else if (errorVal == DRV_IDLE)
										appendToEditControl(hStatusLogEdit, L"Abort error: system not acquiring.\n");

									gFlagAcquiring = FALSE;
									gCounterODSeries = 0;
								}
							}
						}
						return 0;
					default:
						return 0;
				}

			}
			return 0;
		case WM_TIMER:
			{
				switch LOWORD(wParam)
				{
					case ID_ACQ_TIMER:
						{
							int status;
							int errorVal = GetStatus(&status);

							if (errorVal != DRV_SUCCESS) {
								KillTimer(GetActiveWindow(), ID_ACQ_TIMER);
								MessageBox(GetActiveWindow(), L"GetStatus error\n", L"Warning", MB_OK);
								
								AbortAcquisition();
								Button_Enable(hGoButton, TRUE);
								Button_Enable(hStopButton, FALSE);
								gFlagAcquiring = FALSE;
							}
							else {
								if (status == DRV_IDLE) {
									KillTimer(gHandleMain, ID_ACQ_TIMER);

									if (gFlagAcquiring) {
										
										gCounterODSeries++;
										getImageData();

										std::wostringstream wss;
										wss << L"Acquired " << gCounterODSeries << L" of " << KRbCAM_OD_SERIES_LENGTH << L" in series.\n";
										appendToEditControl(hStatusLogEdit, wss.str().c_str());

										if (gCounterODSeries < KRbCAM_OD_SERIES_LENGTH) {
											int errorVal = StartAcquisition();
											if (errorVal != DRV_SUCCESS) {
												MessageBox(GetActiveWindow(), L"SetupAcquisition error\n", L"StartAcquisition error.", MB_OK);
												gFlagAcquiring = FALSE;
												return -1;
											}
											else {
												gFlagAcquiring = TRUE;
												SetTimer(gHandleMain, ID_ACQ_TIMER, KRbCAM_TIMER, NULL);
											}
										}
										else {
											gCounterODSeries = 0;
											Button_Enable(hGoButton, TRUE);
											Button_Enable(hStopButton, FALSE);
											appendToEditControl(hStatusLogEdit, L"Done acquiring.\n");
											gFlagAcquiring = FALSE;
										}
									}
									else {
										gCounterODSeries = 0;
										appendToEditControl(hStatusLogEdit, L"Didn't start acquiring!\n");
										Button_Enable(hGoButton, TRUE);
										Button_Enable(hStopButton, FALSE);
										gFlagAcquiring = FALSE;
									}
									
								}
								else if (status != DRV_ACQUIRING) {
									gCounterODSeries = 0;
									appendToEditControl(hStatusLogEdit, L"Error: not acquiring.\n");
									Button_Enable(hGoButton, TRUE);
									Button_Enable(hStopButton, FALSE);
									gFlagAcquiring = FALSE;
								}
							}
						}
					return 0;
				}
			}
			return 0;
		default:
			return 0;
	}
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

	long* temp = new long[frame_length];
	char kinIndex = 'a';
	for (int i=0; i < num_frames; i++) {
		
		memcpy(temp, pImage + i * frame_length, frame_length * sizeof(long));

		std::ofstream save_file;
		std::wostringstream ss;
		ss << gConfig.folderPath.c_str() << KRbCAM_FILENAME_BASE << gConfig.fileNumber << kinIndex << L".csv";

		if (gCounterODSeries == 1)
			save_file.open(ss.str().c_str());
		else
			save_file.open(ss.str().c_str(), std::fstream::app);
		

		for (int j=0; j < y_limit; j++) {
			for (int k=0; k < x_limit; k++) {
				save_file << temp[j * x_limit + k] << ",";
			}
			save_file << "\n";
		}
		save_file.close();
		kinIndex++;
	}
	delete temp;

	if (gCounterODSeries == KRbCAM_OD_SERIES_LENGTH) {
		incrementFileNumber(&gConfig);
		SetConfigFormData(gConfig);
	}

	appendToEditControl(hStatusLogEdit, L"Data saved.\n");

	return 0;
}
