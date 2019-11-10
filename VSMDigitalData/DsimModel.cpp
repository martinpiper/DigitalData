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
			mActiveModel->getData().init(mActiveModel->mFilename.c_str());
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
				fprintf(mPatternFP, "@time:%f\t", rtime);
				fprintf(mPatternFP, "d$%08x\n", value);
			}
		}
		else
		{
			int i;
			unsigned int value = 0;
			unsigned int valuePosEdge = 0;
			unsigned int valueNegEdge = 0;
			for (i = 7; i >= 0; i--)
			{
				if (ishigh(mPinID[i]->istate()))
				{
					value |= 1;
				}
				value <<= 1;

				if (mPinID[i]->isposedge())
				{
					valuePosEdge |= 1;
				}
				valuePosEdge <<= 1;

				if (mPinID[i]->isnegedge())
				{
					valueNegEdge |= 1;
				}
				valueNegEdge <<= 1;
			}

			unsigned int dataoutput = 0;

			Data& data = mActiveModel->getData();
			data.simulate(realtime(time), value , valuePosEdge , valueNegEdge);
			dataoutput = data.getData();

			for (i = 0; i < 32; i++)
			{
				if (dataoutput & (1 << i))
				{
					if (!ishigh(mPinD[i]->istate()))
					{
						mPinD[i]->setstate(time, 1, SHI);
					}
				}
				else
				{
					if (!islow(mPinD[i]->istate()))
					{
						mPinD[i]->setstate(time, 1, SLO);
					}
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
