#ifndef KRBCAM_ANDOR_HPP
#define KRBCAM_ANDOR_HPP

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


#define KRbCAM_ACQ_MODE 				4 // 2 for accumulate (use this for frame transfer), 4 for fast kinetics
#define KRbCAM_FT 						TRUE
#define KRbCAM_READ_MODE 				4 // 4 for image
#define KRbCAM_USE_INTERNAL_SHUTTER 	FALSE // controls whether the internal shutter is used.
#define KRbCAM_TRIGGER_MODE 			0 // 1 for External Trigger Mode; 0 for Internal mode
#define KRbCAM_EM_MODE 				0 // 0 is normal EM mode
#define KRbCAM_EXPOSED_ROWS 			512 // Number of exposed rows for kinetics imaging
#define KRbCAM_FK_SERIES_LENGTH 		2 // number of shots in fast kinetics series
#define KRbCAM_OD_SERIES_LENGTH		3 // number of fast kinetics series to obtain an OD image
#define	KRbCAM_FK_BINNING_MODE 		4 // 4 for image mode
#define KRbCAM_N_ACC					1 // Number of accumulations: 1

#define KRbCAM_BIN_SIZE				2 // binning is 2x2
#define KRbCAM_TIMER					100 // ms for acquisition loop

#define KRbCAM_LOOP_ACQ				FALSE // Loop acquisitions?

#ifndef KRbCAM_FILENAME_BASE
#define KRbCAM_FILENAME_BASE			L"iXon_img"
#endif

using namespace std;


// Struct for holding other Andor information
struct camInfo_t {
	char model[32]; // Head model number
	int detDim[2]; // {x, y} dimensions in pixels
	BOOL internalShutter;
	int shutterMinT[2]; // {closing time, opening time}
	int emGainRange[2]; // {emLow, emHigh}
};


class iXonClass {
protected:
	AndorCapabilities caps;
	camInfo_t camInfo;
	HWND hMainWindow;
	HWND hStatusLogEdit;
public:
	BOOL flagVerbose;
	BOOL errorFlag;
	wstring errorMsg;
	
	iXonClass() {}

	void Init(HWND main, HWND log) {
		hMainWindow = main;
		hStatusLogEdit = log;
		flagVerbose = FALSE;
		errorFlag = FALSE;
		errorMsg = L"";
	}

	~iXonClass() {
		int status = GetStatus(&status);
		if (status == DRV_ACQUIRING) {
			int errorVal = AbortAcquisition();
			if (errorVal != DRV_SUCCESS)
				MessageBox(GetActiveWindow(), L"Error aborting acquisition while closing.", L"Warning.", MB_OK);
			else
				MessageBox(GetActiveWindow(), L"Successfully aborted acquisition while closing.", L"Exit message.", MB_OK);
		}
		int errorVal = SetShutter(1, 2, 0, 0);
		if (errorVal != DRV_SUCCESS)
			MessageBox(GetActiveWindow(), L"Error closing shutter while exiting.", L"Warning.", MB_OK);

		if (ShutDown() == DRV_SUCCESS)
			MessageBox(GetActiveWindow(), L"Shut down successfully.", L"Exit message.", MB_OK);
		else
			MessageBox(GetActiveWindow(), L"There was an error shutting down.", L"Exit message.", MB_OK);
	}	

	int iXonInit(void) {
		wstring errorMsg(L"");
		wostringstream wss;

		errorFlag = FALSE;

		// Get current directory for Andor
		wchar_t buffer[256];
		GetCurrentDirectory(256, buffer);

		_Initialize(buffer);
		_GetCapabilities(&caps);
		_GetHeadModel(camInfo.model);
		_GetDetector(&camInfo.detDim[0], &camInfo.detDim[1]);
		_GetShutterAvailable(&camInfo.internalShutter);
		_GetShutterMinTimes(&camInfo.shutterMinT[0], &camInfo.shutterMinT[1]);
		
		if (errorFlag) {
			MessageBox(GetActiveWindow(), errorMsg.c_str(), L"Error.", MB_OK);
			return -1;
		}

		StartCooler();
		return 0;
	}

	int iXonArm(void) {
		return 0;
	}

	int iXonSetupAcquisition(void) {
		return 0;
	}

	int _Initialize(LPCWSTR path) {
		int errorVal = Initialize((char*)path);
		errorMsg = handleErrors(errorVal, errorMsg, L"Init. error: ", L"SDK initialized.\n", TRUE);
		return errorVal;
	}
	int _GetCapabilities(AndorCapabilities* capsOut) {
		capsOut->ulSize = sizeof(AndorCapabilities);
		int errorVal = GetCapabilities(capsOut);
		errorMsg = handleErrors(errorVal, errorMsg, L"GetCapabilities error: ", L"", FALSE);
		return errorVal;
	}
	void _PrintCameraType(BOOL printFlag) {
		if (printFlag) {
			switch (caps.ulCameraType) {
				case (AC_CAMERATYPE_IXONULTRA):
					appendToEditControl(hStatusLogEdit, L"iXon Ultra connected.\n");
					break;
				case (AC_CAMERATYPE_IXON):
					appendToEditControl(hStatusLogEdit, L"iXon connected.\n");
					break;
				default:
					appendToEditControl(hStatusLogEdit, L"Some kind of camera (not an iXon!) connected.\n");
					break;
			}
		}
	}
	int _GetHeadModel(char* model) {
		wostringstream ss;
		int errorVal = GetHeadModel(model);
		ss << "Head model is " << camInfo.model << ".\n";
		handleErrors(errorVal, errorMsg, L"GetHeadModel error: ", ss.str().c_str(), flagVerbose);
		return errorVal;
	}
	int _GetDetector(int* dimX, int* dimY) {
		int errorVal = GetDetector(dimX, dimY);
		std::wostringstream ss;
	  	ss << "Array is " << *dimX << " x " << *dimY << " pixels.\n";
	 	handleErrors(errorVal, errorMsg, L"GetDetector error: ", ss.str().c_str(), flagVerbose);
	 	return errorVal;
	}
	int _GetNumberVSSpeeds(int* num) {
		int errorVal = GetNumberVSSpeeds(num);
		std::wostringstream ss;
	  	ss << "Number of vertical shift speeds: " << *num << ".\n";
	 	std::wstring wstr = ss.str();
	    handleErrors(errorVal, errorMsg, L"GetNumberVSSpeeds error: ", ss.str().c_str(), flagVerbose);
	    return errorVal;
	}

	int _GetShutterAvailable(BOOL* internalShutter) {
		int shutter = 0;
		int errorVal = IsInternalMechanicalShutter(&shutter);
		handleErrors(errorVal, errorMsg, L"IsInternalMechanicalShutter error: ",
					(shutter == 0 ? L"No internal shutter.\n" : L"Has internal shutter.\n"), flagVerbose);
		if (shutter != 0)
			*internalShutter = TRUE;
		else
			*internalShutter = FALSE;
		return errorVal;
	}

	int _GetShutterMinTimes(int* closing, int* opening) {
		int errorVal = GetShutterMinTimes(closing, opening);
		std::wostringstream ss;
		ss << "Minimum shutter closing (opening) time: " << *closing << " (" << *opening << ") ms.\n";
		handleErrors(errorVal, errorMsg, L"GetShutterMinTimes error :", ss.str().c_str(), flagVerbose);
		return errorVal;
	}

	int _SetShutter(BOOL useShutter, int closing, int opening) {
		int mode = (useShutter ? 0 : 1);

		wostringstream ss;
		ss << "Internal shutter closing (opening) time set to " << closing << " (" << opening << ") ms.\n";

		int errorVal = SetShutter(1, mode, closing, opening);
		handleErrors(errorVal, errorMsg, L"SetShutter error: ",
					(useShutter ? ss.str().c_str() : L"Internal shutter set to always open.\n"), flagVerbose);
		return errorVal;
	}
	int _SetTriggerMode(int mode) {
		wstring w(L"");
		switch (mode) {
			case 0:
				w = L"Internal trigger mode.\n";
				break;
			case 1:
				w = L"External trigger mode.\n";
				break;
			default:
				w = L"Other trigger mode.\n";
				break;
		}

		int errorVal = SetTriggerMode(mode);
		handleErrors(errorVal, errorMsg, L"SetTriggerMode error: ", w.c_str(), flagVerbose);
		return errorVal;
	}
	int _SetEMGainMode(int mode) {
		wstring w(L"");
		switch (mode) {
			case 0:
				w = L"EM Gain set to default mode (0-255 range).\n";
				break;
			case 1:
				w = L"EM Gain set to 0-4095 range.\n";
				break;
			case 2:
				w = L"EM Gain set to Linear mode.\n";
				break;
			case 3:
				w = L"EM Gain set to Real EM gain mode.\n";
				break;
		}

		int errorVal = SetEMGainMode(mode);
		handleErrors(errorVal, errorMsg, L"SetEMGainMode error: ", w.c_str(), flagVerbose);
		return errorVal;
	}
	int _GetEMGainRange(int* low, int* high) {
		int errorVal = GetEMGainRange(low, high);
		wostringstream ss;
		ss << "EM gain range is " << *low << " to " << *high << ".\n";
		handleErrors(errorVal, errorMsg, L"GetEMGainRange error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}
	int _SetEMCCDGain(int gain) {
		int errorVal = SetEMCCDGain(gain);
		wostringstream ss;
		ss << L"EM Gain set to " << gain << L".\n";
		handleErrors(errorVal, errorMsg, L"SetEMCCDGain error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}

	int _SetAcquisitionMode(int mode) {
		wstring w(L"");
		switch (mode) {
			case 1:
				w = L"Single Scan.\n";
				break;
			case 2:
				w = L"Accumulate.\n";
				break;
			case 3:
				w = L"Kinetics.\n";
				break;
			case 4:
				w = L"Fast Kinetics.\n";
				break;
			case 5:
				w = L"Run until abort.\n";
				break;
		}
		w = L"Acquisition mode set to " + w;

		int errorVal = SetAcquisitionMode(mode);
		handleErrors(errorVal, errorMsg, L"SetAcquisitionMode error: ", w.c_str(), TRUE);
		return errorVal;
	}

	int _SetReadMode(int mode) {
		wstring w(L"");
		switch (mode) {
			case 0:
    			w = L"FVB.\n";
    			break;
    		case 1:
    			w = L"Multi-Track.\n";
    			break;
    		case 2:
    			w = L"Random-Track.\n";
    			break;
    		case 3:
    			w = L"Single-Track.\n";
    			break;
    		case 4:
    			w = L"Image.\n";
    			break;
		}
		w = L"Read mode set to " + w;
		int errorVal = SetReadMode(mode);
		handleErrors(errorVal, errorMsg, L"SetReadMode error: ", w.c_str(), TRUE);
		return errorVal;
	}

	// height in physical pixels, exposure in ms
	int _SetupFastKinetics(int height, int series_length, float exposure, int binning, int y_offset) {
		exposure *= pow(10.0, -3.0);

		int errorVal = SetFastKineticsEx(height, series_length, exposure, 4, binning, binning, y_offset);
		handleErrors(errorVal, errorMsg, L"SetFastKineticsEx error: ", L"Fast kinetics set.\n", flagVerbose);
		return errorVal;
	}

	int _GetFKExposureTime(float* time) {
		*time = 0;
		int errorVal = GetFKExposureTime(time);
		wostringstream ss;
		ss << "Real Fast Kinetics exposure time is " << (*time) * pow(10.0, 3.0) << " ms.\n";
		handleErrors(errorVal, errorMsg, L"GetFKExposureTime error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}

	int _SetFastKineticsStorageMode(int mode) {
		wstring w(L"");
		switch (mode) {
			case 0:
				w = L"Binning in readout register.\n";
				break;
			case 1:
				w = L"Binning in storage area.\n";
				break;
		}
		int errorVal = SetFastKineticsStorageMode(mode);
		handleErrors(errorVal, errorMsg, L"SetFastKineticsStorageMode error: ", w.c_str(), flagVerbose);
		return errorVal;
	}

	int _SetFastKineticsTimeScanMode(int exposedRows, int seriesLength, int mode) {
		wstring w(L"");
		switch (mode) {
			case 0:
				w = L"FK time scan off.\n";
				break;
			case 1:
				w = L"FK time scan: accumulate mode.\n";
				break;
			case 2:
				w = L"FK time scan: series mode.\n";
				break;
		}

		int errorVal = SetFastKineticsTimeScanMode(exposedRows, seriesLength, mode);
		handleErrors(errorVal, errorMsg, L"SetFastKineticsTimeScanMode error: ", w.c_str(), flagVerbose);
		return errorVal;
	}

	int _GetAcquisitionTimings(float* expTime, float* accTime, float* kinTime) {
		*expTime = 0;
		*accTime = 0;
		*kinTime = 0;

		int errorVal = GetAcquisitionTimings(expTime, accTime, kinTime);
		wostringstream ss;
		ss << "Real (exp., acc., kin.) times are ("
			<< (*expTime) * pow(10.0, 3.0) << ", "
			<< (*accTime) * pow(10.0, 3.0) << ", "
			<< (*kinTime) * pow(10.0, 3.0) << ") ms.\n";

		handleErrors(errorVal, errorMsg, L"GetAcquisitionTimings error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}

	int _GetReadoutTime(float* readout) {
		*readout = 0;
		int errorVal = GetReadOutTime(readout);

		wostringstream ss;
		ss << "Readout time is " << (*readout) * pow(10.0, 3.0) << " ms.\n";
		handleErrors(errorVal, errorMsg, L"GetReadoutTime error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}

	int _StartAcquisition(void) {
		int errorVal = StartAcquisition();
		handleErrors(errorVal, errorMsg, L"StartAcquisition error: ", L"Acquiring\n", TRUE);
		return errorVal;
	}

	int _GetNumberAvailableImages(long* index1, long* index2) {
		*index1 = 0;
		*index2 = 0;
		int errorVal = GetNumberAvailableImages(index1, index2);
		std::wostringstream ss;
		ss << "Available images are " << *index1 << " to " << *index2 << ".\n";
		handleErrors(errorVal, errorMsg, L"GetNumberAvailableImages error: ", ss.str().c_str(), flagVerbose);
		return errorVal;
	}

	int _GetImages(long index1, long index2, long* buffer, long data_length) {
		long valInd1, valInd2;
		int errorVal = GetImages(index1, index2, buffer, data_length, &valInd1, &valInd2);
		handleErrors(errorVal, errorMsg, L"GetImages error: ", L"Readout complete!\n", TRUE);
		return errorVal;
	}


	int StartCooler(void) {
		return 0;
	}

	camInfo_t GetCamInfo(void) {return camInfo;}


protected:
	wstring handleErrors(int errorCode, wstring msg, const wchar_t* errorIdentifier, LPCWSTR successMsg, BOOL printFlag) {
		std::wstring msg2(errorIdentifier);

		switch (errorCode) {
			case DRV_SUCCESS: {
				if (printFlag)
					appendToEditControl(hStatusLogEdit, successMsg);
				return msg;
			}	
			case DRV_VXDNOTINSTALLED:
				msg += msg2 + L"VxD not installed\n";
				break;
			case DRV_INIERROR:
				msg += msg2 + L"Unable to load \"DETECTOR.INI\"\n";
				break;
			case DRV_COFERROR:
				msg += msg2 + L"Unable to load \"*.COF\"\n";
				break;
			case DRV_FLEXERROR:
				msg += msg2 + L"Unable to load \"*.RBF\"\n";
				break;
			case DRV_ERROR_FILELOAD:
				msg += msg2 + L"Unable to load \"*.COF\" or \"*.RBF\" files\n";
				break;
			case DRV_ERROR_PAGELOCK:
				msg += msg2 + L"Unable to acquire lock on requested memory\n";
				break;
			case DRV_USBERROR:
				msg += msg2 + L"Unable to detect USB device or not USB2.0\n";
				break;
			case DRV_ERROR_NOCAMERA:
				msg += msg2 + L"No camera found\n";
				break;
			case DRV_P1INVALID:
				msg += msg2 + L"Parameter 1 invalid\n";
				break;
			case DRV_P2INVALID:
				msg += msg2 + L"Parameter 2 invalid\n";
				break;
			case DRV_P3INVALID:
				msg += msg2 + L"Parameter 3 invalid\n";
				break;
			case DRV_P4INVALID:
				msg += msg2 + L"Parameter 4 invalid\n";
				break;
			case DRV_P5INVALID:
				msg += msg2 + L"Parameter 5 invalid\n";
				break;
			case DRV_P6INVALID:
				msg += msg2 + L"Parameter 6 invalid\n";
				break;
			case DRV_P7INVALID:
				msg += msg2 + L"Parameter 7 invalid\n";
				break;
			default:
				msg += msg2 + L"Other error\n";
				break;
		}
		errorFlag = TRUE;
		return msg;
	}		

};




class KRbFastKinetics : public iXonClass {
public:
	int iXonArm(void) {
		errorFlag = FALSE;
		errorMsg = L"";

		iXonClass::_SetAcquisitionMode(KRbCAM_ACQ_MODE);
		iXonClass::_SetReadMode(KRbCAM_READ_MODE);
		iXonClass::_SetShutter(KRbCAM_USE_INTERNAL_SHUTTER, camInfo.shutterMinT[0], camInfo.shutterMinT[1]);
		iXonClass::_SetTriggerMode(KRbCAM_TRIGGER_MODE);
		iXonClass::_SetEMGainMode(KRbCAM_EM_MODE);
		iXonClass::_GetEMGainRange(&camInfo.emGainRange[0], &camInfo.emGainRange[1]);

		if (errorFlag) {
			MessageBox(GetActiveWindow(), errorMsg.c_str(), L"Error.", MB_OK);
			return -1;
		}
		else
			return 0;
	}

	int iXonSetupAcquisition(config_form_input_t config) {
		errorFlag = FALSE;
		errorMsg = L"";

		iXonClass::_SetEMCCDGain(config.emGain);
		iXonClass::_SetupFastKinetics(config.height, KRbCAM_FK_SERIES_LENGTH, config.expTime, config.binning ? KRbCAM_BIN_SIZE : 1, config.yOffset);
		iXonClass::_SetFastKineticsStorageMode(0);
		iXonClass::_SetFastKineticsTimeScanMode(0, 0, 0);

		float realExp;
		iXonClass::_GetFKExposureTime(&realExp);

		float realAcc, realKin;
		iXonClass::_GetAcquisitionTimings(&realExp, &realAcc, &realKin);

		float readout;
		iXonClass::_GetReadoutTime(&readout);

		if (errorFlag) {
			MessageBox(GetActiveWindow(), errorMsg.c_str(), L"Error.", MB_OK);
			return -1;
		}
		else
			return 0;
	}

};






#endif