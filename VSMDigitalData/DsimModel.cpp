#include "StdAfx.h"
#include "DsimModel.h"
#include <stdio.h>
#include <string>
#include "ActiveModel.h"
#include "assert.h"
#include "..\..\C64\Common\ParamToNum.h"

static void TrimString(std::string& tidy)
{
	while (!tidy.empty() && isspace(tidy.front()))
	{
		tidy.erase(tidy.begin());
	}
	while (!tidy.empty() && isspace(tidy.back()))
	{
		tidy.erase(tidy.end() - 1);
	}
}

INT DsimModel::isdigital (CHAR *pinname)
{
	return TRUE;
}

VOID DsimModel::setup (IINSTANCE *instance, IDSIMCKT *dsimckt)
{
	mLastAdded.mInput = 0;
	mLastAdded.mInputPositiveEdge = 0;
	mLastAdded.mInputNegativeEdge = 0;
	mPatternFP = 0;
	mLastHiClockTime = 0;
	mRvalueWhenLow = 0;
	mRvalueAddress = 0;
	mRvalueData = 0;
	mRvalueAddressLastChangeTime = 0;
	mRvalueDataLastChangeTime = 0;

	mActiveModel = (ActiveModel*) (instance->getactivemodel());

	mInstance = instance;
	mDigitalComponent = dsimckt;

	CHAR *theID = mInstance->id();
	parseExtraConfig(theID);

	CHAR *t = getstrval((CHAR*)"PATTERN");
	mFilename = t;

	t = getstrval((CHAR*)"TOLOW");
	mToLow = atof(t);
	t = getstrval((CHAR*)"THENTOHI");
	mThenToHigh = atof(t);
	t = getstrval((CHAR*)"THENGUARD");
	mThenGuard = atof(t);

	t = getstrval((CHAR*)"FORCEHIBEFORE");
	mForceHiBefore = atof(t);
	t = getstrval((CHAR*)"FORCEHIAFTER");
	mForceHiAfter = atof(t);
	t = getstrval((CHAR*)"FORCELOBEFORE");
	mForceLoBefore = atof(t);
	t = getstrval((CHAR*)"FORCELOAFTER");
	mForceLoAfter = atof(t);

	t = getstrval((CHAR*)"RECORD");
	mRecord = atoi(t) ? true : false;

	t = getstrval((CHAR*)"OUTPUTTIME");
	mOutputTime = atoi(t) ? true : false;
	t = getstrval((CHAR*)"OUTPUTTIMEDELTA");
	mOutputTimeDelta = atoi(t) ? true : false;

	t = getstrval((CHAR*)"OUTPUTUSINGWAITS");
	mOutputUsingWaitsValue = 0;
	mOutputUsingWaitsValueLast = 0;
	if (t != 0)
	{
		mOutputUsingWaitsValue = ParamToUNum(t);
	}

	mRecordSenseChangeMask = 0;
	t = getstrval((CHAR*)"RECORDSENSECHANGEMASK");
	if (t != 0)
	{
		mRecordSenseChangeMask = ParamToUNum(t);
	}

	t = getstrval((CHAR*)"OUTPUTIGNOREZEROWRITES");
	mOutputIgnoreZeroWrites = false;
	if (t != 0)
	{
		mOutputIgnoreZeroWrites = atoi(t) ? true : false;
	}

	t = getstrval((CHAR*)"TRIGGERLEADSTOWRITE");
	mTriggerLeadsToWrite = false;
	if (t != 0)
	{
		mTriggerLeadsToWrite = atoi(t) ? true : false;
	}

	mMWCheckAddressHeldOnLow = false;
	t = getstrval((CHAR*)"MWCHECKADDRESSHELDONLOW");
	if (t != 0)
	{
		mMWCheckAddressHeldOnLow = atoi(t) ? true : false;
	}

	mMWCheckDataHeldOnLow = false;
	t = getstrval((CHAR*)"MWCHECKDATAHELDONLOW");
	if (t != 0)
	{
		mMWCheckDataHeldOnLow = atoi(t) ? true : false;
	}

	t = getstrval((CHAR*)"MWADDRESSHELDTIMEBEFORENEGEDGE");
	mMWCheckAddressHeldTimeBeforeNegEdge = atof(t);

	t = getstrval((CHAR*)"MWADDRESSDATAHELDTIMEBEFOREPOSEDGE");
	mMWCheckAddressDataHeldTimeBeforePosEdge = atof(t);
	
	t = getstrval((CHAR*)"MWADDRESSDATAHELDTIMEAFTERPOSEDGE");
	mMWCheckAddressDataHeldTimeAfterPosEdge = atof(t);

	t = getstrval((CHAR*)"EXITPROCCESSAFTER");
	if (t != 0)
	{
		mExitProccessAfter = dsimtime(atof(t));
	}

	t = getstrval((CHAR*)"FORCEFLUSH");
	if (t != 0)
	{
		mForceFlush = atoi(t) ? true : false;
	}

	t = getstrval((CHAR*)"IGNOREQUEUE");
	if (t != 0)
	{
		mIgnoreQueue = atoi(t) ? true : false;
	}


	// Setup pins

	int i;
	for (i = 0; i < 32; i++)
	{
		char temp[8];
		sprintf(temp, "D%d", i);
		mPinD[i] = mInstance->getdsimpin(temp, true);

		if (!mRecord)
		{
			mPinD[i]->setstate(0, 0, SLO);
		}

		sprintf(temp, "ID%d", i);
		mPinID[i] = mInstance->getdsimpin(temp, true);
	}

	mPinMEMWRITE = mInstance->getdsimpin((CHAR*)"_MEMWRITE", true);
	mPinCLOCK = mInstance->getdsimpin((CHAR*)"CLOCK", true);
	mPinMWFail = mInstance->getdsimpin((CHAR*)"MWFAIL", true);
	if (mPinMWFail != 0)
	{
		mPinMWFail->setstate(0, 0, SLO);
	}

	if (mDoStart)
	{
		if (mRecord)
		{
			mPatternFP = fopen(mFilename.c_str(), "w");
			mData.addRecordingFile(mPatternFP);
		}
		else
		{
			mData.init(mFilename.c_str());
		}
		mTryGetData = true;
	}

	// Read capture groups if any
	if (mRecord)
	{
		for (i = 0; i < 32; i++)
		{
			char name[32];
			sprintf(name, "LABELVALUE%d", i);
			t = getstrval((CHAR*)name);
			if (t != 0)
			{
				std::string working(t);
				size_t pos = working.find('=');
				if (std::string::npos != pos)
				{
					if (mDoStart && mPatternFP != 0)
					{
						fprintf(mPatternFP, ".%s\n", working.c_str());
					}
					std::string label = working.substr(0, pos);
					std::string value = working.substr(pos + 1);
					TrimString(label);
					TrimString(value);
					unsigned long theValue = ParamToUNum(value.c_str());
					int numBits = 0;
					unsigned long calcBits = theValue;
					while (calcBits != 0)
					{
						if (calcBits & 1)
						{
							numBits++;
						}
						// Not sign MSB extended
						calcBits >>= 1;
					}
					bitCountOfPairLabelValue.insert(std::pair<int, std::pair<std::string, unsigned long> >(calcBits, std::pair<std::string, unsigned long>(label, theValue)));
				}
			}
		}
	}

	mDoStart = false;
}

VOID DsimModel::runctrl (RUNMODES mode)
{
	switch (mode)
	{
	case RM_START:
		mDoStart = true;
		mNotEarlierThan = 0;
		mNumWarnings = 0;
		mLastTime = 0;
		break;
	case RM_SUSPEND:
		if (0 != mPatternFP)
		{
			fflush(mPatternFP);
		}
		break;
	case RM_STOP:
		if (0 != mPatternFP)
		{
			mData.removeRecordingFile(mPatternFP);

			fclose(mPatternFP);
		}
		mPatternFP = 0;
		if (!mRecord)
		{
			mData.clear();
		}
		break;
	}
}

VOID DsimModel::actuate (REALTIME time, ACTIVESTATE newstate)
{
}

BOOL DsimModel::indicate (REALTIME time, ACTIVEDATA *data)
{
	return FALSE;
}

VOID DsimModel::simulate(ABSTIME time, DSIMMODES mode)
{
	int i;
	unsigned int value = 0;
	unsigned int valuePosEdge = 0;
	unsigned int valueNegEdge = 0;
	for (i = 31; i >= 0; i--)
	{
		value <<= 1;
		if (ishigh(mPinID[i]->istate()))
		{
			value |= 1;
		}

		valuePosEdge <<= 1;
		if (mPinID[i]->isposedge())
		{
			valuePosEdge |= 1;
		}

		valueNegEdge <<= 1;
		if (mPinID[i]->isnegedge())
		{
			valueNegEdge |= 1;
		}
	}

	unsigned int recordSenseChange = 0;
	if (mRecord)
	{
		for (i = 31; i >= 0; i--)
		{
			if (mRecordSenseChangeMask & (1<<i))
			{
				if (mPinD[i]->isedge())
				{
					recordSenseChange |= 1<<i;
				}
			}
		}
	}

	BufferedTransitions potentialTransition;
	potentialTransition.mInput = value;
	potentialTransition.mInputPositiveEdge = valuePosEdge;
	potentialTransition.mInputNegativeEdge = valueNegEdge;

	Data& data = mData;
	bool wasWaitFromQueue = false;

//	if (data.getCurrentLineNumber() == 158244)
//	{
//		int a = 0;
//	}

	if (!mRecord)
	{
		if (data.waitingForInput())
		{
			// Stop any data trigger on the positive clock edge for a tick
			mTryGetData = false;

			int lastWaitPrint = 0;
			// Retire as many queued events as we can while waiting
			while (!mQueuedEvents.empty() && data.waitingForInput())
			{
				if (mActiveModel)
				{
					lastWaitPrint = data.getWaitingForData();
					sprintf(mActiveModel->mDisplayFileAndLine, "Waiting queued: $%08x: %d %s", lastWaitPrint, data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
				}

				// Queued events, so retire them first
				BufferedTransitions &firstOne = mQueuedEvents.front();
				data.simulate(realtime(time), firstOne.mInput, firstOne.mInputPositiveEdge, firstOne.mInputNegativeEdge);
				mQueuedEvents.pop_front();
			}
			if (data.waitingForInput())
			{
				if (mActiveModel)
				{
					lastWaitPrint = data.getWaitingForData();
					sprintf(mActiveModel->mDisplayFileAndLine, "Waiting: $%08x: %d %s", lastWaitPrint, data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
				}

				// No queued events, so handle the event now
				data.simulate(realtime(time), value, valuePosEdge, valueNegEdge);
			}
			else
			{
				if (!mTriggerLeadsToWrite)
				{
					// And queue the current event
					QueueOrCheck(potentialTransition);
				}
			}

			if (data.anyError())
			{
				mInstance->fatal((CHAR*)data.getError().c_str());
			}
			if (data.waitingForInput())
			{
				return;
			}

			if (mActiveModel)
			{
				sprintf(mActiveModel->mDisplayFileAndLine, "Completed wait: $%08x: %d %s", lastWaitPrint, data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
			}

			if (!mTriggerLeadsToWrite)
			{
				return;
			}

			wasWaitFromQueue = true;
			mTryGetData = true;
		}
	}

	bool isClockEdgePos = mPinCLOCK->isposedge();
	bool isClockEdgeNeg = mPinCLOCK->isnegedge();
	bool clockIsLow = islow(mPinCLOCK->istate());
	bool gotErrorThisTime = false;

	// Get the value on the positive edge or any sense change
	if (mRecord && (isClockEdgePos || recordSenseChange))
	{
		int i;
		mRvalueOnPosEdge = 0;
		for (i = 31; i >= 0; i--)
		{
			mRvalueOnPosEdge <<= 1;
			if (ishigh(mPinD[i]->istate()))
			{
				mRvalueOnPosEdge |= 1;
			}
		}

		if (mMWCheckAddressDataHeldTimeBeforePosEdge > 0.0f)
		{
			// Coincident events are allowed
			ABSTIME atime = time - mRvalueAddressLastChangeTime;
			double delta = realtime(atime);
			if (delta > 0.0f && delta < mMWCheckAddressDataHeldTimeBeforePosEdge)
			{
				gotErrorThisTime = true;
				fprintf(mPatternFP, ";Address not stable before pos edge @petime:%f:$%08x with last change time @actime:%f atime %d\n", realtime(time), mRvalueOnPosEdge, realtime(mRvalueAddressLastChangeTime), (int)atime);
			}
			atime = time - mRvalueDataLastChangeTime;
			delta = realtime(atime);
			if (delta > 0.0f && delta < mMWCheckAddressDataHeldTimeBeforePosEdge)
			{
				gotErrorThisTime = true;
				fprintf(mPatternFP, ";Data not stable before pos edge @petime:%f:$%08x with last change time @dctime:%f atime %d delta_ns:%f\n", realtime(time), mRvalueOnPosEdge, realtime(mRvalueDataLastChangeTime), (int)atime , delta * 1e9);
			}
		}

		mLastTimePosEdge = time;
	}

	// Get the value on the negative edge
	if (mRecord && isClockEdgeNeg)
	{
		int i;
		mRvalueOnNegEdge = 0;
		for (i = 31; i >= 0; i--)
		{
			mRvalueOnNegEdge <<= 1;
			if (ishigh(mPinD[i]->istate()))
			{
				mRvalueOnNegEdge |= 1;
			}
		}
		// Setup for any held value test
		mRvalueWhenLow = mRvalueOnNegEdge;
		mLastTimeNegEdge = time;
		mLastTimePosEdgeReportTime = 0;	// Reset the report time

		if (mMWCheckAddressHeldTimeBeforeNegEdge > 0.0f)
		{
			double delta = realtime(time - mRvalueAddressLastChangeTime);
			if (delta < mMWCheckAddressHeldTimeBeforeNegEdge)
			{
				gotErrorThisTime = true;
				fprintf(mPatternFP, ";Address not stable before neg edge @netime:%f:$%08x with last change time @actime:%f\n", realtime(time), mRvalueOnNegEdge, realtime(mRvalueAddressLastChangeTime));
			}
		}
	}

	// Update any held data timings for any detected change to address or data
	// Must be done after any clock edge detection triggers
	if (mRecord)
	{
		int i;
		unsigned int newValue = 0;
		unsigned int undefined = 0;
		for (i = 31; i >= 0; i--)
		{
			newValue <<= 1;
			if (ishigh(mPinD[i]->istate()))
			{
				newValue |= 1;
			}
			undefined <<= 1;
			if (iscontention(mPinD[i]->istate()))
			{
				undefined |= 1;
			}
		}

		if (mRvalueAddress != (newValue & mRecordMaskAddress))
		{
			mRvalueAddress = (newValue & mRecordMaskAddress);
			mRvalueAddressLastChangeTime = time;
			mLastTimePosEdgeReportTime = 0;	// Reset the time on any change
		}
		if (mRvalueData != (newValue & mRecordMaskData))
		{
			// Excessive amounts of debug
//			fprintf(mPatternFP, ";Data changed @time:%f atime %d old %08x new %08x udef %08x\n", realtime(time), (int)time, mRvalueData, (newValue & mRecordMaskData), (int)undefined);

			mRvalueData = (newValue & mRecordMaskData);
			mRvalueDataLastChangeTime = time;
			mLastTimePosEdgeReportTime = 0;	// Reset the time on any change
		}

		if (mAllLastInputBits != newValue)
		{
			// Excessive amounts of debug
//			fprintf(mPatternFP, ";All data changed @time:%f atime %d delta %d old %08x new %08x udef %08x\n", realtime(time), (int)time, (int)(time - mAllLastInputBitsTime), mAllLastInputBits, newValue, undefined);
			mAllLastInputBits = newValue;
			mAllLastInputBitsTime = time;
		}

		if (mLastTimeNegEdge > 0 && mLastTimePosEdge > 0 && mMWCheckAddressDataHeldTimeAfterPosEdge > 0.0f)
		{
			if (mLastTimePosEdgeReportTime == 0)
			{
				mLastTimePosEdgeReportTime = time;
				double delta = realtime(mRvalueAddressLastChangeTime - mLastTimePosEdge);
				if (delta >= 0.0f && delta < mMWCheckAddressDataHeldTimeAfterPosEdge)
				{
					gotErrorThisTime = true;
					fprintf(mPatternFP, ";Address not stable after pos edge @petime:%f:$%08x with last change time @actime:%f\n", realtime(mLastTimePosEdge), newValue, realtime(mRvalueAddressLastChangeTime));
				}
				delta = realtime(mRvalueDataLastChangeTime - mLastTimePosEdge);
				if (delta >= 0.0f && delta < mMWCheckAddressDataHeldTimeAfterPosEdge)
				{
					gotErrorThisTime = true;
					fprintf(mPatternFP, ";Data not stable after pos edge @petime:%f:$%08x with last change time @dctime:%f\n", realtime(mLastTimePosEdge), newValue, realtime(mRvalueDataLastChangeTime));
				}
			}
		}
	}

	// Get held last value on clock low before any edges
	if (mRecord && !isClockEdgeNeg && !isClockEdgePos && clockIsLow)
	{
		int i;
		unsigned int newHeldValue = 0;
		for (i = 31; i >= 0; i--)
		{
			newHeldValue <<= 1;
			if (ishigh(mPinD[i]->istate()))
			{
				newHeldValue |= 1;
			}
		}

		// For now assume the address is D[8..31]
		// and assume data is D[0..7]
		if (mMWCheckAddressHeldOnLow)
		{
			if ( (newHeldValue & mRecordMaskAddress) != (mRvalueWhenLow & mRecordMaskAddress))
			{
				gotErrorThisTime = true;
				fprintf(mPatternFP, ";Address not held @time:%f:$%08x on neg $%08x @ntime:%f\n", realtime(time), newHeldValue , mRvalueOnNegEdge , realtime(mLastTimeNegEdge));
			}
		}
		if (mMWCheckDataHeldOnLow)
		{
			if ((newHeldValue & mRecordMaskData) != (mRvalueWhenLow & mRecordMaskData))
			{
				gotErrorThisTime = true;
				fprintf(mPatternFP, ";Data not held @time:%f:$%08x on neg $%08x @ntime:%f\n", realtime(time), newHeldValue, mRvalueOnNegEdge, realtime(mLastTimeNegEdge));
			}
		}

		mRvalueWhenLow = newHeldValue;
	}

	if (gotErrorThisTime)
	{
		// And then indicate a reset for the failure signal
		if (mPinMWFail != 0)
		{
			mPinMWFail->setstate(time, 0, SHI);
			mPinMWFail->setstate(dsimtime(realtime(time) + 0.0000001f), 0, SLO);
		}
	}

	if (!isClockEdgePos && ishigh(mPinCLOCK->istate()))
	{
		mLastHiClockTime = time;
	}

	if (wasWaitFromQueue || isClockEdgePos || recordSenseChange)
	{
		if (mRecord)
		{
			REALTIME rtime = realtime(time);

			if (mOutputIgnoreZeroWrites && mRvalueWhenLow == 0)
			{
				return;
			}

			bool willIgnoreWrite = false;
			ABSTIME tickDelta = time - mLastHiClockTime;
			// Too short from the last negative edge to positive edge, then ignore the write
			if (mOutputIgnoreZeroWrites && (tickDelta <= dsimtime(0.0000001L)))
			{
				// Currently disabled because changes on state change were arriving rapidly, since signals take time to settle on transition.
//				willIgnoreWrite = true;
			}

			if (mOutputTime)
			{
				fprintf(mPatternFP, ";@time:%f\n", rtime);
			}
			if (mOutputTimeDelta)
			{
				REALTIME delta = rtime - mLastTime;
				fprintf(mPatternFP, ";delta:%f\n", delta);
			}
			if (recordSenseChange && !(wasWaitFromQueue || isClockEdgePos))
			{
				fprintf(mPatternFP, ";recordSenseChange:$%08x\n", recordSenseChange);
			}
			if (mOutputUsingWaitsValue)
			{
				unsigned long waitValue = mOutputUsingWaitsValue & value;
				// Only output the wait if it has changed
				if (mOutputUsingWaitsValueLast != waitValue)
				{
					// Deliberately use the full value, without the mask, so we can use it to include extra debug info if needed
					if (willIgnoreWrite)
					{
						fprintf(mPatternFP, ";w$%08x,$%08x\n", mOutputUsingWaitsValue, value);
					}
					else
					{
						fprintf(mPatternFP, "w$%08x,$%08x\n", mOutputUsingWaitsValue, value);
					}
					mOutputUsingWaitsValueLast = waitValue;
				}
			}
			if (willIgnoreWrite)
			{
				fprintf(mPatternFP, ";ignored short write d$%08x\n", mRvalueOnPosEdge);
			}
			else
			{
//				fprintf(mPatternFP, "d$%08x;  %d\n", mRvalueOnPosEdge , (int)tickDelta);
				if (!bitCountOfPairLabelValue.empty())
				{
					std::string replacements;
					// We reverse iterate the list because we want to group those labels with the most bits to the left
					unsigned int remainingValue = mRvalueOnPosEdge;
					auto it = bitCountOfPairLabelValue.crbegin();
					while (it != bitCountOfPairLabelValue.crend())
					{
						auto value = it->second;

						const std::string& name = value.first;
						unsigned long theValue = value.second;

						if ((remainingValue & theValue) == theValue)
						{
							// Found a precise bit pattern match, so remove it from the remaining value and output the replacement
							remainingValue &= ~theValue;

							if (!replacements.empty())
							{
								replacements += " | ";
							}

							replacements += name;
						}

						it++;
					}

					if (!replacements.empty() && remainingValue != 0)
					{
						fprintf(mPatternFP, "d=%s | $%08x\n", replacements.c_str() , remainingValue);
					}
					else if (!replacements.empty() && remainingValue == 0)
					{
						fprintf(mPatternFP, "d=%s\n", replacements.c_str());
					}
					else
					{
						assert(remainingValue == mRvalueOnPosEdge);
						// Fall back to the old method if there are no matches
						fprintf(mPatternFP, "d$%08x\n", mRvalueOnPosEdge);
					}
				}
				else
				{
					fprintf(mPatternFP, "d$%08x\n", mRvalueOnPosEdge);
				}
				if (mForceFlush)
				{
					fflush(mPatternFP);
				}
			}

			mLastTime = rtime;
		}
		else
		{
			if (mTryGetData)
			{
				if (realtime(time) < mNotEarlierThan)
				{
					QueueOrCheck(potentialTransition);

					if (mNumWarnings < 100)
					{
						// Avoid any early triggers before the rising edge for memory write has completed
						mInstance->warning((CHAR *)"Early data trigger ignored");

						mNumWarnings++;
						if (mNumWarnings >= 100)
						{
							mInstance->warning((CHAR *)"Too many warnings, others will be not be shown");
						}
					}
					return;
				}

				if (mQueuedEvents.empty())
				{
					// No queued events, so handle the event now
					data.simulate(realtime(time), value, valuePosEdge, valueNegEdge);

					if (mActiveModel)
					{
						sprintf(mActiveModel->mDisplayFileAndLine, "Running: %d %s", data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
					}
				}
				else
				{
					// And queue the current event
					QueueOrCheck(potentialTransition);

					if (!mQueuedEvents.empty())
					{
						// Queued events, so retire them first
						BufferedTransitions& firstOne = mQueuedEvents.front();
						data.simulate(realtime(time), firstOne.mInput, firstOne.mInputPositiveEdge, firstOne.mInputNegativeEdge);
						mQueuedEvents.pop_front();
					}

					if (mActiveModel)
					{
						sprintf(mActiveModel->mDisplayFileAndLine, "Running queued (%d): %d %s", (int) mQueuedEvents.size(), data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
					}
				}

				if (data.waitingForInput())
				{
					return;
				}

				if (data.anyError())
				{
					mInstance->fatal((CHAR*)data.getError().c_str());
				}
				else
				{
					if (data.anyFileOpen())
					{
						unsigned int dataoutput = data.getData();

						if (mForceLoBefore > 0.0f)
						{
							for (i = 0; i < 32; i++)
							{
								mPinD[i]->setstate(time, 1, SLO);
							}
						}
						if (mForceHiBefore > 0.0f)
						{
							for (i = 0; i < 32; i++)
							{
								mPinD[i]->setstate(time, 1, SHI);
							}
						}

						for (i = 0; i < 32; i++)
						{
							if (dataoutput & (1 << i))
							{
								//					if (!ishigh(mPinD[i]->istate()))
								{
									mPinD[i]->setstate(dsimtime(realtime(time) + max(mForceLoBefore, mForceHiBefore)), 1, SHI);
								}
							}
							else
							{
								//					if (!islow(mPinD[i]->istate()))
								{
									mPinD[i]->setstate(dsimtime(realtime(time) + max(mForceLoBefore, mForceHiBefore)), 1, SLO);
								}
							}
						}

						if (mForceLoAfter > 0.0f)
						{
							for (i = 0; i < 32; i++)
							{
								mPinD[i]->setstate(dsimtime(realtime(time) + mForceLoAfter), 1, SLO);
							}
						}
						if (mForceHiAfter > 0.0f)
						{
							for (i = 0; i < 32; i++)
							{
								mPinD[i]->setstate(dsimtime(realtime(time) + mForceHiAfter), 1, SHI);
							}
						}

						mPinMEMWRITE->setstate(time, 1, SHI);
						mPinMEMWRITE->setstate(dsimtime(realtime(time) + mToLow), 1, SLO);
						mPinMEMWRITE->setstate(dsimtime(realtime(time) + mToLow + mThenToHigh), 1, SHI);
						mNotEarlierThan = realtime(time) + mToLow + mThenToHigh + mThenGuard;
					}
					else
					{
						sprintf(mActiveModel->mDisplayFileAndLine, "Stopped: %d %s", data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
						mIgnoreQueue = true;	// Stop queue to save memory since there won't be any more events, at all...
					}
				}
			}
			mTryGetData = true;
		}
	}
	else
	{
		QueueOrCheck(potentialTransition);
	}

	if (mExitProccessAfter > 0 && (time >= mExitProccessAfter))
	{
		if (mRecord)
		{
			fflush(mPatternFP);
			fclose(mPatternFP);
		}
		exit(0);
	}
}

void DsimModel::QueueOrCheck(BufferedTransitions &potentialTransition)
{
	if (!mRecord && !(mLastAdded == potentialTransition))
	{
		if (!mIgnoreQueue)
		{
			mQueuedEvents.push_back(potentialTransition);
		}
		mLastAdded = potentialTransition;
	}
}

VOID DsimModel::callback (ABSTIME time, EVENTID eventid)
{
}

void DsimModel::parseExtraConfig(CHAR *id)
{
	std::string filename = std::string("ModelConfig_") + std::string(id) + std::string(".txt");
	FILE *fp = fopen(filename.c_str(), "r");

	if (!fp)
	{
		return;
	}

	mInstance->log((CHAR *) filename.c_str());


	while (!feof(fp))
	{
		char buffer[1024];
		char *got = fgets(buffer , sizeof(buffer)-1, fp);
		if (!got)
		{
			break;
		}

		std::string tidy(got);
		TrimString(tidy);

		if (tidy.empty())
		{
			continue;
		}

		size_t pos = tidy.find_first_of('=');

		if (pos == std::string::npos)
		{
			continue;
		}

		std::string key = tidy.substr(0,pos);
		std::string value = tidy.substr(pos+1);

		TrimString(key);
		TrimString(value);

		if (key.empty())
		{
			continue;
		}

		mConfig.insert(std::pair<std::string , std::string>(key , value));
	}

	fclose(fp);

	if (mConfig.find("DELETETHISFILE") != mConfig.end())
	{
		DeleteFileA(filename.c_str());
	}
}

CHAR *DsimModel::getstrval(CHAR *name, CHAR *defval)
{
	std::unordered_map<std::string , std::string>::iterator found = mConfig.find(name);
	if (found != mConfig.end())
	{
		const std::string &value = found->second;

		char message[1024];
		sprintf(message , "Replacing '%s' with '%s'" , found->first.c_str() , value.c_str());
		mInstance->log(message);

		return (CHAR *) value.c_str();
	}

	CHAR *ret = mInstance->getstrval(name, defval);
	return ret;
}
