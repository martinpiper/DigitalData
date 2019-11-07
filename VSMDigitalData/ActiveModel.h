#pragma once
#include "StdAfx.h"
#include "vsm.hpp"
#include "Data.h"

class ActiveModel : public IACTIVEMODEL
{
public:
    VOID initialize (ICOMPONENT *cpt);
    ISPICEMODEL *getspicemodel (CHAR *primitive);
    IDSIMMODEL *getdsimmodel (CHAR *primitive);
    VOID plot (ACTIVESTATE state);
    VOID animate (INT element, ACTIVEDATA *newstate);
    BOOL actuate (WORD key, INT x, INT y, DWORD flags);

	void drawElements(void);
	void drawText(void);
	void drawScreen(void);

	Data& getData()
	{
		return mData;
	}

private:
	ICOMPONENT *mComponent;
	Data mData;
};
