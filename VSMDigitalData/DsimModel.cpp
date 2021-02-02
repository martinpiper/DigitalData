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
	mPatternFP = 0;

	mActiveModel = (ActiveModel*) (instance->getactivemodel());

	mInstance = instance;
	mDigitalComponent = dsimckt;

	CHAR *t = mInstance->getstrval((CHAR*)"PATTERN");
	mFilename = t;

	t = mInstance->getstrval((CHAR*)"TOLOW");
	mToLow = atof(t);
	t = mInstance->getstrval((CHAR*)"TOHI");
	mToHigh = atof(t);

	t = mInstance->getstrval((CHAR*)"RECORD");
	mRecord = atoi(t) ? true : false;

	int i;
	for (i = 0; i < 32; i++)
	{
		char temp[8];
		sprintf(temp, "D%d", i);
		mPinD[i] = mInstance->getdsimpin(temp, true);

		sprintf(temp, "ID%d", i);
		mPinID[i] = mInstance->getdsimpin(temp, true);
	}

	mPinMEMWRITE = mInstance->getdsimpin((CHAR*)"_MEMWRITE", true);
	mPinCLOCK = mInstance->getdsimpin((CHAR*)"CLOCK", true);

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

	Data& data = mData;

	if (data.waitingForInput())
	{
		// Stop any data trigger on the positive clock edge for a tick
		mTryGetData = false;

		if (mActiveModel)
		{
			sprintf(mActiveModel->mDisplayFileAndLine, "Waiting: $%08x: %d %s", data.getWaitingForData(), data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
		}
		data.simulate(realtime(time), value, valuePosEdge, valueNegEdge);
		if (data.anyError())
		{
			mInstance->fatal((CHAR*)data.getError().c_str());
		}
		if (data.waitingForInput())
		{
			return;
		}
		return;
	}


	bool clockEdge = (bool)mPinCLOCK->isposedge();
	if (clockEdge)
	{
		if (mRecord)
		{
			double rtime = realtime(time);

			int i;
			value = 0;
			for (i = 31; i >= 0; i--)
			{
				value <<= 1;
				if (ishigh(mPinD[i]->istate()))
				{
					value |= 1;
				}
			}

//			fprintf(mPatternFP, ";@time:%f\n", rtime);
			fprintf(mPatternFP, "d$%08x\n", value);
		}
		else
		{
			if (mTryGetData)
			{
				if (realtime(time) < mNotEarlierThan)
				{
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
				data.simulate(realtime(time), value, valuePosEdge, valueNegEdge);
				if (mActiveModel)
				{
					sprintf(mActiveModel->mDisplayFileAndLine, "Running: %d %s", data.getCurrentLineNumber(), data.getCurrentFilename().c_str());
				}

				if (data.anyError())
				{
					mInstance->fatal((CHAR*)data.getError().c_str());
				}
				else
				{
					unsigned int dataoutput = data.getData();

					for (i = 0; i < 32; i++)
					{
						if (dataoutput & (1 << i))
						{
							//					if (!ishigh(mPinD[i]->istate()))
							{
								mPinD[i]->setstate(time, 1, SHI);
							}
						}
						else
						{
							//					if (!islow(mPinD[i]->istate()))
							{
								mPinD[i]->setstate(time, 1, SLO);
							}
						}
					}

					mPinMEMWRITE->setstate(time, 1, SHI);
					mPinMEMWRITE->setstate(dsimtime(realtime(time) + mToLow), 1, SLO);
					mPinMEMWRITE->setstate(dsimtime(realtime(time) + mToHigh), 1, SHI);
					mNotEarlierThan = realtime(time) + mToHigh + (mToHigh - mToLow);
				}
			}
			mTryGetData = true;
		}
	}
}

VOID DsimModel::callback (ABSTIME time, EVENTID eventid)
{
}
