#ifndef KRBCAM_HELPERS_HPP
#define KRBCAM_HELPERS_HPP

#ifndef UNICODE
#define UNICODE
#endif

#define KRBCAM_ACQ_MODE 				4 // 2 for accumulate (use this for frame transfer), 4 for fast kinetics
#define KRBCAM_READ_MODE 				4 // 4 for image
#define KRBCAM_USE_INTERNAL_SHUTTER 	FALSE // controls whether the internal shutter is used.
#define KRBCAM_TRIGGER_MODE 			0 // 1 for External Trigger Mode; 0 for Internal mode
#define KRBCAM_EM_MODE 					0 // 0 is normal EM mode
#define KRBCAM_EXPOSED_ROWS 			512 // Number of exposed rows for kinetics imaging
#define KRBCAM_FK_SERIES_LENGTH 		2 // number of shots in fast kinetics series
#define KRBCAM_OD_SERIES_LENGTH			3 // number of fast kinetics series to obtain an OD image
#define	KRBCAM_FK_BINNING_MODE 			4 // 4 for image mode
#define KRBCAM_N_ACC					1 // Number of accumulations: 1

#define KRBCAM_BIN_SIZE					2 // binning is 2x2
#define KRBCAM_TIMER					500 // ms for acquisition loop

#define KRBCAM_LOOP_ACQ					FALSE // Loop acquisitions?


#define KRBCAM_FILENAME_BASE			L"iXon_img" // Filename base for saving files


// Struct for holding other Andor information
struct camInfo_t {
	char model[32]; // Head model number
	int detDim[2]; // {x, y} dimensions in pixels
	BOOL internalShutter; // Has internal shutter?
	int shutterMinT[2]; // {closing time, opening time}
	int emGainRange[2]; // {emLow, emHigh}
};

// Struct for holding the data from the user input form
struct config_form_input_t {
	float expTime; // Exposure time
	int xOffset; // X offset of ROI in px
	int yOffset; // Y offset of ROI in px
	int width; // Width of ROI in px
	int height; // Height of ROI in px
	BOOL binning; // Binning?
	BOOL emEnable; // EM gain enabled?
	int emGain; // EM gain value
	int fileNumber; // Current file number
	std::wstring folderPath; // Path to the save folder
};


void ValidateConfigValues(config_form_input_t* pData, camInfo_t info) {
	
	// Get bounds for xOffset and yOffset
	int x_limit = info.detDim[0];
	int y_limit = info.detDim[1];

	// If Fast Kinetics, set y_limit to the number of exposed rows.
	// The number of exposed rows must be smaller than the array size / 2
	if (KRBCAM_ACQ_MODE == 4) {
		y_limit = (KRBCAM_EXPOSED_ROWS > info.detDim[1] ? info.detDim[1] : KRBCAM_EXPOSED_ROWS);
	}

	// Limit xOffset to the size of the array - 1
	if (pData->xOffset > x_limit) {
		pData->xOffset = x_limit - 1;
	}

	// Limit yOffset to the size of the array - 1
	if (pData->yOffset > y_limit) {
		pData->yOffset = y_limit - 1;
	}


	// Limit the width to the difference between 
	// the edge of the array and the xOffset
	int w_limit = x_limit - pData->xOffset;
	if (pData->width > w_limit)
		pData->width = w_limit;

	// Limit the height to the difference between 
	// the edge of the array (or last exposed row) and the xOffset
	int h_limit = y_limit - pData->yOffset;
	if (pData->height > h_limit)
		pData->height = h_limit;

	// If binning, ensure that the ROI is a multiple of the bin size
	if (pData->binning) {
		pData->height -= pData->height % 2;
		pData->height -= pData->height % 2;
	}

	// If EM gain enabled, make sure it's in range
	if (pData->emEnable) {
		if (pData->emGain > info.emGainRange[1])
			pData->emGain = info.emGainRange[1];
		else if (pData->emGain < info.emGainRange[0])
			pData->emGain = info.emGainRange[0];
	}
	else
		pData->emGain = 0;

	// If Fast Kinetics, the correct readout width is the whole array
	// So set the width to the entire x-dimension size
	if (KRBCAM_ACQ_MODE == 4) {
		pData->width = info.detDim[0];
	}
}


void incrementFileNumber(config_form_input_t* data) {
	data->fileNumber++;
}

#endif