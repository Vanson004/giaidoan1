
// Tuan1.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CTuan1App:
// See Tuan1.cpp for the implementation of this class
//

class CTuan1App : public CWinApp
{
public:
	CTuan1App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CTuan1App theApp;
