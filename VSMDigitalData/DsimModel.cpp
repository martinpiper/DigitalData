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
	}
}

VOID DsimModel::runctrl (RUNMODES mode)
{
	switch (mode)
	{
	case RM_START:
		mDoStart = true;
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

	bool clockEdge = (bool) mPinCLOCK->isposedge();

	if (clockEdge || valuePosEdge || valueNegEdge)
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
			unsigned int dataoutput = 0;

			Data& data = mData;
			data.simulate(realtime(time), value , valuePosEdge , valueNegEdge);
			dataoutput = data.getData();

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

			mPinMEMWRITE->setstate(dsimtime(realtime(time) + mToLow), 1, SLO);
			mPinMEMWRITE->setstate(dsimtime(realtime(time) + mToHigh), 1, SHI);
		}
	}
}

VOID DsimModel::callback (ABSTIME time, EVENTID eventid)
{
}
