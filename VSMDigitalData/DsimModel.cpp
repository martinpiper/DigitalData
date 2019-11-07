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
	mActiveModel = instance->getactivemodel();

	mInstance = instance;
	mDigitalComponent = dsimckt;
	int i;
	for (i = 0; i < 32; i++)
	{
		char temp[8];
		sprintf(temp, "D%d", i);
		mPinD[i] = mInstance->getdsimpin(temp, true);
	}

	mPinMEMWRITE = mInstance->getdsimpin((CHAR*)"_MEMWRITE", true);
	mPinCLOCK = mInstance->getdsimpin((CHAR*)"CLOCK", true);

}

VOID DsimModel::runctrl (RUNMODES mode)
{
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

	unsigned int dataoutput = 0;

	if (mActiveModel)
	{
		Data& data = ((ActiveModel*)mActiveModel)->getData();
		data.simulate(time, clockEdge);
		dataoutput = data.getData();
	}
	if (clockEdge)
	{
		int i;
		for (i = 0; i < 32; i++)
		{
			if(dataoutput & (1<<i))
			{
				mPinD[i]->setstate(time, 1, SHI);
			}
			else
			{
				mPinD[i]->setstate(time, 1, SLO);
			}
		}

		mPinMEMWRITE->setstate(dsimtime(realtime(time) + 0.1f), 1, SLO);
		mPinMEMWRITE->setstate(dsimtime(realtime(time) + 0.2f), 1, SHI);
	}
}

VOID DsimModel::callback (ABSTIME time, EVENTID eventid)
{
}
