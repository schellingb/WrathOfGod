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

#include "include.h"
#include "world.h"

ZL_Font fntMain, fntBig;

struct sGodzilla : public ZL_Application
{
	sGodzilla() : ZL_Application(60) { }

	virtual void Load(int argc, char *argv[])
	{
		if (!LoadReleaseDesktopDataBundle()) return;
		ZL_Display::Init("Wrath of God", 1280, 720, ZL_DISPLAY_ALLOWRESIZEHORIZONTAL);
		ZL_Display::ClearFill(ZL_Color::Black);
		ZL_Audio::Init();
		ZL_Timer::Init();

		fntMain = ZL_Font("Data/fntMain.png");
		fntBig = ZL_Font("Data/typomoderno.ttf.zip", 100).SetDrawAtBaseline(false);

		World.InitGlobal();

		ZL_SceneManager::Init(SCENE_TITLE);
	}
} Godzilla;


void DrawTextBordered(const ZL_Vector& p, const char* txt, scalar scale, const ZL_Color& colfill, const ZL_Color& colborder, ZL_Font& fnt, int border, ZL_Origin::Type origin)
{
	for (int i = 0; i < 9; i++) if (i != 4) fnt.Draw(p.x+(border*((i%3)-1)), p.y+8+(border*((i/3)-1)), txt, scale, scale, colborder, origin);
	fnt.Draw(p.x  , p.y+8  , txt, scale, scale, colfill, origin);
}

#define METEOR_IMCSONG_LEN 0x1
#define METEOR_IMCSONG_ROWLENSAMPLES 5512
#define METEOR_IMCSONG_ENVLISTSIZE 3
#define METEOR_IMCSONG_ENVCOUNTERLISTSIZE 4
#define METEOR_IMCSONG_OSCLISTSIZE 9
#define METEOR_IMCSONG_EFFECTLISTSIZE 0
#define METEOR_IMCSONG_VOL 100
unsigned int METEOR_ImcSongOrderTable[] = { 0x000000001, };
unsigned char METEOR_ImcSongPatternData[] = { 0x50, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     };
unsigned char METEOR_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope METEOR_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 256, 13, 8, 16, 255, true, 255, },
  { 0, 256, 13, 10, 14, 255, true, 255, },
};
TImcSongEnvelopeCounter METEOR_ImcSongEnvCounterList[] = { { 0, 0, 256 }, { 1, 0, 256 }, { 2, 0, 248 }, { -1, -1, 256 } };
TImcSongOscillator METEOR_ImcSongOscillatorList[] = {
  { 9, 66, IMCSONGOSCTYPE_SQUARE, 0, -1, 68, 1, 2 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 0, 0, 10, 3, 3 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
unsigned char METEOR_ImcSongChannelVol[8] = {100, 100, 100, 100, 100, 100, 100, 100 };
unsigned char METEOR_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool METEOR_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataMETEOR = {
  METEOR_IMCSONG_LEN, METEOR_IMCSONG_ROWLENSAMPLES, METEOR_IMCSONG_ENVLISTSIZE, METEOR_IMCSONG_ENVCOUNTERLISTSIZE, METEOR_IMCSONG_OSCLISTSIZE, METEOR_IMCSONG_EFFECTLISTSIZE, METEOR_IMCSONG_VOL,
  METEOR_ImcSongOrderTable, METEOR_ImcSongPatternData, METEOR_ImcSongPatternLookupTable, METEOR_ImcSongEnvList, METEOR_ImcSongEnvCounterList, METEOR_ImcSongOscillatorList, NULL,
  METEOR_ImcSongChannelVol, METEOR_ImcSongChannelEnvCounter, METEOR_ImcSongChannelStopNote };
ZL_Sound sndMeteor(ZL_SynthImcTrack::LoadAsSample(&imcDataMETEOR));

#define DAMAGE_IMCSONG_LEN 0x1
#define DAMAGE_IMCSONG_ROWLENSAMPLES 2594/8
#define DAMAGE_IMCSONG_ENVLISTSIZE 3
#define DAMAGE_IMCSONG_ENVCOUNTERLISTSIZE 4
#define DAMAGE_IMCSONG_OSCLISTSIZE 9
#define DAMAGE_IMCSONG_EFFECTLISTSIZE 0
#define DAMAGE_IMCSONG_VOL 10
unsigned int DAMAGE_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char DAMAGE_ImcSongPatternData[] = { 0x50, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     };
unsigned char DAMAGE_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope DAMAGE_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 256, 444, 0, 24, 255, true, 255, },
  { 0, 256, 69, 13, 255, 255, true, 255, },
};
TImcSongEnvelopeCounter DAMAGE_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { 1, 0, 128 }, { 2, 0, 206 }, { -1, -1, 256 },
};
TImcSongOscillator DAMAGE_ImcSongOscillatorList[] = {
  { 7, 174, IMCSONGOSCTYPE_SQUARE, 0, -1, 62, 1, 2 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 0, 0, 164, 3, 3 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
unsigned char DAMAGE_ImcSongChannelVol[8] = {100, 100, 100, 100, 100, 100, 100, 100 };
unsigned char DAMAGE_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool DAMAGE_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataDAMAGE = {
  DAMAGE_IMCSONG_LEN, DAMAGE_IMCSONG_ROWLENSAMPLES, DAMAGE_IMCSONG_ENVLISTSIZE, DAMAGE_IMCSONG_ENVCOUNTERLISTSIZE, DAMAGE_IMCSONG_OSCLISTSIZE, DAMAGE_IMCSONG_EFFECTLISTSIZE, DAMAGE_IMCSONG_VOL,
  DAMAGE_ImcSongOrderTable, DAMAGE_ImcSongPatternData, DAMAGE_ImcSongPatternLookupTable, DAMAGE_ImcSongEnvList, DAMAGE_ImcSongEnvCounterList, DAMAGE_ImcSongOscillatorList, NULL,
  DAMAGE_ImcSongChannelVol, DAMAGE_ImcSongChannelEnvCounter, DAMAGE_ImcSongChannelStopNote };

ZL_Sound sndDamage(ZL_SynthImcTrack::LoadAsSample(&imcDataDAMAGE));

#define SCORE_IMCSONG_LEN 0x1
#define SCORE_IMCSONG_ROWLENSAMPLES 3307
#define SCORE_IMCSONG_ENVLISTSIZE 3
#define SCORE_IMCSONG_ENVCOUNTERLISTSIZE 4
#define SCORE_IMCSONG_OSCLISTSIZE 9
#define SCORE_IMCSONG_EFFECTLISTSIZE 0
#define SCORE_IMCSONG_VOL 50
unsigned int SCORE_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char SCORE_ImcSongPatternData[] = { 0x50, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     };
unsigned char SCORE_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope SCORE_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 255, 1046, 0, 24, 255, true, 255, },
  { 0, 256, 31, 8, 255, 255, true, 255, },
};
TImcSongEnvelopeCounter SCORE_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { 1, 0, 127 }, { 2, 0, 256 }, { -1, -1, 256 },
};
TImcSongOscillator SCORE_ImcSongOscillatorList[] = {
  { 11, 106, IMCSONGOSCTYPE_SINE, 0, -1, 150, 1, 2 },
  { 9, 106, IMCSONGOSCTYPE_SINE, 0, 0, 255, 3, 3 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
unsigned char SCORE_ImcSongChannelVol[8] = {100, 100, 100, 100, 100, 100, 100, 100 };
unsigned char SCORE_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool SCORE_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataSCORE = {
  SCORE_IMCSONG_LEN, SCORE_IMCSONG_ROWLENSAMPLES, SCORE_IMCSONG_ENVLISTSIZE, SCORE_IMCSONG_ENVCOUNTERLISTSIZE, SCORE_IMCSONG_OSCLISTSIZE, SCORE_IMCSONG_EFFECTLISTSIZE, SCORE_IMCSONG_VOL,
  SCORE_ImcSongOrderTable, SCORE_ImcSongPatternData, SCORE_ImcSongPatternLookupTable, SCORE_ImcSongEnvList, SCORE_ImcSongEnvCounterList, SCORE_ImcSongOscillatorList, NULL,
  SCORE_ImcSongChannelVol, SCORE_ImcSongChannelEnvCounter, SCORE_ImcSongChannelStopNote };

ZL_Sound sndScore(ZL_SynthImcTrack::LoadAsSample(&imcDataSCORE));

#define VOLCANO_IMCSONG_LEN 0x1
#define VOLCANO_IMCSONG_ROWLENSAMPLES 6615
#define VOLCANO_IMCSONG_ENVLISTSIZE 5
#define VOLCANO_IMCSONG_ENVCOUNTERLISTSIZE 5
#define VOLCANO_IMCSONG_OSCLISTSIZE 9
#define VOLCANO_IMCSONG_EFFECTLISTSIZE 0
#define VOLCANO_IMCSONG_VOL 100
unsigned int VOLCANO_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char VOLCANO_ImcSongPatternData[] = {
  0x50, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
unsigned char VOLCANO_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope VOLCANO_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 256, 15, 4, 20, 255, true, 255, },
  { -50, 300, 3, 30, 255, 4, true, 255, },
  { 0, 256, 10, 0, 255, 255, true, 255, },
  { 0, 256, 2092, 8, 255, 255, true, 255, },
};
TImcSongEnvelopeCounter VOLCANO_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { 1, 0, 224 }, { 2, 0, 48 }, { 3, 0, 128 },
  { 4, 0, 256 },
};
TImcSongOscillator VOLCANO_ImcSongOscillatorList[] = {
  { 6, 66, IMCSONGOSCTYPE_SQUARE, 0, -1, 40, 1, 2 },
  { 6, 85, IMCSONGOSCTYPE_SQUARE, 0, 0, 26, 3, 4 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
unsigned char VOLCANO_ImcSongChannelVol[8] = {100, 100, 100, 100, 100, 100, 100, 100 };
unsigned char VOLCANO_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool VOLCANO_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataVOLCANO = {
  VOLCANO_IMCSONG_LEN, VOLCANO_IMCSONG_ROWLENSAMPLES, VOLCANO_IMCSONG_ENVLISTSIZE, VOLCANO_IMCSONG_ENVCOUNTERLISTSIZE, VOLCANO_IMCSONG_OSCLISTSIZE, VOLCANO_IMCSONG_EFFECTLISTSIZE, VOLCANO_IMCSONG_VOL,
  VOLCANO_ImcSongOrderTable, VOLCANO_ImcSongPatternData, VOLCANO_ImcSongPatternLookupTable, VOLCANO_ImcSongEnvList, VOLCANO_ImcSongEnvCounterList, VOLCANO_ImcSongOscillatorList, NULL,
  VOLCANO_ImcSongChannelVol, VOLCANO_ImcSongChannelEnvCounter, VOLCANO_ImcSongChannelStopNote };
ZL_Sound sndVolcano(ZL_SynthImcTrack::LoadAsSample(&imcDataVOLCANO));

#define BELL_IMCSONG_LEN 0x1
#define BELL_IMCSONG_ROWLENSAMPLES 3307/2
#define BELL_IMCSONG_ENVLISTSIZE 1
#define BELL_IMCSONG_ENVCOUNTERLISTSIZE 2
#define BELL_IMCSONG_OSCLISTSIZE 9
#define BELL_IMCSONG_EFFECTLISTSIZE 1
#define BELL_IMCSONG_VOL 100
unsigned int BELL_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char BELL_ImcSongPatternData[] = {
  0x50, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
unsigned char BELL_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope BELL_ImcSongEnvList[] = {
  { 0, 256, 65, 5, 18, 255, true, 255, },
};
TImcSongEnvelopeCounter BELL_ImcSongEnvCounterList[] = {
 { 0, 0, 238 }, { -1, -1, 256 },
};
TImcSongOscillator BELL_ImcSongOscillatorList[] = {
  { 8, 0, IMCSONGOSCTYPE_SINE, 0, -1, 255, 1, 1 },
  { 10, 66, IMCSONGOSCTYPE_SINE, 0, 0, 236, 1, 1 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
TImcSongEffect BELL_ImcSongEffectList[] = {
  { 90, 0, 1, 0, IMCSONGEFFECTTYPE_HIGHPASS, 1, 0 },
};
unsigned char BELL_ImcSongChannelVol[8] = {115, 100, 100, 100, 100, 100, 100, 100 };
unsigned char BELL_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool BELL_ImcSongChannelStopNote[8] = {true, false, false, false, false, false, false, false };
TImcSongData imcDataBELL = {
  BELL_IMCSONG_LEN, BELL_IMCSONG_ROWLENSAMPLES, BELL_IMCSONG_ENVLISTSIZE, BELL_IMCSONG_ENVCOUNTERLISTSIZE, BELL_IMCSONG_OSCLISTSIZE, BELL_IMCSONG_EFFECTLISTSIZE, BELL_IMCSONG_VOL,
  BELL_ImcSongOrderTable, BELL_ImcSongPatternData, BELL_ImcSongPatternLookupTable, BELL_ImcSongEnvList, BELL_ImcSongEnvCounterList, BELL_ImcSongOscillatorList, BELL_ImcSongEffectList,
  BELL_ImcSongChannelVol, BELL_ImcSongChannelEnvCounter, BELL_ImcSongChannelStopNote };
ZL_Sound sndBell(ZL_SynthImcTrack::LoadAsSample(&imcDataBELL));


#define THEME_IMCSONG_LEN 0x2
#define THEME_IMCSONG_ROWLENSAMPLES 5512
#define THEME_IMCSONG_ENVLISTSIZE 5
#define THEME_IMCSONG_ENVCOUNTERLISTSIZE 6
#define THEME_IMCSONG_OSCLISTSIZE 13
#define THEME_IMCSONG_EFFECTLISTSIZE 6
#define THEME_IMCSONG_VOL 100

unsigned int THEME_ImcSongOrderTable[] = {
  0x011000001, 0x022000001,
};
unsigned char THEME_ImcSongPatternData[] = {
  0x5B, 0,    0,    0,    0x55, 0,    0,    0,    0x53, 0,    0,    0,    0x60, 0,    0,    0,
  0x30, 0,    0,    0,    0x42, 0,    0,    0,    0x29, 0,    0,    0,    0x24, 0,    0,    0,
  0x22, 0,    0x22, 0,    0x22, 0,    0x22, 0,    0x22, 0,    0x22, 0,    0x22, 0,    0x22, 0,
  0x30, 0,    0,    0,    0x27, 0,    0,    0,    0x32, 0,    0,    0,    0x29, 0,    0,    0,
  0x39, 0,    0,    0x22, 0x24, 0,    0,    0,    0x39, 0,    0,    0x22, 0x24, 0,    0,    0,
};
unsigned char THEME_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 3, };
TImcSongEnvelope THEME_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 256, 31, 8, 255, 255, true, 255, },
  { 0, 256, 379, 8, 16, 255, true, 255, },
  { 32, 256, 196, 8, 16, 255, true, 255, },
  { 100, 150, 76, 8, 255, 255, false, 255, },
};
TImcSongEnvelopeCounter THEME_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { -1, -1, 256 }, { 1, 0, 256 }, { 2, 6, 256 },
  { 3, 6, 256 }, { 4, 7, 150 },
};
TImcSongOscillator THEME_ImcSongOscillatorList[] = {
  { 8, 0, IMCSONGOSCTYPE_SINE, 0, -1, 100, 1, 1 },
  { 8, 0, IMCSONGOSCTYPE_SAW, 0, 0, 100, 1, 2 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 6, -1, 127, 1, 4 },
  { 8, 2, IMCSONGOSCTYPE_SAW, 7, -1, 134, 1, 1 },
  { 8, 0, IMCSONGOSCTYPE_SAW, 7, -1, 182, 1, 1 },
  { 9, 0, IMCSONGOSCTYPE_SAW, 7, -1, 126, 1, 1 },
  { 8, 2, IMCSONGOSCTYPE_SAW, 7, 8, 20, 1, 1 },
  { 8, 127, IMCSONGOSCTYPE_SQUARE, 7, 10, 30, 1, 1 },
};
TImcSongEffect THEME_ImcSongEffectList[] = {
  { 128, 0, 6449, 6, IMCSONGEFFECTTYPE_DELAY, 0, 0 },
  { 255, 110, 1, 6, IMCSONGEFFECTTYPE_RESONANCE, 1, 1 },
  { 227, 0, 1, 6, IMCSONGEFFECTTYPE_HIGHPASS, 1, 0 },
  { 0, 0, 151, 7, IMCSONGEFFECTTYPE_FLANGE, 5, 0 },
  { 13081, 641, 1, 7, IMCSONGEFFECTTYPE_OVERDRIVE, 0, 1 },
  { 57, 0, 1, 7, IMCSONGEFFECTTYPE_LOWPASS, 1, 0 },
};
unsigned char THEME_ImcSongChannelVol[8] = {140, 100, 100, 100, 100, 100, 56, 17 };
unsigned char THEME_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 3, 1 };
bool THEME_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, true, true };
TImcSongData imcDataTHEME = {
  THEME_IMCSONG_LEN, THEME_IMCSONG_ROWLENSAMPLES, THEME_IMCSONG_ENVLISTSIZE, THEME_IMCSONG_ENVCOUNTERLISTSIZE, THEME_IMCSONG_OSCLISTSIZE, THEME_IMCSONG_EFFECTLISTSIZE, THEME_IMCSONG_VOL,
  THEME_ImcSongOrderTable, THEME_ImcSongPatternData, THEME_ImcSongPatternLookupTable, THEME_ImcSongEnvList, THEME_ImcSongEnvCounterList, THEME_ImcSongOscillatorList, THEME_ImcSongEffectList,
  THEME_ImcSongChannelVol, THEME_ImcSongChannelEnvCounter, THEME_ImcSongChannelStopNote };
ZL_SynthImcTrack imcTheme(&imcDataTHEME);

#define GAMEOVER_IMCSONG_LEN 0x1
#define GAMEOVER_IMCSONG_ROWLENSAMPLES 9450
#define GAMEOVER_IMCSONG_ENVLISTSIZE 5
#define GAMEOVER_IMCSONG_ENVCOUNTERLISTSIZE 6
#define GAMEOVER_IMCSONG_OSCLISTSIZE 8
#define GAMEOVER_IMCSONG_EFFECTLISTSIZE 0
#define GAMEOVER_IMCSONG_VOL 100
unsigned int GAMEOVER_ImcSongOrderTable[] = {
  0x000000011,
};
unsigned char GAMEOVER_ImcSongPatternData[] = {
  0x40, 0,    0,    0,    0x3B, 0,    0,    0,    255,  0,    0,    0,    0,    0,    0,    0,
  255,  0,    0,    0,    0,    0,    0,    0,    0x3B, 0x39, 0x37, 0x35, 0x32, 0,    0,    0,
};
unsigned char GAMEOVER_ImcSongPatternLookupTable[] = { 0, 1, 2, 2, 2, 2, 2, 2, };
TImcSongEnvelope GAMEOVER_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 256, 28, 8, 255, 255, true, 255, },
  { 100, 200, 199, 3, 53, 255, true, 255, },
  { 0, 256, 38, 8, 16, 255, true, 255, },
  { 100, 200, 305, 3, 255, 255, true, 255, },
};
TImcSongEnvelopeCounter GAMEOVER_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { 1, 0, 256 }, { 2, 0, 180 }, { 0, 1, 256 },
  { 3, 1, 256 }, { 4, 1, 180 },
};
TImcSongOscillator GAMEOVER_ImcSongOscillatorList[] = {
  { 8, 200, IMCSONGOSCTYPE_SQUARE, 0, -1, 100, 1, 2 },
  { 8, 66, IMCSONGOSCTYPE_SAW, 1, -1, 100, 4, 5 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
unsigned char GAMEOVER_ImcSongChannelVol[8] = {100, 100, 100, 100, 100, 100, 100, 100 };
unsigned char GAMEOVER_ImcSongChannelEnvCounter[8] = {0, 3, 0, 0, 0, 0, 0, 0 };
bool GAMEOVER_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataGAMEOVER = {
  GAMEOVER_IMCSONG_LEN, GAMEOVER_IMCSONG_ROWLENSAMPLES, GAMEOVER_IMCSONG_ENVLISTSIZE, GAMEOVER_IMCSONG_ENVCOUNTERLISTSIZE, GAMEOVER_IMCSONG_OSCLISTSIZE, GAMEOVER_IMCSONG_EFFECTLISTSIZE, GAMEOVER_IMCSONG_VOL,
  GAMEOVER_ImcSongOrderTable, GAMEOVER_ImcSongPatternData, GAMEOVER_ImcSongPatternLookupTable, GAMEOVER_ImcSongEnvList, GAMEOVER_ImcSongEnvCounterList, GAMEOVER_ImcSongOscillatorList, NULL,
  GAMEOVER_ImcSongChannelVol, GAMEOVER_ImcSongChannelEnvCounter, GAMEOVER_ImcSongChannelStopNote };
ZL_SynthImcTrack imcGameOver(&imcDataGAMEOVER, false);

#define CLEARED_IMCSONG_LEN 0x1
#define CLEARED_IMCSONG_ROWLENSAMPLES 3307
#define CLEARED_IMCSONG_ENVLISTSIZE 2
#define CLEARED_IMCSONG_ENVCOUNTERLISTSIZE 3
#define CLEARED_IMCSONG_OSCLISTSIZE 9
#define CLEARED_IMCSONG_EFFECTLISTSIZE 1
#define CLEARED_IMCSONG_VOL 100
unsigned int CLEARED_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char CLEARED_ImcSongPatternData[] = {
  0x40, 0,    0,    0,    0x40, 0,    0x47, 0,    0,    0,    0,    0,    0,    0,    0,    0,
};
unsigned char CLEARED_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope CLEARED_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 256, 26, 8, 16, 255, true, 255, },
};
TImcSongEnvelopeCounter CLEARED_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { 1, 0, 256 }, { -1, -1, 256 },
};
TImcSongOscillator CLEARED_ImcSongOscillatorList[] = {
  { 9, 66, IMCSONGOSCTYPE_SAW, 0, -1, 100, 1, 2 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 0, 0, 4, 2, 2 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
TImcSongEffect CLEARED_ImcSongEffectList[] = {
  { 85, 0, 1, 0, IMCSONGEFFECTTYPE_LOWPASS, 2, 0 },
};
unsigned char CLEARED_ImcSongChannelVol[8] = {100, 100, 100, 100, 100, 100, 100, 100 };
unsigned char CLEARED_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool CLEARED_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataCLEARED = {
  CLEARED_IMCSONG_LEN, CLEARED_IMCSONG_ROWLENSAMPLES, CLEARED_IMCSONG_ENVLISTSIZE, CLEARED_IMCSONG_ENVCOUNTERLISTSIZE, CLEARED_IMCSONG_OSCLISTSIZE, CLEARED_IMCSONG_EFFECTLISTSIZE, CLEARED_IMCSONG_VOL,
  CLEARED_ImcSongOrderTable, CLEARED_ImcSongPatternData, CLEARED_ImcSongPatternLookupTable, CLEARED_ImcSongEnvList, CLEARED_ImcSongEnvCounterList, CLEARED_ImcSongOscillatorList, CLEARED_ImcSongEffectList,
  CLEARED_ImcSongChannelVol, CLEARED_ImcSongChannelEnvCounter, CLEARED_ImcSongChannelStopNote };
ZL_SynthImcTrack imcCleared(&imcDataCLEARED, false);
