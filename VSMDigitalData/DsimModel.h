#pragma once
#include "StdAfx.h"
#include <stdio.h>
#include "vsm.hpp"
#include "ActiveModel.h"

#define InfoLog(__s__) inst->log(__s__);

#define DsimModel_DEBUG_FORMAT "time %20llu R%02x G%02x B%02x H%02x V%02x\n"

class DsimModel : public IDSIMMODEL
{
public:
	INT isdigital (CHAR *pinname);
	VOID setup (IINSTANCE *inst, IDSIMCKT *dsim);
	VOID runctrl (RUNMODES mode);
	VOID actuate (REALTIME time, ACTIVESTATE newstate);
	BOOL indicate (REALTIME time, ACTIVEDATA *data);
	VOID simulate (ABSTIME time, DSIMMODES mode);
	VOID callback (ABSTIME time, EVENTID eventid);
private:
	IINSTANCE *mInstance;
	IDSIMCKT *mDigitalComponent;
	IDSIMPIN *mPinD[32];
	IDSIMPIN *mPinID[32];
	IDSIMPIN *mPinMEMWRITE , *mPinCLOCK;

	FILE *mPatternFP;

	ActiveModel *mActiveModel;

	bool mDoStart;

	std::string mFilename;
	double mToLow, mThenToHigh, mThenGuard;
	double mForceHiBefore, mForceHiAfter;
	double mForceLoBefore, mForceLoAfter;
	bool mRecord;
	bool mOutputTime , mOutputTimeDelta;

	Data mData;
	bool mTryGetData;
	REALTIME mNotEarlierThan;
	REALTIME mLastTime;
	int mNumWarnings;
};
