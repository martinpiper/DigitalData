#pragma once
#include "StdAfx.h"
#include "vsm.hpp"
#include "Data.h"
#include <string>

#if VSM_API_VERSION < 200
#define KEYCODE WORD
#endif

class ActiveModel : public IACTIVEMODEL
{
public:
    VOID initialize (ICOMPONENT *cpt);
    ISPICEMODEL *getspicemodel (CHAR *primitive);
    IDSIMMODEL *getdsimmodel (CHAR *primitive);
    VOID plot (ACTIVESTATE state);
    VOID animate (INT element, ACTIVEDATA *newstate);
	BOOL actuate(KEYCODE key, INT x, INT y, DWORD flags);

	void drawElements(void);
	void drawText(void);
	void drawScreen(void);

	CHAR mDisplayFileAndLine[512];

private:
	ICOMPONENT *mComponent;

	POINT mDisplayFileAndLinePos;
};
