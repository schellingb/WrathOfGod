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

static int lastcountdown, lastthingsexcited, lastscore;
static ZL_TextBuffer txtCountdown, txtPercent, txtScore;
static scalar scaleCountdown, scalePercent, scaleScore;
static scalar FadeGameOver;
static bool paused, gameover;

void DrawTextBordered(scalar x, scalar y, const ZL_TextBuffer &txt, ZL_Origin::Type origin = ZL_Origin::BottomLeft, scalar scale = 1, const ZL_Color& colfill = ZL_Color::White, const ZL_Color &colborder = ZL_Color::Black)
{
	txt.Draw(x+5, y-5, scale, scale, ZLLUMA(0,.5), origin);
	for (int i = 0; i < 9; i++) if (i != 4) txt.Draw(x+(2*((i%3)-1)), y+(2*((i/3)-1)), scale, scale, colborder, origin);
	txt.Draw(x  , y  , scale, scale, colfill, origin);
}

struct sPointer
{
	enum eAction { UNKNOWN, LEFTRIGHT, DOWNANY, UP, UPSIDE } Action;
	ZL_Vector start, now;
	ticks_t timedown;
	bool isdown;
	sPointer() : isdown(false) { }
	void Down(ZL_PointerPressEvent& e)
	{
		if (isdown) return;
		now = start = e;
		Action = UNKNOWN;
		isdown = true;
		timedown = ZLTICKS;
	}

	void Up(ZL_PointerPressEvent& e)
	{
		if (!isdown) return;
		switch (Action)
		{
			case LEFTRIGHT: World.Quake(0); break;
			case DOWNANY: World.LaunchMeteor(start.x, (now-ZLV(start.x, ZLHEIGHT)).Mul(5000).Div(s(ZLTICKS+1-timedown)*s(.2)).Div(ZLWIDTH)); break;
			case UP: World.LaunchVolcano(start.x); break;
			case UPSIDE: World.LaunchWater(start.x, (now-start).Mul(2000).Div(s(ZLTICKS+1-timedown)*s(.2)).Div(ZLWIDTH)); break;
			default:;
		}
		isdown = false;
	}

	void Move(ZL_PointerMoveEvent& e)
	{
		if (!isdown) return;
		now = e;
		if (Action == UNKNOWN && !start.Near(now, ZLWIDTH/s(40)))
		{
			ZL_Vector diff = now-start;
			if (sabs(diff.x) > sabs(diff.y)*5) Action = LEFTRIGHT;
			else if (diff.y < 0) Action = DOWNANY;
			else if (diff.y > sabs(diff.x)*5) Action = UP;
			else Action = UPSIDE;
		}
		if (Action == LEFTRIGHT)
		{
			World.Quake(e.xrel / 5.0f);
		}
	}
};
map<unsigned char, sPointer> Pointers;

struct sSceneGame : public ZL_Scene
{
	sSceneGame() : ZL_Scene(SCENE_GAME) { }

	int InitTransitionEnter(ZL_SceneType SceneTypeFrom, void* data)
	{
		World.Init();
		lastcountdown = lastthingsexcited = lastscore = -1;
		txtCountdown = ZL_TextBuffer(fntBig, "");
		txtPercent = ZL_TextBuffer(fntBig, "");
		txtScore = ZL_TextBuffer(fntBig, "");
		paused = gameover = false;
		return 400;
	}

	void InitAfterTransition()
	{
		ZL_Display::sigPointerDown.connect(this, &sSceneGame::OnPointerDown);
		ZL_Display::sigPointerUp.connect(this, &sSceneGame::OnPointerUp);
		ZL_Display::sigPointerMove.connect(this, &sSceneGame::OnPointerMove);
		ZL_Display::sigMouseWheel.connect(this, &sSceneGame::OnMouseWheel);
		ZL_Display::sigKeyDown.connect(this, &sSceneGame::OnKeyDown);
		ZL_Display::sigActivated.connect(this, &sSceneGame::OnActivated);
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		return 400;
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (paused && e.key == ZLK_ESCAPE) ZL_SceneManager::GoToScene(SCENE_TITLE);
		else if (paused) paused = false;
		else if (gameover && FadeGameOver > s(.5)) ZL_SceneManager::GoToScene(SCENE_TITLE);
		else if (gameover) { }
		else if (e.key == ZLK_ESCAPE) paused = true;
	}

	void OnPointerDown(ZL_PointerPressEvent& e)
	{
		if (paused) paused = false;
		else if (gameover && FadeGameOver > s(.5)) ZL_SceneManager::GoToScene(SCENE_TITLE);
		else if (gameover) { }
		else Pointers[e.which].Down(e);
	}
	void OnPointerUp(ZL_PointerPressEvent& e)   { Pointers[e.which].Up(e);   }
	void OnPointerMove(ZL_PointerMoveEvent& e)  { Pointers[e.which].Move(e); }

	void OnMouseWheel(ZL_MouseWheelEvent& e)
	{
		if      (e.y > 0 && World.ZoomTarget > 0.5f) World.ZoomTarget *= 0.80f;
		else if (e.y < 0 && World.ZoomTarget < 3.0f) World.ZoomTarget *= 1.25f;
	}

	void OnActivated(ZL_WindowActivateEvent& e)
	{
		if (e.key_focus && e.mouse_focus && !e.minimized) return;
		for (map<unsigned char, sPointer>::iterator it = Pointers.begin(); it != Pointers.end(); ++it)
			it->second.isdown = false;
	}

	void Calculate()
	{
		if (gameover || paused) return;
		World.Calculate();
		if (World.countdown)
		{
			int countdown = 1+World.countdown/1000;
			if (countdown != lastcountdown)
			{
				txtCountdown.SetText(ZL_String(countdown));
				lastcountdown = countdown;
				ZL_Timer::EndTransitionFor(&scaleCountdown);
				ZL_Timer::AddTransitionScalar(&(scaleCountdown = 3), 2, 900);
				sndBell.SetSpeedFactor(1).Play();
			}
		}
		else if (lastcountdown)
		{
			txtCountdown.SetText("GO!");
			lastcountdown = 0;
			ZL_Timer::EndTransitionFor(&scaleCountdown);
			ZL_Timer::AddTransitionScalar(&(scaleCountdown = 2), 3, 500);
			sndBell.SetSpeedFactor(1.5f).Play();
		}
		else if (scaleCountdown == 3)
		{
			scaleCountdown = 0;
		}
		if (World.things_excited != lastthingsexcited)
		{
			scalar percent = ZL_Math::Max(0, ZL_Math::Min(1, s(World.things_excited) / (s((World.things_total*10/11)-5))));
			txtPercent.SetText(ZL_String::format("%d%%", (int)(percent*100)));
			lastthingsexcited = World.things_excited;
			ZL_Timer::EndTransitionFor(&scalePercent);
			ZL_Timer::AddTransitionScalar(&(scalePercent = (s(.6)+(percent*s(.4))) + s(.2)), (s(.6)+(percent*s(.4))), 100);
		}
		if (World.score != lastscore)
		{
			txtScore.SetText(ZL_String(World.score) << "pts");
			lastscore = World.score;
			ZL_Timer::EndTransitionFor(&scaleScore);
			if (World.score) ZL_Timer::AddTransitionScalar(&(scaleScore = s(1)), s(0.7), 100);
			else scaleScore = s(0.7);
			sndScore.Stop();
			sndScore.Play();
		}
		if (gameover == false && World.gameover)
		{
			gameover = true;
			ZL_Timer::AddTransitionScalar(&(FadeGameOver = 0), 1, 1000);
			imcGameOver.Play(true);
		}
	}

	void Draw()
	{
		World.Draw();

		scalar colTimerF = s(MIN(3000, World.time))/3000.0f;
		ZL_Color colTimer(1, colTimerF, colTimerF);
		DrawTextBordered(ZLHALFW, ZLFROMH(40), ZL_TextBuffer(fntBig, ZL_String::format("%02d.%02d", (int)(World.time/1000), (int)(s(World.time%1000)/s(10)))), ZL_Origin::Center, s(1.5) - (colTimerF*s(.5)), colTimer);

		DrawTextBordered(ZLHALFW, ZLFROMH(110)-(s(1-colTimerF)*s(10)), txtPercent, ZL_Origin::Center, scalePercent);

		if (scaleCountdown)
			DrawTextBordered(ZLHALFW, ZLHALFH, txtCountdown, ZL_Origin::Center, scaleCountdown, (lastcountdown == 0 ? ZL_Color::Green : ZLRGB(1,1,(s(.33)*lastcountdown))));

		if (!gameover) DrawTextBordered(40, ZLFROMH(40), txtScore, ZL_Origin::CenterLeft, scaleScore);

		if (gameover)
		{
			ZL_Display::FillRect(0, 0, ZLWIDTH, ZLHEIGHT, ZLLUMA(0, FadeGameOver*s(.5)));
			DrawTextBordered(ZLHALFW, ZLHALFH+30, ZL_TextBuffer(fntBig, "GAME OVER"), ZL_Origin::Center, s(.3)+FadeGameOver);
			DrawTextBordered(ZLHALFW-10, ZLHALFH-60, ZL_TextBuffer(fntBig, ZL_String("Final Score: ")), ZL_Origin::CenterRight, s(.3)+FadeGameOver*s(.5));
			DrawTextBordered(ZLHALFW, ZLHALFH-60, txtScore, ZL_Origin::CenterLeft, s(.3)+FadeGameOver*s(.5));
		}
		else if (paused)
		{
			ZL_Display::FillRect(0, 0, ZLWIDTH, ZLHEIGHT, ZLLUMA(0, s(.5)));
			DrawTextBordered(ZLHALFW, ZLHALFH, ZL_TextBuffer(fntBig, "PAUSED"), ZL_Origin::Center, 1);
		}
	}
} SceneGame;
