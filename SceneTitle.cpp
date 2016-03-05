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

static ZL_Surface srfLudumDare, srf1GAMLogo, srfDude;
static ZL_Rectf recStart;
static ticks_t TimeStart;

struct sTitleDude
{
	ZL_Vector pos, vel;
	scalar ang, rotspeed;
	int index;
	sTitleDude() : pos(RAND_RANGE(-100, ZLWIDTH+100), RAND_RANGE(-100, ZLHEIGHT+100)), vel(RAND_RANGE(-1,-5), RAND_RANGE(-1,-5)), ang(RAND_ANGLE), rotspeed(RAND_RANGE(1,3)), index(1+2*RAND_INT_RANGE(0,3)) { }
};
vector<sTitleDude> TitleDudes;

static void OnStartPress()
{
	sndBell.SetSpeedFactor(1.75).Play();
	ZL_SceneManager::GoToScene(SCENE_GAME);
}

struct sSceneTitle : public ZL_Scene
{
	sSceneTitle() : ZL_Scene(SCENE_TITLE) { }

	int InitTransitionEnter(ZL_SceneType, void*)
	{
		imcTheme.Play(true);
		TimeStart = ZLTICKS;
		srfLudumDare = ZL_Surface("Data/ludumdare.png").SetDrawOrigin(ZL_Origin::BottomRight);
		srf1GAMLogo = ZL_Surface("Data/1gamlogo.png").SetDrawOrigin(ZL_Origin::Center);
		srfDude = ZL_Surface("Data/dude.png").SetDrawOrigin(ZL_Origin::Center).SetRotateOrigin(ZL_Origin::Center).SetScale(2).SetTilesetClipping(4, 2);
		for (int i = 0; i < 100; i++) TitleDudes.push_back(sTitleDude());
		return 400;
	}

	void InitAfterTransition()
	{
		ZL_Display::sigPointerUp.connect(this, &sSceneTitle::OnPointerUp);
		ZL_Display::sigKeyDown.connect(this, &sSceneTitle::OnKeyDown);
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		return 1000;
	}

	void DeInitAfterTransition()
	{
		imcTheme.Stop();
		srfLudumDare = ZL_Surface();
		srf1GAMLogo = ZL_Surface();
		TitleDudes.clear();
	}

	void OnPointerUp(ZL_PointerPressEvent& e)
	{
		if (recStart.Contains(e)) OnStartPress();
		if (e.y < 60 && e.x < 480) ZL_Application::OpenExternalUrl("https://zillalib.github.io/");
		if (e.x >= ZLFROMW(220) && e.y >= ZLFROMH(150)) ZL_Application::OpenExternalUrl("http://www.onegameamonth.com/B_Schelling");
		if (e.y < 60 && e.x > ZLFROMW(400)) ZL_Application::OpenExternalUrl("http://www.ludumdare.com/compo/ludum-dare-27/?action=preview&uid=20775");
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (e.key == ZLK_ESCAPE) ZL_Application::Quit();
	}

	void Calculate()
	{
		recStart = ZL_Rectf(ZLHALFW - 300, 150, ZLHALFW + 300, 310);
	}

	void Draw()
	{
		ZL_Display::ClearFill(ZLRGB( .7+ssin(ZLTICKS*s(.005))*s(.1),  s(.2), s(.1)));

		for (vector<sTitleDude>::iterator it = TitleDudes.begin(); it != TitleDudes.end(); ++it)
		{
			it->ang += ZLELAPSEDF(it->rotspeed);
			it->pos += it->vel * ZLELAPSEDF(10);
			srfDude.SetTilesetIndex(it->index).Draw(it->pos, it->ang);
			if      (it->pos.x <          - 100) it->pos.x =  ZLWIDTH + 100;
			else if (it->pos.x >  ZLWIDTH + 100) it->pos.x =          - 100;
			if      (it->pos.y <          - 100) it->pos.y = ZLHEIGHT + 100;
			else if (it->pos.y > ZLHEIGHT + 100) it->pos.y =          - 100;
		}

		ZL_Display::FillRect(0, 0, ZLWIDTH, ZLHEIGHT, ZLLUMA(0, 0.3));

		ZL_Display::PushMatrix();
		ZL_Display::Translate(ZLHALFW, ZLHALFH);
		ZL_Display::Rotate(scos(ZLTICKS*s(.001))*s(.02));


		int index = ((ZLTICKS-TimeStart)%2000)/500;
		scalar scaletick = 1 + (s((ZLTICKS-TimeStart)%500)/s(500)*s(-.2));
		ZL_Display::Scale(1+ssin(ZLTICKS*s(.001))*s(.02));
		ZL_Display::Translate(-ZLHALFW, -ZLHALFH);

		fntBig.Draw(ZLHALFW + 10,       ZLFROMH(200) - 10, "OF",    s(1.4) * (index == 0 || index == 2 ? scaletick : s(.8)), ZLLUMA(0,0.5), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW + 10 -  40, ZLFROMH(110) - 10, "WRATH", s(2.0) * (index == 0 || index == 1 ? scaletick : s(.8)), ZLLUMA(0,0.5), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW + 10 + 100, ZLFROMH(290) - 10, "GOD",   s(2.0) * (index == 0 || index == 3 ? scaletick : s(.8)), ZLLUMA(0,0.5), ZL_Origin::Center);
		DrawTextBordered(ZL_Vector(ZLHALFW,       ZLFROMH(200)), "OF",    s(1.4) * (index == 0 || index == 2 ? scaletick : s(.8)), ZLRGB(.7,.5,.3), ZLBLACK, fntBig, 3);
		DrawTextBordered(ZL_Vector(ZLHALFW -  40, ZLFROMH(110)), "WRATH", s(2.0) * (index == 0 || index == 1 ? scaletick : s(.8)), ZLRGB(.9,.9,.1), ZLBLACK, fntBig, 3);
		DrawTextBordered(ZL_Vector(ZLHALFW + 100, ZLFROMH(290)), "GOD",   s(2.0) * (index == 0 || index == 3 ? scaletick : s(.8)), ZLRGB(.8,.8,.2), ZLBLACK, fntBig, 3);

		DrawTextBordered(recStart.Center(), "Click To Start", 1 * scaletick);

		srf1GAMLogo.Draw(ZLFROMW(120)+6/2, ZLFROMH(70)+6/2, s(-.3), ZL_Color::Black);
		srf1GAMLogo.Draw(ZLFROMW(120), ZLFROMH(70), s(-.3));

		ZL_Display::PopMatrix();

		DrawTextBordered(ZL_Vector(ZLHALFW,140), "Use mouse gestures to create disasters", 0.5);
		DrawTextBordered(ZL_Vector(ZLHALFW,100), "Inflict your wrath upon all the cities!", 0.5);

		DrawTextBordered(ZL_Vector(18, 12), "(c) 2013 Bernhard Schelling - Nukular Design", s(1), ZLWHITE, ZLBLACK, fntMain, 1, ZL_Origin::BottomLeft);

		srfLudumDare.Draw(ZLFROMW(10), 10);
	}

	void DrawTransition(scalar f, bool IsLeaveTransition)
	{
		imcTheme.SetSongVolume(80-(int)(f*80));
		Draw();
		ZL_Display::FillRect(0, 0, ZLWIDTH, ZLHEIGHT, ZLLUMA(0, f));
	}
} SceneTitle;
