#include "StdAfx.h"
#include "DsimModel.h"
#include <stdio.h>
#include <string>
#include "ActiveModel.h"

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

	mActiveModel = (ActiveModel*) (instance->getactivemodel());

	mInstance = instance;
	mDigitalComponent = dsimckt;

	CHAR *t = mInstance->getstrval((CHAR*)"PATTERN");
	mFilename = t;

	t = mInstance->getstrval((CHAR*)"TOLOW");
	mToLow = atof(t);
	t = mInstance->getstrval((CHAR*)"THENTOHI");
	mThenToHigh = atof(t);
	t = mInstance->getstrval((CHAR*)"THENGUARD");
	mThenGuard = atof(t);

	t = mInstance->getstrval((CHAR*)"FORCEHIBEFORE");
	mForceHiBefore = atof(t);
	t = mInstance->getstrval((CHAR*)"FORCEHIAFTER");
	mForceHiAfter = atof(t);
	t = mInstance->getstrval((CHAR*)"FORCELOBEFORE");
	mForceLoBefore = atof(t);
	t = mInstance->getstrval((CHAR*)"FORCELOAFTER");
	mForceLoAfter = atof(t);

	t = mInstance->getstrval((CHAR*)"RECORD");
	mRecord = atoi(t) ? true : false;

	t = mInstance->getstrval((CHAR*)"OUTPUTTIME");
	mOutputTime = atoi(t) ? true : false;
	t = mInstance->getstrval((CHAR*)"OUTPUTTIMEDELTA");
	mOutputTimeDelta = atoi(t) ? true : false;

	t = mInstance->getstrval((CHAR*)"OUTPUTUSINGWAITS");
	mOutputUsingWaitsValue = 0;
	mOutputUsingWaitsValueLast = 0;
	if (t != 0)
	{
		if (t[0] == '$')
		{
			mOutputUsingWaitsValue = std::strtoul(t+1, nullptr, 16);
		}
		else
		{
			mOutputUsingWaitsValue = std::strtoul(t, nullptr, 10);
		}
	}

	t = mInstance->getstrval((CHAR*)"OUTPUTIGNOREZEROWRITES");
	mOutputIgnoreZeroWrites = false;
	if (t != 0)
	{
		mOutputIgnoreZeroWrites = atoi(t) ? true : false;
	}

	t = mInstance->getstrval((CHAR*)"TRIGGERLEADSTOWRITE");
	mTriggerLeadsToWrite = false;
	if (t != 0)
	{
		mTriggerLeadsToWrite = atoi(t) ? true : false;
	}

	mMWCheckAddressHeldOnLow = false;
	t = mInstance->getstrval((CHAR*)"MWCHECKADDRESSHELDONLOW");
	if (t != 0)
	{
		mMWCheckAddressHeldOnLow = atoi(t) ? true : false;
	}

	mMWCheckDataHeldOnLow = false;
	t = mInstance->getstrval((CHAR*)"MWCHECKDATAHELDONLOW");
	if (t != 0)
	{
		mMWCheckDataHeldOnLow = atoi(t) ? true : false;
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
		mDoStart = false;
		if (mRecord)
		{
			mPatternFP = fopen(mFilename.c_str(), "w");
		}
		else
		{
			mData.init(mFilename.c_str());
		}
		mTryGetData = true;
	}
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

	// Get the sustained value just before the edges
	if (mRecord && isClockEdgePos)
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
	}
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
	}

	// Get held last value on clock low before any edges
	if (mRecord && !isClockEdgeNeg && !isClockEdgePos && clockIsLow)
	{
		int i;
		int newHeldValue = 0;
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
		bool gotErrorThisTime = false;
		if (mMWCheckAddressHeldOnLow)
		{
			if ( (newHeldValue & 0xffffff00) != (mRvalueWhenLow & 0xffffff00))
			{
				if (mPinMWFail != 0)
				{
					mPinMWFail->setstate(time, 0, SHI);
				}
				gotErrorThisTime = true;

				REALTIME rtime = realtime(time);
				fprintf(mPatternFP, ";Address not held @time:%f:$%08x on neg $%08x @ntime:%f\n", rtime, newHeldValue , mRvalueOnNegEdge , realtime(mLastTimeNegEdge));
			}
		}
		if (mMWCheckDataHeldOnLow)
		{
			if ((newHeldValue & 0xff) != (mRvalueWhenLow & 0xff))
			{
				if (!gotErrorThisTime)
				{
					if (mPinMWFail != 0)
					{
						mPinMWFail->setstate(time, 0, SHI);
					}
				}
				gotErrorThisTime = true;

				REALTIME rtime = realtime(time);
				fprintf(mPatternFP, ";Data not held @time:%f:$%08x on neg $%08x @ntime:%f\n", rtime , newHeldValue, mRvalueOnNegEdge, realtime(mLastTimeNegEdge));
			}
		}
		if (gotErrorThisTime)
		{
			// And then indicate a reset for the failure signal
			if (mPinMWFail != 0)
			{
//				mPinMWFail->setstate(time, 1000, SLO);	// I hate the way the relative time doesn't seem to work!!!
				mPinMWFail->setstate(dsimtime(realtime(time) + 0.0000001f) , 0 , SLO);
			}
		}

		mRvalueWhenLow = newHeldValue;
	}

	if (!isClockEdgePos && ishigh(mPinCLOCK->istate()))
	{
		mLastHiClockTime = time;
	}

	if (wasWaitFromQueue || isClockEdgePos)
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
				fprintf(mPatternFP, ";ignored short write d$%08x\n", mRvalueWhenLow);
			}
			else
			{
//				fprintf(mPatternFP, "d$%08x;  %d\n", mRvalueWhenLow , (int)tickDelta);
				fprintf(mPatternFP, "d$%08x\n", mRvalueWhenLow);
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

					// Queued events, so retire them first
					BufferedTransitions &firstOne = mQueuedEvents.front();
					data.simulate(realtime(time), firstOne.mInput, firstOne.mInputPositiveEdge, firstOne.mInputNegativeEdge);
					mQueuedEvents.pop_front();

					if (mActiveModel)
					{
						sprintf(mActiveModel->mDisplayFileAndLine, "Running queued: %d %s", data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
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
}

void DsimModel::QueueOrCheck(BufferedTransitions &potentialTransition)
{
	if (!mRecord && !(mLastAdded == potentialTransition))
	{
		mQueuedEvents.push_back(potentialTransition);
		mLastAdded = potentialTransition;
	}
}

VOID DsimModel::callback (ABSTIME time, EVENTID eventid)
{
}
