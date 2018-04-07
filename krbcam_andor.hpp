#ifndef KRBCAM_ANDOR_HPP
#define KRBCAM_ANDOR_HPP

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

#include <cmath>

#include <string>
#include <sstream>

// Andor library
#include "atmcd32d.h"

// Struct defs, macros, and a few helper functions:
#include "krbcam_helpers.hpp"

using namespace std;

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
		errorMsg = L"";

		int status;
		iXonClass::GetStatus(&status);

		if (status == DRV_ACQUIRING)
			iXonClass::AbortAcquisition(L"Error aborting acquisition while closing.\n");

		int errorVal = ::SetShutter(1, 2, 0, 0);
		handleErrors(errorVal, errorMsg, L"SetShutter error while closing.\n", L"", FALSE, TRUE);

		errorVal = ::ShutDown();
		handleErrors(errorVal, errorMsg, L"There was an error shutting down.\n", L"", FALSE, TRUE);
	}	

	int iXonInit(void) {
		wstring errorMsg(L"");
		wostringstream wss;

		errorFlag = FALSE;

		// Get current directory for Andor
		wchar_t buffer[256];
		GetCurrentDirectory(256, buffer);

		iXonClass::Initialize(buffer);
		iXonClass::GetCapabilities(&caps);
		iXonClass::GetHeadModel(camInfo.model);
		iXonClass::GetDetector(&camInfo.detDim[0], &camInfo.detDim[1]);
		iXonClass::GetShutterAvailable(&camInfo.internalShutter);
		iXonClass::GetShutterMinTimes(&camInfo.shutterMinT[0], &camInfo.shutterMinT[1]);
		
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

	wstring GetErrorMessage(void) {
		return errorMsg;
	}

	int iXonClass::AbortAcquisition(const wchar_t* msg = L"AbortAcquisition error: ") {
		int errorVal = ::AbortAcquisition();
		errorMsg = handleErrors(errorVal, errorMsg, msg, L"Acquisition aborted.", TRUE, TRUE);
		return errorVal;
	}

	int iXonClass::GetStatus(int* status) {
		int errorVal = ::GetStatus(status);
		errorMsg = handleErrors(errorVal, errorMsg, L"GetStatus error: ", L"", FALSE, TRUE);
		return errorVal;
	}

	int iXonClass::Initialize(LPCWSTR path) {
		int errorVal = ::Initialize((char*)path);
		errorMsg = handleErrors(errorVal, errorMsg, L"Init. error: ", L"SDK initialized.\n", TRUE);
		return errorVal;
	}
	int iXonClass::GetCapabilities(AndorCapabilities* capsOut) {
		capsOut->ulSize = sizeof(AndorCapabilities);
		int errorVal = ::GetCapabilities(capsOut);
		errorMsg = handleErrors(errorVal, errorMsg, L"GetCapabilities error: ", L"", FALSE);
		return errorVal;
	}
	void iXonClass::PrintCameraType(BOOL printFlag) {
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
	int iXonClass::GetHeadModel(char* model) {
		wostringstream ss;
		int errorVal = ::GetHeadModel(model);
		ss << "Head model is " << camInfo.model << ".\n";
		handleErrors(errorVal, errorMsg, L"GetHeadModel error: ", ss.str().c_str(), flagVerbose);
		return errorVal;
	}
	int iXonClass::GetDetector(int* dimX, int* dimY) {
		*dimX = 0; *dimY = 0;

		int errorVal = ::GetDetector(dimX, dimY);
		wostringstream ss;
	  	ss << "Array is " << *dimX << " x " << *dimY << " pixels.\n";
	 	handleErrors(errorVal, errorMsg, L"GetDetector error: ", ss.str().c_str(), flagVerbose);
	 	return errorVal;
	}
	int iXonClass::GetNumberVSSpeeds(int* num) {
		*num = 0;

		int errorVal = ::GetNumberVSSpeeds(num);
		wostringstream ss;
	  	ss << "Number of vertical shift speeds: " << *num << ".\n";
	 	wstring wstr = ss.str();
	    handleErrors(errorVal, errorMsg, L"GetNumberVSSpeeds error: ", ss.str().c_str(), flagVerbose);
	    return errorVal;
	}

	int iXonClass::GetShutterAvailable(BOOL* internalShutter) {
		int shutter = 0;
		int errorVal = ::IsInternalMechanicalShutter(&shutter);
		handleErrors(errorVal, errorMsg, L"IsInternalMechanicalShutter error: ",
					(shutter == 0 ? L"No internal shutter.\n" : L"Has internal shutter.\n"), flagVerbose);
		if (shutter != 0)
			*internalShutter = TRUE;
		else
			*internalShutter = FALSE;
		return errorVal;
	}

	int iXonClass::GetShutterMinTimes(int* closing, int* opening) {
		*closing = 0; *opening = 0;

		int errorVal = ::GetShutterMinTimes(closing, opening);
		wostringstream ss;
		ss << "Minimum shutter closing (opening) time: " << *closing << " (" << *opening << ") ms.\n";
		handleErrors(errorVal, errorMsg, L"GetShutterMinTimes error :", ss.str().c_str(), flagVerbose);
		return errorVal;
	}

	int iXonClass::SetShutter(BOOL useShutter, int closing, int opening) {
		int mode = (useShutter ? 0 : 1);

		wostringstream ss;
		ss << "Internal shutter closing (opening) time set to " << closing << " (" << opening << ") ms.\n";

		int errorVal = ::SetShutter(1, mode, closing, opening);
		handleErrors(errorVal, errorMsg, L"SetShutter error: ",
					(useShutter ? ss.str().c_str() : L"Internal shutter set to always open.\n"), flagVerbose);
		return errorVal;
	}
	int iXonClass::SetTriggerMode(int mode) {
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

		int errorVal = ::SetTriggerMode(mode);
		handleErrors(errorVal, errorMsg, L"SetTriggerMode error: ", w.c_str(), flagVerbose);
		return errorVal;
	}
	int iXonClass::SetEMGainMode(int mode) {
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

		int errorVal = ::SetEMGainMode(mode);
		handleErrors(errorVal, errorMsg, L"SetEMGainMode error: ", w.c_str(), flagVerbose);
		return errorVal;
	}
	int iXonClass::GetEMGainRange(int* low, int* high) {
		*low = 0; *high = 0;

		int errorVal = ::GetEMGainRange(low, high);
		wostringstream ss;
		ss << "EM gain range is " << *low << " to " << *high << ".\n";
		handleErrors(errorVal, errorMsg, L"GetEMGainRange error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}
	int iXonClass::SetEMCCDGain(int gain) {
		int errorVal = ::SetEMCCDGain(gain);
		wostringstream ss;
		ss << L"EM Gain set to " << gain << L".\n";
		handleErrors(errorVal, errorMsg, L"SetEMCCDGain error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}

	int iXonClass::SetAcquisitionMode(int mode) {
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

		int errorVal = ::SetAcquisitionMode(mode);
		handleErrors(errorVal, errorMsg, L"SetAcquisitionMode error: ", w.c_str(), TRUE);
		return errorVal;
	}

	int iXonClass::SetReadMode(int mode) {
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
		int errorVal = ::SetReadMode(mode);
		handleErrors(errorVal, errorMsg, L"SetReadMode error: ", w.c_str(), TRUE);
		return errorVal;
	}

	// height in physical pixels, exposure in ms
	int iXonClass::SetupFastKinetics(int height, int series_length, float exposure, int binning, int y_offset) {
		exposure *= pow(10.0, -3.0);

		int errorVal = ::SetFastKineticsEx(height, series_length, exposure, 4, binning, binning, y_offset);
		handleErrors(errorVal, errorMsg, L"SetFastKineticsEx error: ", L"Fast kinetics set.\n", flagVerbose);
		return errorVal;
	}

	int iXonClass::GetFKExposureTime(float* time) {
		*time = 0;
		int errorVal = ::GetFKExposureTime(time);
		wostringstream ss;
		ss << "Real Fast Kinetics exposure time is " << (*time) * pow(10.0, 3.0) << " ms.\n";
		handleErrors(errorVal, errorMsg, L"GetFKExposureTime error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}

	int iXonClass::SetFastKineticsStorageMode(int mode) {
		wstring w(L"");
		switch (mode) {
			case 0:
				w = L"Binning in readout register.\n";
				break;
			case 1:
				w = L"Binning in storage area.\n";
				break;
		}
		int errorVal = ::SetFastKineticsStorageMode(mode);
		handleErrors(errorVal, errorMsg, L"SetFastKineticsStorageMode error: ", w.c_str(), flagVerbose);
		return errorVal;
	}

	int iXonClass::SetFastKineticsTimeScanMode(int exposedRows, int seriesLength, int mode) {
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

		int errorVal = ::SetFastKineticsTimeScanMode(exposedRows, seriesLength, mode);
		handleErrors(errorVal, errorMsg, L"SetFastKineticsTimeScanMode error: ", w.c_str(), flagVerbose);
		return errorVal;
	}

	int iXonClass::GetAcquisitionTimings(float* expTime, float* accTime, float* kinTime) {
		*expTime = 0;
		*accTime = 0;
		*kinTime = 0;

		int errorVal = ::GetAcquisitionTimings(expTime, accTime, kinTime);
		wostringstream ss;
		ss << "Real (exp., acc., kin.) times are ("
			<< (*expTime) * pow(10.0, 3.0) << ", "
			<< (*accTime) * pow(10.0, 3.0) << ", "
			<< (*kinTime) * pow(10.0, 3.0) << ") ms.\n";

		handleErrors(errorVal, errorMsg, L"GetAcquisitionTimings error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}

	int iXonClass::GetReadoutTime(float* readout) {
		*readout = 0;
		int errorVal = ::GetReadOutTime(readout);

		wostringstream ss;
		ss << "Readout time is " << (*readout) * pow(10.0, 3.0) << " ms.\n";
		handleErrors(errorVal, errorMsg, L"GetReadoutTime error: ", ss.str().c_str(), TRUE);
		return errorVal;
	}

	int iXonClass::StartAcquisition(void) {
		int errorVal = ::StartAcquisition();
		handleErrors(errorVal, errorMsg, L"StartAcquisition error: ", L"Acquiring...\n", TRUE, TRUE);
		return errorVal;
	}

	int iXonClass::GetNumberAvailableImages(long* index1, long* index2) {
		*index1 = 0;
		*index2 = 0;
		int errorVal = ::GetNumberAvailableImages(index1, index2);
		wostringstream ss;
		ss << "Available images are " << *index1 << " to " << *index2 << ".\n";
		handleErrors(errorVal, errorMsg, L"GetNumberAvailableImages error: ", ss.str().c_str(), flagVerbose);
		return errorVal;
	}

	int iXonClass::GetImages(long index1, long index2, long* buffer, long data_length) {
		long valInd1, valInd2;
		int errorVal = ::GetImages(index1, index2, buffer, data_length, &valInd1, &valInd2);
		handleErrors(errorVal, errorMsg, L"GetImages error: ", L"Readout complete!\n", TRUE);
		return errorVal;
	}


	int iXonClass::StartCooler(void) {
		return 0;
	}

	camInfo_t GetCamInfo(void) {return camInfo;}


protected:
	wstring handleErrors(int errorCode, wstring msg, const wchar_t* errorIdentifier, LPCWSTR successMsg, BOOL printFlag, BOOL throwMB = FALSE) {
		wstring msg2(errorIdentifier);

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

		if (throwMB)
			MessageBox(GetActiveWindow(), msg.c_str(), L"Error!", MB_OK);
		return msg;
	}		

};




class KRbFastKinetics : public iXonClass {
public:
	int iXonArm(void) {
		errorFlag = FALSE;
		errorMsg = L"";

		iXonClass::SetAcquisitionMode(KRBCAM_ACQ_MODE);
		iXonClass::SetReadMode(KRBCAM_READ_MODE);
		iXonClass::SetShutter(KRBCAM_USE_INTERNAL_SHUTTER, camInfo.shutterMinT[0], camInfo.shutterMinT[1]);
		iXonClass::SetTriggerMode(KRBCAM_TRIGGER_MODE);
		iXonClass::SetEMGainMode(KRBCAM_EM_MODE);
		iXonClass::GetEMGainRange(&camInfo.emGainRange[0], &camInfo.emGainRange[1]);

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

		iXonClass::SetEMCCDGain(config.emGain);
		iXonClass::SetupFastKinetics(config.height, KRBCAM_FK_SERIES_LENGTH, config.expTime, config.binning ? KRBCAM_BIN_SIZE : 1, config.yOffset);
		iXonClass::SetFastKineticsStorageMode(0);
		iXonClass::SetFastKineticsTimeScanMode(0, 0, 0);

		float realExp;
		iXonClass::GetFKExposureTime(&realExp);

		float realAcc, realKin;
		iXonClass::GetAcquisitionTimings(&realExp, &realAcc, &realKin);

		float readout;
		iXonClass::GetReadoutTime(&readout);

		if (errorFlag) {
			MessageBox(GetActiveWindow(), errorMsg.c_str(), L"Error.", MB_OK);
			return -1;
		}
		else
			return 0;
	}

};






#endif