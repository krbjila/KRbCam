#ifndef KRBCAM_HELPERS_HPP
#define KRBCAM_HELPERS_HPP

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

#define KRbCAM_TIMER					100 // ms for acquisition loop

#define KRbCAM_LOOP_ACQ				FALSE // Loop acquisitions?

#ifndef KRbCAM_FILENAME_BASE
#define KRbCAM_FILENAME_BASE			L"iXon_img"
#endif


void ValidateConfigValues(config_form_input_t* pData, camInfo_t info) {
	
	// Check that xOffset and yOffset are within bounds
	int x_limit = info.detDim[0];
	int y_limit = info.detDim[1];

	// if Kinetics
	if (KRbCAM_ACQ_MODE == 4) {
		y_limit = (KRbCAM_EXPOSED_ROWS > 512 ? 512 : KRbCAM_EXPOSED_ROWS);
	}

	if (pData->xOffset > x_limit) {
		pData->xOffset = x_limit - 1;
	}

	if (pData->yOffset > y_limit) {
		pData->yOffset = y_limit - 1;
	}


	int w_limit = x_limit - pData->xOffset;
	if (pData->width > w_limit)
		pData->width = w_limit;

	int h_limit = y_limit - pData->yOffset;
	if (pData->height > h_limit)
		pData->height = h_limit;

	// If binning, ensure that the ROI is a multiple of the bin size
	if (pData->binning) {
		pData->height -= pData->height % 2;
		pData->height -= pData->height % 2;
	}

	if (pData->emEnable) {
		if (pData->emGain > info.emGainRange[1])
			pData->emGain = info.emGainRange[1];
		else if (pData->emGain < info.emGainRange[0])
			pData->emGain = info.emGainRange[0];
	}
	else
		pData->emGain = 0;

	if (KRbCAM_ACQ_MODE == 4) {
		pData->width = info.detDim[0];
	}
}


void incrementFileNumber(config_form_input_t* data) {
	data->fileNumber++;
}

#endif