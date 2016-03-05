/*
  Wrath of God
  Copyright (C) 2013,2016 Bernhard Schelling

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _GODZILLA_INCLUDE_
#define _GODZILLA_INCLUDE_

#include <ZL_Application.h>
#include <ZL_Display.h>
#include <ZL_Surface.h>
#include <ZL_Signal.h>
#include <ZL_Audio.h>
#include <ZL_Font.h>
#include <ZL_Scene.h>
#include <ZL_Timer.h>
#include <ZL_Particles.h>
#include <ZL_Math.h>
#include <ZL_Network.h>
#include <ZL_SynthImc.h>
#include <../Opt/chipmunk/chipmunk.h>

#include <iostream>
#include <map>
#include <vector>
#include <list>
using namespace std;

extern ZL_Font fntMain;
extern ZL_Font fntBig;
extern ZL_Sound sndMeteor, sndDamage, sndScore, sndVolcano, sndBell;
extern ZL_SynthImcTrack imcTheme, imcGameOver, imcCleared;

#define SCENE_TITLE 1
#define SCENE_GAME 5

void DrawTextBordered(const ZL_Vector& p, const char* txt, scalar scale = 1, const ZL_Color& colfill = ZLWHITE, const ZL_Color& colborder = ZLBLACK, ZL_Font& fnt = fntBig, int border = 2, ZL_Origin::Type origin = ZL_Origin::Center);

#endif //_GODZILLA_INCLUDE_
