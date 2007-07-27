/////////////////////////////////////////////////////////////
//
// ApnCamData_CCD4720B.cpp:  Implementation file for the CApnCamData_CCD4720B class.
//
// Copyright (c) 2003, 2004 Apogee Instruments, Inc.
//
/////////////////////////////////////////////////////////////

#include "ApnCamData_CCD4720B.h"

#include <stdlib.h>
#include <string.h>

/////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////


CApnCamData_CCD4720B::CApnCamData_CCD4720B()
{
}


CApnCamData_CCD4720B::~CApnCamData_CCD4720B()
{
}


void CApnCamData_CCD4720B::Initialize()
{
	strcpy( m_Sensor, "CCD4720B" );
	strcpy( m_CameraModel, "4720" );
	m_CameraId = 21;
	m_InterlineCCD = false;
	m_SupportsSerialA = true;
	m_SupportsSerialB = true;
	m_SensorTypeCCD = true;
	m_TotalColumns = 1072;
	m_ImagingColumns = 1024;
	m_ClampColumns = 24;
	m_PreRoiSkipColumns = 0;
	m_PostRoiSkipColumns = 0;
	m_OverscanColumns = 24;
	m_TotalRows = 2058;
	m_ImagingRows = 1024;
	m_UnderscanRows = 1033;
	m_OverscanRows = 1;
	m_VFlushBinning = 1;
	m_EnableSingleRowOffset = true;
	m_RowOffsetBinning = 1033;
	m_HFlushDisable = false;
	m_ShutterCloseDelay = 0;
	m_PixelSizeX = 13;
	m_PixelSizeY = 13;
	m_Color = false;
	m_ReportedGainSixteenBit = 2;
	m_MinSuggestedExpTime = 1.0;
	m_CoolingSupported = true;
	m_RegulatedCoolingSupported = true;
	m_TempSetPoint = -20.0;
	m_TempRampRateOne = 700;
	m_TempRampRateTwo = 4000;
	m_TempBackoffPoint = 2.0;
	m_PrimaryADType = ApnAdType_Alta_Sixteen;
	m_AlternativeADType = ApnAdType_Alta_Twelve;
	m_DefaultGainTwelveBit = 400;
	m_DefaultOffsetTwelveBit = 200;
	m_DefaultRVoltage = 1000;

	set_vpattern();

	set_hpattern_clamp_sixteen();
	set_hpattern_skip_sixteen();
	set_hpattern_roi_sixteen();

	set_hpattern_clamp_twelve();
	set_hpattern_skip_twelve();
	set_hpattern_roi_twelve();
}


void CApnCamData_CCD4720B::set_vpattern()
{
	const unsigned short Mask = 0xE;
	const unsigned short NumElements = 129;
	unsigned short Pattern[NumElements] = 
	{
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 
		0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 
		0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 
		0x0002, 0x0002, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 
		0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 
		0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0004, 0x0004, 0x0004, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 
		0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 
		0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x0008, 
		0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 
		0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 
		0x0008, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000
	};

	m_VerticalPattern.Mask = Mask;
	m_VerticalPattern.NumElements = NumElements;
	m_VerticalPattern.PatternData = 
		(unsigned short *)malloc(NumElements * sizeof(unsigned short));

	for ( int i=0; i<NumElements; i++ )
	{
		m_VerticalPattern.PatternData[i] = Pattern[i];
	}
}


void CApnCamData_CCD4720B::set_hpattern_skip_sixteen()
{
	const unsigned short Mask = 0x2;
	const unsigned short BinningLimit = 1;
	const unsigned short RefNumElements = 22;
	const unsigned short SigNumElements = 8;

	unsigned short RefPatternData[RefNumElements] = 
	{
		0x006C, 0x0068, 0x006A, 0x006A, 0x1068, 0x1068, 0x1068, 0x0068, 0x00E8, 0x00C8, 
		0x00D8, 0x00D8, 0x00D8, 0x00D8, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D4, 0x00D4, 
		0x00D4, 0x00D4
	};

	unsigned short SigPatternData[SigNumElements] = 
	{
		0x0144, 0x0104, 0x0104, 0x0104, 0x0104, 0x0004, 0x0005, 0x0004
	};

	unsigned short BinNumElements[APN_MAX_HBINNING] = 
	{
		0x0002
	};

	unsigned short BinPatternData[1][256] = {
	{
		0x0054, 0x0044
	} };

	set_hpattern(	&m_SkipPatternSixteen,
					Mask,
					BinningLimit,
					RefNumElements,
					SigNumElements,
					BinNumElements,
					RefPatternData,
					SigPatternData,
					BinPatternData );
}


void CApnCamData_CCD4720B::set_hpattern_clamp_sixteen()
{
	const unsigned short Mask = 0x2;
	const unsigned short BinningLimit = 1;
	const unsigned short RefNumElements = 22;
	const unsigned short SigNumElements = 8;

	unsigned short RefPatternData[RefNumElements] = 
	{
		0x006C, 0x0068, 0x006A, 0x006A, 0x1068, 0x1068, 0x1068, 0x0068, 0x00E8, 0x00C8, 
		0x00D8, 0x00D8, 0x00D8, 0x00D8, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D4, 0x00D4, 
		0x00D4, 0x00D4
	};

	unsigned short SigPatternData[SigNumElements] = 
	{
		0x0144, 0x0104, 0x0104, 0x0104, 0x0104, 0x0004, 0x0005, 0x0004
	};

	unsigned short BinNumElements[APN_MAX_HBINNING] = 
	{
		0x0002
	};

	unsigned short BinPatternData[1][256] = {
	{
		0x0054, 0x0044
	} };

	set_hpattern(	&m_ClampPatternSixteen,
					Mask,
					BinningLimit,
					RefNumElements,
					SigNumElements,
					BinNumElements,
					RefPatternData,
					SigPatternData,
					BinPatternData );
}


void CApnCamData_CCD4720B::set_hpattern_roi_sixteen()
{
	const unsigned short Mask = 0x2;
	const unsigned short BinningLimit = 6;
	const unsigned short RefNumElements = 59;
	const unsigned short SigNumElements = 35;

	unsigned short RefPatternData[RefNumElements] = 
	{
		0x006C, 0x006C, 0x0068, 0x0068, 0x0068, 0x0068, 0x0068, 0x0078, 0x0078, 0x0078, 
		0x0078, 0x0078, 0x0070, 0x0070, 0x0074, 0x0074, 0x0076, 0x0076, 0x0076, 0x0074, 
		0x0074, 0x0074, 0x0074, 0x0074, 0x0074, 0x1074, 0x1074, 0x0074, 0x0074, 0x0074, 
		0x0074, 0x0074, 0x00F4, 0x00F4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 
		0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 
		0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4, 0x00D4
	};

	unsigned short SigPatternData[SigNumElements] = 
	{
		0x0044, 0x0044, 0x0044, 0x0144, 0x0144, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 
		0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 
		0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x8104, 
		0x8104, 0x0004, 0x0004, 0x0405, 0x0404
	};

	unsigned short BinNumElements[APN_MAX_HBINNING] = 
	{
		0x0002, 0x0024, 0x0048, 0x006C, 0x0090, 0x00B4
	};

	unsigned short BinPatternData[6][256] = {
	{
		0x00C4, 0x0044
	},
	{
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 
		0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 
		0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 
		0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044
	},
	{
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 
		0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 
		0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 
		0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 0x0048, 0x0048, 0x0048, 0x0048, 
		0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 0x0050, 0x0050, 0x0050, 0x0050, 
		0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x0044
	},
	{
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 
		0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 
		0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 
		0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 0x0048, 0x0048, 0x0048, 0x0048, 
		0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 0x0050, 0x0050, 0x0050, 0x0050, 
		0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 
		0x004C, 0x004C, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 
		0x0058, 0x0058, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 
		0x0054, 0x0054, 0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044
	},
	{
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 
		0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 
		0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 
		0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 0x0048, 0x0048, 0x0048, 0x0048, 
		0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 0x0050, 0x0050, 0x0050, 0x0050, 
		0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 
		0x004C, 0x004C, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 
		0x0058, 0x0058, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 
		0x0054, 0x0054, 0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 0x0048, 0x0048, 
		0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 0x0050, 0x0050, 
		0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0044, 
		0x0044, 0x0044, 0x0044, 0x0044
	},
	{
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 
		0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 
		0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 
		0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 0x0048, 0x0048, 0x0048, 0x0048, 
		0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 0x0050, 0x0050, 0x0050, 0x0050, 
		0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 
		0x004C, 0x004C, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 
		0x0058, 0x0058, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 
		0x0054, 0x0054, 0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 
		0x0044, 0x0044, 0x0044, 0x004C, 0x004C, 0x004C, 0x004C, 0x004C, 0x0048, 0x0048, 
		0x0048, 0x0048, 0x0048, 0x0048, 0x0058, 0x0058, 0x0058, 0x0058, 0x0050, 0x0050, 
		0x0050, 0x0050, 0x0050, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0044, 
		0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 0x004C, 
		0x004C, 0x004C, 0x004C, 0x004C, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 
		0x0058, 0x0058, 0x0058, 0x0058, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0054, 
		0x0054, 0x0054, 0x0054, 0x0054, 0x0054, 0x0044, 0x0044, 0x0044, 0x0044, 0x0044, 
	} };

	set_hpattern(	&m_RoiPatternSixteen,
					Mask,
					BinningLimit,
					RefNumElements,
					SigNumElements,
					BinNumElements,
					RefPatternData,
					SigPatternData,
					BinPatternData );
}


void CApnCamData_CCD4720B::set_hpattern_skip_twelve()
{
	const unsigned short Mask = 0x0;
	const unsigned short BinningLimit = 1;
	const unsigned short RefNumElements = 0;
	const unsigned short SigNumElements = 0;

	unsigned short *RefPatternData = NULL;

	unsigned short *SigPatternData = NULL;

	unsigned short BinNumElements[APN_MAX_HBINNING] = 
	{
		0x0017
	};

	unsigned short BinPatternData[1][256] = {
	{
		0x000C, 0x0008, 0x400A, 0x000A, 0x0008, 0x0008, 0x0218, 0x0018, 0x0018, 0x0018, 
		0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x2014, 0x0004, 0x0004, 0x0004, 0x0004, 
		0x0004, 0x0005, 0x0004
	} };

	set_hpattern(	&m_SkipPatternTwelve,
					Mask,
					BinningLimit,
					RefNumElements,
					SigNumElements,
					BinNumElements,
					RefPatternData,
					SigPatternData,
					BinPatternData );
}


void CApnCamData_CCD4720B::set_hpattern_clamp_twelve()
{
	const unsigned short Mask = 0x0;
	const unsigned short BinningLimit = 3;
	const unsigned short RefNumElements = 0;
	const unsigned short SigNumElements = 0;

	unsigned short *RefPatternData = NULL;

	unsigned short *SigPatternData = NULL;

	unsigned short BinNumElements[APN_MAX_HBINNING] = 
	{
		0x0017, 0x002E, 0x0044
	};

	unsigned short BinPatternData[3][256] = {
	{
		0x080C, 0x0808, 0x0A0A, 0x080A, 0x0808, 0x0808, 0x0818, 0x0818, 0x0818, 0x0818, 
		0x0810, 0x0810, 0x2810, 0x0810, 0x0814, 0x0814, 0x0804, 0x0804, 0x0804, 0x0804, 
		0x4804, 0x0805, 0x0804
	},
	{
		0x000C, 0x0008, 0x400A, 0x000A, 0x0008, 0x0008, 0x0218, 0x0018, 0x0018, 0x0018, 
		0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x2014, 0x0004, 0x0004, 0x0004, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x000C, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0018, 
		0x0018, 0x0018, 0x0018, 0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x0014, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x0004, 0x8005, 0x8004
	},
	{
		0x000C, 0x0008, 0x400A, 0x000A, 0x0008, 0x0008, 0x0218, 0x0018, 0x0018, 0x0018, 
		0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x2014, 0x0004, 0x0004, 0x0004, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x000C, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0018, 
		0x0018, 0x0018, 0x0018, 0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x0014, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0008, 0x0008, 0x0008, 0x0008, 
		0x0008, 0x0018, 0x0018, 0x0018, 0x0018, 0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 
		0x0014, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x8005, 0x8004
	} };

	set_hpattern(	&m_ClampPatternTwelve,
					Mask,
					BinningLimit,
					RefNumElements,
					SigNumElements,
					BinNumElements,
					RefPatternData,
					SigPatternData,
					BinPatternData );
}


void CApnCamData_CCD4720B::set_hpattern_roi_twelve()
{
	const unsigned short Mask = 0x6802;
	const unsigned short BinningLimit = 3;
	const unsigned short RefNumElements = 0;
	const unsigned short SigNumElements = 0;

	unsigned short *RefPatternData = NULL;

	unsigned short *SigPatternData = NULL;

	unsigned short BinNumElements[APN_MAX_HBINNING] = 
	{
		0x0017, 0x002E, 0x0044
	};

	unsigned short BinPatternData[3][256] = {
	{
		0x000C, 0x0008, 0x020A, 0x000A, 0x0008, 0x0008, 0x0018, 0x0018, 0x0018, 0x0018, 
		0x0010, 0x0010, 0x2010, 0x2010, 0x0014, 0x0014, 0x0004, 0x0004, 0x0004, 0x0004, 
		0x0004, 0xC005, 0xC004
	},
	{
		0x000C, 0x0008, 0x400A, 0x000A, 0x0008, 0x0008, 0x0218, 0x0018, 0x0018, 0x0018, 
		0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x2014, 0x0004, 0x0004, 0x0004, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x000C, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0018, 
		0x0018, 0x0018, 0x0018, 0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x0014, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x0004, 0x8005, 0x8004
	},
	{
		0x000C, 0x0008, 0x400A, 0x000A, 0x0008, 0x0008, 0x0218, 0x0018, 0x0018, 0x0018, 
		0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x2014, 0x0004, 0x0004, 0x0004, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x000C, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0018, 
		0x0018, 0x0018, 0x0018, 0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 0x0014, 0x0004, 
		0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0008, 0x0008, 0x0008, 0x0008, 
		0x0008, 0x0018, 0x0018, 0x0018, 0x0018, 0x0010, 0x0010, 0x0010, 0x0010, 0x0014, 
		0x0014, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x8005, 0x8004
	} };

	set_hpattern(	&m_RoiPatternTwelve,
					Mask,
					BinningLimit,
					RefNumElements,
					SigNumElements,
					BinNumElements,
					RefPatternData,
					SigPatternData,
					BinPatternData );
}


void CApnCamData_CCD4720B::set_hpattern(	APN_HPATTERN_FILE	*Pattern,
											unsigned short	Mask,
											unsigned short	BinningLimit,
											unsigned short	RefNumElements,
											unsigned short	SigNumElements,
											unsigned short	BinNumElements[],
											unsigned short	RefPatternData[],
											unsigned short	SigPatternData[],
											unsigned short	BinPatternData[][APN_MAX_PATTERN_ENTRIES] )
{
	int i, j;

	Pattern->Mask = Mask;
	Pattern->BinningLimit = BinningLimit;
	Pattern->RefNumElements = RefNumElements;
	Pattern->SigNumElements = SigNumElements;

	if ( RefNumElements > 0 )
	{
		Pattern->RefPatternData = 
			(unsigned short *)malloc(RefNumElements * sizeof(unsigned short));

		for ( i=0; i<RefNumElements; i++ )
		{
			Pattern->RefPatternData[i] = RefPatternData[i];
		}
	}

	if ( SigNumElements > 0 )
	{
		Pattern->SigPatternData = 
			(unsigned short *)malloc(SigNumElements * sizeof(unsigned short));

		for ( i=0; i<SigNumElements; i++ )
		{
			Pattern->SigPatternData[i] = SigPatternData[i];
		}
	}

	if ( BinningLimit > 0 )
	{
		for ( i=0; i<BinningLimit; i++ )
		{
			Pattern->BinNumElements[i] = BinNumElements[i];

			Pattern->BinPatternData[i] = 
				(unsigned short *)malloc(BinNumElements[i] * sizeof(unsigned short));

			for ( j=0; j<BinNumElements[i]; j++ )
			{
				Pattern->BinPatternData[i][j] = BinPatternData[i][j];
			}
		}
	}
}
