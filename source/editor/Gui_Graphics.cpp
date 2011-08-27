#include "pch.h"
#include "Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
using namespace MyGUI;
using namespace Ogre;


///  Gui Events
//-----------------------------------------------------------------------------------------------------------

//  [View]  . . . . . . . . . . . . . . . . . . . .	--- checks ----	. . . . . . . . . . . . . . . . . . . .
#define ChkEv(var)  \
	pSet->var = !pSet->var; \
	ButtonPtr chk = wp->castType<Button>(); \
	chk->setStateCheck(pSet->var);

//  Startup
void App::chkOgreDialog(WP wp){		ChkEv(ogre_dialog);	}
void App::chkAutoStart(WP wp){		ChkEv(autostart);	}
void App::chkEscQuits(WP wp){		ChkEv(escquit);		}
