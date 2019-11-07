#include "StdAfx.h"
#include "ActiveModel.h"
#include "DsimModel.h"

VOID ActiveModel::initialize(ICOMPONENT *cpt)
{
	mComponent = cpt;
	if (!mComponent)
	{
		return;
	}

	CHAR *filename = mComponent->getprop((CHAR*)"PATTERN");
	mData.init(filename);
}

ISPICEMODEL *ActiveModel::getspicemodel (CHAR *primitive)
{
	return NULL;
}

IDSIMMODEL *ActiveModel::getdsimmodel (CHAR *primitive)
{
	return new DsimModel;
}

VOID ActiveModel::plot (ACTIVESTATE state)
{
	drawElements();
	drawScreen();
}

VOID ActiveModel::animate(INT element, ACTIVEDATA *newstate)
{
	drawText();
	drawScreen();
}

void ActiveModel::drawElements(void)
{
	if (!mComponent)
	{
		return;
	}

	mComponent->drawsymbol(-1);
	mComponent->drawsymbol(0);
}

void ActiveModel::drawText(void)
{
	if (!mComponent)
	{
		return;
	}
}

void ActiveModel::drawScreen(void)
{
	if (!mComponent)
	{
		return;
	}
}

BOOL ActiveModel::actuate (WORD key, INT x, INT y, DWORD flags)
{
	return FALSE;
}
