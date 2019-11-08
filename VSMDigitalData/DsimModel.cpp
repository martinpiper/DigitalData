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
	mPreviousData = 0;

	mActiveModel = (ActiveModel*) (instance->getactivemodel());

	mInstance = instance;
	mDigitalComponent = dsimckt;

	int i;
	for (i = 0; i < 32; i++)
	{
		char temp[8];
		sprintf(temp, "D%d", i);
		mPinD[i] = mInstance->getdsimpin(temp, true);
	}
	for (i = 0; i < 8; i++)
	{
		char temp[8];
		sprintf(temp, "ID%d", i);
		mPinID[i] = mInstance->getdsimpin(temp, true);
	}

	mPinMEMWRITE = mInstance->getdsimpin((CHAR*)"_MEMWRITE", true);
	mPinCLOCK = mInstance->getdsimpin((CHAR*)"CLOCK", true);

	if (mDoStart)
	{
		mDoStart = false;
		if (mActiveModel->mRecord)
		{
			mPatternFP = fopen(mActiveModel->mFilename.c_str(), "w");
		}
		else
		{
			mPatternFP = fopen(mActiveModel->mFilename.c_str(), "r");
		}
	}
}

VOID DsimModel::runctrl (RUNMODES mode)
{
	switch (mode)
	{
	case RM_START:
		mDoStart = true;
		mFirstTime = true;
		break;
	case RM_SUSPEND:
		fflush(mPatternFP);
		break;
	case RM_STOP:
		fclose(mPatternFP);
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
	bool clockEdge = (bool) mPinCLOCK->isposedge();

	if (clockEdge)
	{
		if (mActiveModel->mRecord)
		{
			double rtime = realtime(time);

			int i;
			unsigned int value = 0;
			for (i = 31; i >= 0; i--)
			{
				if (ishigh(mPinD[i]->istate()))
				{
					value |= 1;
				}
				value <<= 1;
			}

			if (mFirstTime || (mPreviousData != value))
			{
				mFirstTime = false;
				mPreviousData = value;
				fprintf(mPatternFP, "time:%f\n", rtime);
				fprintf(mPatternFP, "data:$%08x\n", value);
			}
		}
		else
		{
			unsigned int dataoutput = 0;

			Data& data = mActiveModel->getData();
			data.simulate(time, clockEdge);
			dataoutput = data.getData();

			int i;
			for (i = 0; i < 32; i++)
			{
				if (dataoutput & (1 << i))
				{
					mPinD[i]->setstate(time, 1, SHI);
				}
				else
				{
					mPinD[i]->setstate(time, 1, SLO);
				}
			}

			mPinMEMWRITE->setstate(dsimtime(realtime(time) + mActiveModel->mToLow), 1, SLO);
			mPinMEMWRITE->setstate(dsimtime(realtime(time) + mActiveModel->mToHigh), 1, SHI);
		}
	}
}

VOID DsimModel::callback (ABSTIME time, EVENTID eventid)
{
}
