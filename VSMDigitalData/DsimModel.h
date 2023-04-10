#pragma once
#include "StdAfx.h"
#include <stdio.h>
#include <list>
#include "vsm.hpp"
#include "ActiveModel.h"

#define InfoLog(__s__) inst->log(__s__);

#define DsimModel_DEBUG_FORMAT "time %20llu R%02x G%02x B%02x H%02x V%02x\n"

struct BufferedTransitions
{
	unsigned int mInput, mInputPositiveEdge, mInputNegativeEdge;

	bool operator==(const BufferedTransitions& rhs)
	{
		return (mInput = rhs.mInput) && (mInputPositiveEdge = rhs.mInputPositiveEdge) && (mInputNegativeEdge = rhs.mInputNegativeEdge);
	}
};

class DsimModel : public IDSIMMODEL
{
public:
	INT isdigital (CHAR *pinname);
	VOID setup (IINSTANCE *inst, IDSIMCKT *dsim);
	VOID runctrl (RUNMODES mode);
	VOID actuate (REALTIME time, ACTIVESTATE newstate);
	BOOL indicate (REALTIME time, ACTIVEDATA *data);
	VOID simulate (ABSTIME time, DSIMMODES mode);
	void QueueOrCheck(BufferedTransitions &potentialTransition);
	VOID callback (ABSTIME time, EVENTID eventid);
private:
	IINSTANCE *mInstance;
	IDSIMCKT *mDigitalComponent;
	IDSIMPIN *mPinD[32];
	IDSIMPIN *mPinID[32];
	IDSIMPIN *mPinMEMWRITE , *mPinCLOCK , *mPinMWFail;

	FILE *mPatternFP;

	ActiveModel *mActiveModel;

	bool mDoStart;

	std::string mFilename;
	double mToLow, mThenToHigh, mThenGuard;
	double mForceHiBefore, mForceHiAfter;
	double mForceLoBefore, mForceLoAfter;
	bool mRecord;
	bool mOutputTime , mOutputTimeDelta;
	unsigned long mOutputUsingWaitsValue;
	unsigned long mOutputUsingWaitsValueLast;

	Data mData;
	bool mTryGetData;
	REALTIME mNotEarlierThan;
	REALTIME mLastTime;
	int mNumWarnings;

	bool mOutputIgnoreZeroWrites;
	bool mTriggerLeadsToWrite;

	std::list<BufferedTransitions> mQueuedEvents;
	BufferedTransitions mLastAdded;
	ABSTIME mLastHiClockTime;
	unsigned int mRvalueOnNegEdge , mRvalueOnPosEdge;
	unsigned int mRecordMaskAddress = 0xffffff00;
	unsigned int mRecordMaskData = 0x000000ff;
	ABSTIME mLastTimeNegEdge = 0;
	ABSTIME mLastTimePosEdge = 0;
	ABSTIME mLastTimePosEdgeReportTime = 0;
	unsigned int mRvalueWhenLow;
	unsigned int mRvalueAddress , mRvalueData;
	ABSTIME mRvalueAddressLastChangeTime , mRvalueDataLastChangeTime;
	bool mMWCheckAddressHeldOnLow = false;
	bool mMWCheckDataHeldOnLow = false;
	double mMWCheckAddressHeldTimeBeforeNegEdge = 0.0f;
	double mMWCheckAddressDataHeldTimeBeforePosEdge = 0.0f;
	double mMWCheckAddressDataHeldTimeAfterPosEdge = 0.0f;

	unsigned int mAllLastInputBits = 0;
	ABSTIME mAllLastInputBitsTime = 0;
};
