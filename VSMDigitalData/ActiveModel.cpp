#include "StdAfx.h"
#include "ActiveModel.h"
#include "DsimModel.h"

VOID ActiveModel::initialize(ICOMPONENT *cpt)
{
	strcpy(mDisplayFileAndLine, "none");

	mComponent = cpt;
	if (!mComponent)
	{
		return;
	}

	BOX textbox;
	mComponent->getsymbolarea(-1, &textbox);

	mDisplayFileAndLinePos.x = textbox.left + 700;
	mDisplayFileAndLinePos.y = textbox.top + 150;
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

	mComponent->settextcolour(0);
	mComponent->drawtext(mDisplayFileAndLinePos.x, mDisplayFileAndLinePos.y, 0, TXJ_LEFT | TXJ_MIDDLE, mDisplayFileAndLine);
}

BOOL ActiveModel::actuate (KEYCODE key, INT x, INT y, DWORD flags)
{
	return FALSE;
}
