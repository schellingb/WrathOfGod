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

#include "world.h"

sWorld World;

static scalar Zoom;
static ZL_Vector posCamera, posCameraTarget;
static ZL_Rectf view;

static cpSpace *space = NULL;
static cpBody *ground = NULL;
static ZL_Surface srfGrass, srfClouds, srfFloor, srfWall, srfDude, srfMeteor, srfWater, srfVolcano;
static scalar sRoomHeight;
static size_t things_added;
static ZL_Color colSkyUp, colSkyDown;
static int roundswitchwait;

static const ZL_Color arrSkyColorsUp[]   = { ZLRGBFF( 23, 79,193) , ZLRGBFF( 16, 50,138) , ZLRGBFF(240,181, 52) , ZLRGBFF( 14, 38, 80) , ZLRGBFF( 54, 66,140) , ZLRGBFF( 22, 44, 68) , ZLRGBFF(141,104,137) , ZLRGBFF( 53, 57, 67) , ZLRGBFF(  9, 14, 39) , ZLRGBFF( 95,118,130) };
static const ZL_Color arrSkyColorsDown[] = { ZLRGBFF( 97,169,255) , ZLRGBFF(228,217,177) , ZLRGBFF(143, 61, 69) , ZLRGBFF(209,214,194) , ZLRGBFF(247,131, 62) , ZLRGBFF(232,192,109) , ZLRGBFF(252,170, 80) , ZLRGBFF(223,202,162) , ZLRGBFF(209,210,211) , ZLRGBFF(114, 59, 70) };

#define GROUP_GROUND 1

struct sThing
{
	cpBody *body;
	enum eType { WALL, FLOOR, DUDE, METEOR, WATER, VOLCANO } type;
	bool excited;
	unsigned char tile;
	sThing(cpBody *body, eType type) : body(body), type(type), excited(false) { }
	~sThing()
	{
		cpSpaceRemoveShape(space, body->shapeList);
		cpSpaceRemoveBody(space, body);
		cpShapeFree(body->shapeList);
		cpBodyFree(body);
	}
};

struct sCloud
{
	ZL_Vector pos;
	int tileindex;
	ZL_Color col;
	scalar scale;
	sCloud(ZL_Vector pos) : pos(pos), tileindex(RAND_INT_RANGE(0,7)), col(ZL_Color::LUMA(RAND_RANGE(.9,1),RAND_RANGE(.6,.9))), scale(RAND_RANGE(.03,.1)) { }
};

static vector<sThing*> Things;
static vector<sCloud> Clouds;

static void UpdateVelocityGroundFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpBodyUpdateVelocity(body, cpvzero, damping, dt);
	if (body == ground) cpvclamp(body->v, 2.9f);
}

void sWorld::LaunchMeteor(scalar screenx, const ZL_Vector& vel)
{
	if (roundswitch || countdown) return;
	scalar x = view.left + (screenx * view.Width() / ZLWIDTH);
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(100, cpMomentForCircle(100, 0, 2, cpvzero)));
	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, 2, cpvzero));
	cpShapeSetFriction(shape, 1);
	cpShapeSetCollisionType(shape, GROUP_GROUND);
	cpBodySetPosition(body, cpv(x, 80));
	cpBodySetVelocity(body, cpv(vel.x, vel.y));
	Things.push_back(new sThing(body, sThing::METEOR));
	body->userData = shape->userData = &*Things.back();
	sndMeteor.Play();
}

void sWorld::LaunchWater(scalar screenx, const ZL_Vector& vel)
{
	if (gameover || roundswitch || countdown) return;
	scalar x = view.left + (screenx * view.Width() / ZLWIDTH);
	for (int i = 0; i < 100; i++)
	{
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(0.5f, cpMomentForCircle(0.5f, 0, 0.3f, cpvzero)));
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, 0.3f, cpvzero));
		cpShapeSetFriction(shape, 1);
		cpShapeSetCollisionType(shape, GROUP_GROUND);
		cpBodySetPosition(body, cpv(x+RAND_RANGE(-3,3), RAND_RANGE(-3,0)));
		cpBodyApplyImpulseAtLocalPoint(body, cpv(vel.x-RAND_RANGE(0,5), vel.y+RAND_RANGE(0,5)), cpvzero);
		Things.push_back(new sThing(body, sThing::WATER));
		body->userData = shape->userData = &*Things.back();
	}
}

void sWorld::LaunchVolcano(scalar screenx)
{
	if (gameover || roundswitch || countdown) return;
	scalar x = - ground->p.x + view.left + (screenx * view.Width() / ZLWIDTH);
	cpBody *body = cpSpaceAddBody(space, cpBodyNewKinematic());
	cpVect verts[4] = { {x, 0}, {x-15, 1}, {x, 15}, {x+15, 1} };
	cpShape *shape = cpSpaceAddShape(space, cpPolyShapeNew(body, 4, verts, cpTransformIdentity, 0));
	cpShapeSetFriction(shape, 10);
	cpShapeSetCollisionType(shape, GROUP_GROUND);
	body->velocity_func = &UpdateVelocityGroundFunc;
	cpBodySetPosition(body, cpv(ground->p.x, -15));
	cpBodySetVelocity(body, cpv(0, 3));
	Things.push_back(new sThing(body, sThing::VOLCANO));
	body->userData = shape->userData = &*Things.back();
}

void sWorld::Quake(scalar velx)
{
	if (gameover || roundswitch || countdown) return;
	cpBodySetVelocity(ground, cpv(ground->v.x * (1+ZLELAPSED) + velx*ZLELAPSEDF(5), 0));
}

void MakeBuilding(cpFloat xcenter, int floors = 10, int columns = 10, cpFloat roomwidth = 4.0f, cpFloat roomheight = 4.0f)
{
	sRoomHeight = roomheight;
	cpBody *body;
	cpShape *shape;
	cpFloat groundwidth = 1.0f+roomwidth*columns;

	for (int floor = 0; floor < floors; floor++)
	{
		float wallweight = 2, floorweight = 1, dudeweight = 0.05f;
		float y = (floor * (roomheight + 0.5f));
		bool hadwall = true;
		for (int column = 0; column <= columns; column++)
		{
			bool firstcolumn = (column==0), left = (column==1||(column==2&&!hadwall)), right = (column==columns);
			float x = xcenter + 0.5f - groundwidth*0.5f + column*roomwidth;

			if (floor)
			{
				bool haswall = (firstcolumn || right || !hadwall || !RAND_CHANCE(3));
				if (!haswall) { hadwall = false; continue; }

				body = cpSpaceAddBody(space, cpBodyNew(wallweight, cpMomentForBox(wallweight, 1, roomheight)));
				shape = cpSpaceAddShape(space,                            cpBoxShapeNew(body, 1, roomheight, 0));
				cpShapeSetFriction(shape, 1);
				cpBodySetPosition(body, cpv(x, y - (roomheight/2)));
				Things.push_back(new sThing(body, sThing::WALL));
				body->userData = shape->userData = &*Things.back();
			}
			if (firstcolumn) continue;

			float floorwidth =     roomwidth + (left ?  0.5f  : 0) + (right ? 0.5f  : 0) + (hadwall ? 0 :  roomwidth);
			float floorx = x - (roomwidth/2) + (left ? -0.25f : 0) + (right ? 0.25f : 0) + (hadwall ? 0 : -roomwidth/2);
			body = cpSpaceAddBody(space, cpBodyNew(floorweight, cpMomentForBox(floorweight, floorwidth, 0.5f)));
			shape = cpSpaceAddShape(space,                              cpBoxShapeNew(body, floorwidth, 0.5f, 0));
			cpShapeSetFriction(shape, 1);
			cpBodySetPosition(body, cpv(floorx, y + 0.25f));
			Things.push_back(new sThing(body, sThing::FLOOR));
			body->userData = shape->userData = &*Things.back();
			cpShapeUpdate(shape, body->transform);

			if (floor && RAND_CHANCE(3))
			{
				body = cpSpaceAddBody(space, cpBodyNew(dudeweight, cpMomentForBox(dudeweight, 0.8f, 1.6f)));
				shape = cpSpaceAddShape(space,                            cpBoxShapeNew(body, 0.8f, 1.6f, 0));
				cpShapeSetFriction(shape, 2);
				cpBodySetPosition(body, cpv(floorx + RAND_RANGE(-roomwidth/4, roomwidth/4), y - roomheight + 0.8f));
				Things.push_back(new sThing(body, sThing::DUDE));
				Things.back()->tile = 2*RAND_INT_RANGE(0,3);
				body->userData = shape->userData = &*Things.back();
			}

			hadwall = true;
		}
	}
}

void sWorld::InitGlobal()
{
	srfGrass = ZL_Surface("Data/grass.png").SetTextureRepeatMode().SetScale(0.04f);
	srfClouds = ZL_Surface("Data/clouds.png").SetTilesetClipping(2, 4).SetScale(0.07f);
	srfFloor = ZL_Surface("Data/floor.png").SetDrawOrigin(ZL_Origin::Center).SetRotateOrigin(ZL_Origin::Center).SetScale(0.03f);
	srfWall = ZL_Surface("Data/wall.png").SetDrawOrigin(ZL_Origin::Center).SetRotateOrigin(ZL_Origin::Center).SetScale(0.03f);
	srfDude = ZL_Surface("Data/dude.png").SetDrawOrigin(ZL_Origin::Center).SetRotateOrigin(ZL_Origin::Center).SetScale(0.05f).SetTilesetClipping(4, 2);
	srfMeteor = ZL_Surface("Data/meteor.png").SetDrawOrigin(ZL_Origin::Center).SetRotateOrigin(ZL_Origin::Center).SetScale(0.05f);
	srfWater = ZL_Surface("Data/water.png").SetDrawOrigin(ZL_Origin::Center).SetRotateOrigin(ZL_Origin::Center).SetScale(0.03f);
	srfVolcano = ZL_Surface("Data/volcano.png").SetDrawOrigin(ZL_Origin::BottomCenter).SetScale(0.115f);

	space = cpSpaceNew();
	cpSpaceSetGravity(space, cpv(0.0f, -20));

	ground = cpSpaceAddBody(space, cpBodyNewKinematic());
	cpShape *groundshape = cpSpaceAddShape(space, cpBoxShapeNew(ground, 1000, 2.0f, 0));
	cpShapeSetFriction(groundshape, 100);
	cpBodySetPosition(ground, cpv(0, -1));
	cpShapeSetCollisionType(groundshape, GROUP_GROUND);
	ground->velocity_func = &UpdateVelocityGroundFunc;
}

void CalculateThings()
{
	for (vector<sThing*>::iterator it = Things.begin(); it != Things.end();)
	{
		sThing *pThing = *it;
		cpBody *body = pThing->body;
		if (pThing->excited == false && !World.roundswitch)
			if (pThing->type == sThing::FLOOR || pThing->type == sThing::WALL || pThing->type == sThing::DUDE)
				if (body->a > 0.15 || body->a < -0.15 || body->v.y > 10 || body->v.y < -10)
					{ pThing->excited = true; World.things_excited++; World.score += 100; sndDamage.Play(); }

		if (pThing->type == sThing::METEOR && (body->v.y >= 0 || body->p.y < 0))
		{
			delete pThing;
			it = Things.erase(it);
			sndMeteor.Stop();
		}
		else if (pThing->type == sThing::WATER && (body->arbiterList || (body->p.y < 0 && body->v.y < 0)))
		{
			delete pThing;
			it = Things.erase(it);
		}
		else
		{
			if (pThing->type == sThing::VOLCANO)
			{
				body->p.x = ground->p.x;
				body->v.y = -1-body->p.y;
			}
			++it;
		}
	}
}

void NextRound()
{
	size_t things_before = Things.size();
	scalar nextx = s(200)*s(World.round);
	World.round++;
	switch (World.round)
	{
		case 1:
			World.ZoomTarget = s(1*0.8*0.8*0.8);
			MakeBuilding(nextx, 5, 5, 4, 3);
			break;

		case 2:
			World.ZoomTarget = s(1);
			MakeBuilding(nextx, 12, 6, 4, 4);
			break;

		case 3:
			World.ZoomTarget = s(1*0.8*0.8);
			MakeBuilding(nextx, 4, 10, 5, 5);
			break;

		case 4:
			World.ZoomTarget = s(1*0.8);
			MakeBuilding(nextx-20, 8, 6, 4, 4);
			MakeBuilding(nextx+20, 4, 6, 4, 4);
			break;

		case 5:
			World.ZoomTarget = s(1);
			MakeBuilding(nextx, 10, 10, 5, 5);
			break;

		case 6:
			World.ZoomTarget = s(1*0.8);
			MakeBuilding(nextx-25, 6, 5, 3, 5);
			MakeBuilding(nextx   , 6, 5, 3, 5);
			MakeBuilding(nextx+25, 6, 5, 3, 5);
			break;

		default:
			World.ZoomTarget = s(1*0.8*0.8*0.8);
			{
				int numb = RAND_INT_RANGE(1,5);
				int columns = RAND_INT_RANGE(3, 14-(numb*2));
				cpFloat roomw = RAND_RANGE(3,5);
				cpFloat spaceb = RAND_RANGE(2,6);
				cpFloat roomh = RAND_RANGE(3,5);
				cpFloat groundw = roomw*columns*numb+(spaceb*(numb-1));
				for (int i = 0; i < numb; i++)
				{
					cpFloat x = nextx - (groundw/2.0f) + (roomw*columns/2.0f) + ((roomw*columns+spaceb) * i);
					MakeBuilding(x, RAND_INT_RANGE(3,10), columns, roomw, roomh);
					World.ZoomTarget *= s(1.25);
				}
			}
	}
	if (World.round == 1) { for (int i = 0; i < 100; i++) Clouds.push_back(sCloud(ZL_Vector(-200 + RAND_RANGE(0,200), RAND_RANGE(1,200)))); }
	for (int i = 0; i < 100; i++) Clouds.push_back(sCloud(ZL_Vector(nextx + RAND_RANGE(0,200), RAND_RANGE(1,200))));
	if (World.round > 1) Clouds.erase(Clouds.begin(), Clouds.end()-300);

	if (World.round > 1)
	{
		int randsky = RAND_INT_RANGE(0, (sizeof(arrSkyColorsUp)/sizeof(arrSkyColorsUp[0]))-1);
		scalar dark = RAND_RANGE(0.5, 1.0);
		ZL_Timer::AddTransitionScalar(&colSkyUp.r,   arrSkyColorsUp[randsky].r*dark,   2000);
		ZL_Timer::AddTransitionScalar(&colSkyUp.g,   arrSkyColorsUp[randsky].g*dark,   2000);
		ZL_Timer::AddTransitionScalar(&colSkyUp.b,   arrSkyColorsUp[randsky].b*dark,   2000);
		ZL_Timer::AddTransitionScalar(&colSkyDown.r, arrSkyColorsDown[randsky].r*dark, 2000);
		ZL_Timer::AddTransitionScalar(&colSkyDown.g, arrSkyColorsDown[randsky].g*dark, 2000);
		ZL_Timer::AddTransitionScalar(&colSkyDown.b, arrSkyColorsDown[randsky].b*dark, 2000);
	}

	things_added = (Things.size() - things_before);
}

void sWorld::Calculate()
{
	if (gameover)
	{
		return;
	}
	else if (countdown)
	{
		countdown -= ZLELAPSEDTICKS;
		if (countdown <= 0)
		{
			countdown = 0;
		}
	}
	else if (roundswitch)
	{
		posCameraTarget.x += ZLELAPSEDF(200);
		if (posCameraTarget.x >= s(200)*s(round-1))
		{
			posCameraTarget.x = s(200)*s(round-1);
			ground->p.x = posCameraTarget.x;
			roundswitch = false;
			countdown = 2999;
			size_t remove_until = (Things.size() - things_added);
			for (size_t i = 0; i < remove_until; i++) { delete Things[i]; }
			Things.erase(Things.begin(), Things.begin() + remove_until);
			things_total = (int)Things.size();
			things_excited = 0;
			time = 10*1000;
		}
	}
	else if (things_excited >= ((things_total*10/11)-5))
	{
		if (roundswitchwait == 0)
		{
			imcCleared.Play();
			roundswitchwait = 1000;
		}
		else
		{
			roundswitchwait -= ZLELAPSEDTICKS;
		}
		if (roundswitchwait <= 0)
		{
			roundswitchwait = 0;
			roundswitch = true;
			cpBodySetVelocity(ground, cpvzero);
			NextRound();
		}
	}
	else
	{
		cpBodySetVelocity(ground, cpvmult(ground->v, s(1)-ZLELAPSED));
		time -= ZLELAPSEDTICKS;
		if (time <= 0)
		{
			time = 0;
			gameover = true;
			ZL_Timer::AddTransitionScalar(&colSkyUp.r,   arrSkyColorsUp[0].r,   2000);
			ZL_Timer::AddTransitionScalar(&colSkyUp.g,   arrSkyColorsUp[0].g,   2000);
			ZL_Timer::AddTransitionScalar(&colSkyUp.b,   arrSkyColorsUp[0].b,   2000);
			ZL_Timer::AddTransitionScalar(&colSkyDown.r, arrSkyColorsDown[0].r, 2000);
			ZL_Timer::AddTransitionScalar(&colSkyDown.g, arrSkyColorsDown[0].g, 2000);
			ZL_Timer::AddTransitionScalar(&colSkyDown.b, arrSkyColorsDown[0].b, 2000);
		}
	}

	static ticks_t TICKSUM = 0;
	TICKSUM += ZLELAPSEDTICKS;
	if (TICKSUM > 160) TICKSUM = 160;
	while (TICKSUM > 16) { cpSpaceStep(space, s(16.0/1000.0)); TICKSUM -= 16; }
	CalculateThings();

	Zoom += ZLELAPSEDF(ZoomTarget-Zoom)*2;
	scalar ZoomY = Zoom*ZLHEIGHT/ZLWIDTH;
	posCamera += (posCameraTarget-posCamera).Mul(ZLELAPSED*6);
	view = ZL_Rectf(posCamera.x - 50*Zoom, posCamera.y - 5*ZoomY, posCamera.x + 50*Zoom, posCamera.y + 95*ZoomY);
}

void sWorld::Init()
{
	for (vector<sThing*>::iterator it = Things.begin(); it != Things.end(); ++it) delete *it;
	Things.clear();
	Clouds.clear();
	cpBodySetPosition(ground, cpv(0, -1));
	roundswitch = false;
	roundswitchwait = 0;
	gameover = false;
	countdown = 3000;
	round = 0;
	NextRound();
	Zoom = ZoomTarget;
	posCamera = posCameraTarget = ZL_Vector(0, 0);
	time = 10*1000;
	things_total = (int)Things.size();
	things_excited = 0;
	colSkyUp = arrSkyColorsUp[0];
	colSkyDown = arrSkyColorsDown[0];
	score = 0;
}

void sWorld::Draw()
{
	ZL_Display::ClearFill(colSkyUp);

	ZL_Rectf viewsky(view.MidX() - 50 - Zoom*10, -5, view.MidX() + 50 + Zoom*10, 80 + Zoom*10);

	ZL_Display::PushOrtho(viewsky);
	ZL_Display::FillGradient(-10000, -5, 10000, 100, colSkyUp, colSkyUp, colSkyDown, colSkyDown);
	for (vector<sCloud>::iterator itCloud = Clouds.begin(); itCloud != Clouds.end(); ++itCloud)
	{
		srfClouds.SetTilesetIndex(itCloud->tileindex).Draw(itCloud->pos, itCloud->scale, itCloud->scale, itCloud->col);
	}
	ZL_Display::PopOrtho();

	ZL_Display::PushOrtho(view);

	for (vector<sThing*>::iterator itbg = Things.begin(); itbg != Things.end(); ++itbg)
	{
		sThing *pThing = *itbg;
		cpBody *body = pThing->body;
		switch (pThing->type)
		{
			case sThing::FLOOR:
				if (!pThing->excited)
					ZL_Display::FillRect(((cpPolyShape*)body->shapeList)->planes[0].v0.x, body->p.y-sRoomHeight-0.5f, ((cpPolyShape*)body->shapeList)->planes[2].v0.x, body->p.y, ZLRGB(0.15,0.1,0));
				break;
			case sThing::VOLCANO: srfVolcano.Draw(((cpPolyShape*)body->shapeList)->planes[1].v0); break;
			default:;
		}
	}

	ZL_Display::FillGradient(-10000, -10, 10000, -1.9f, ZLRGB(0,.33f,0), ZLRGB(0,.33f,0), ZL_Color::Black, ZL_Color::Black);
	srfGrass.DrawTo(-10000, -1.95f, 10000, 0.5f);

	for (vector<sThing*>::iterator it = Things.begin(); it != Things.end(); ++it)
	{
		sThing *pThing = *it;
		cpBody *body = pThing->body;
		switch (pThing->type)
		{
			#define CP_BODY_POLY_GET_FIRST_LOCAL_VERT(b) ((cpPolyShape*)b->shapeList)->planes[((cpPolyShape*)b->shapeList)->count].v0
			case sThing::WALL:    srfWall.Draw(body->p, body->a, CP_BODY_POLY_GET_FIRST_LOCAL_VERT(body).x*-0.06f, CP_BODY_POLY_GET_FIRST_LOCAL_VERT(body).y*-0.015f); break;
			case sThing::FLOOR:	  srfFloor.Draw(body->p, body->a, CP_BODY_POLY_GET_FIRST_LOCAL_VERT(body).x*-0.032f, 0.04f); break;
			case sThing::DUDE:    srfDude.SetTilesetIndex(pThing->tile + (pThing->excited ? 1 : 0)).Draw(body->p, body->a); break;
			case sThing::METEOR:  srfMeteor.Draw(cpvadd(body->p, cpvmult(cpvnormalize(body->v), -4)), cpvtoangle(body->v)); break;
			case sThing::WATER:   srfWater.Draw(body->p, cpvtoangle(body->v)); break;
			default:;
		}
	}

	#ifdef ZILLALOG
	void DrawShape(cpShape *shape, void *data);
	if (ZL_Display::KeyDown[ZLK_LSHIFT]) cpSpaceEachShape(space, DrawShape, NULL);
	#endif

	ZL_Display::PopOrtho();
}

#ifdef ZILLALOG
void DrawShape(cpShape *shape, void *data)
{
	cpBody *body = shape->body;
	if (shape->klass->type == CP_SEGMENT_SHAPE) { ZL_Display::DrawLine(((cpSegmentShape*)shape)->ta, ((cpSegmentShape*)shape)->tb, ZLRGBA(1,0,0,.5)); return; }
	else if (shape->klass->type == CP_CIRCLE_SHAPE) { ZL_Display::DrawCircle(((cpCircleShape*)shape)->tc, ((cpCircleShape*)shape)->r, ZL_Color::Red, ZLRGBA(1,0,0,.5)); return; }
	sThing *pThing = (sThing*)shape->userData; ZL_Color col = (body == ground ? ZLBLACK : (pThing ? (pThing->excited ? ZLRGB(1,0,0) : ZLRGB(0,1,0)) : ZLRGB(0,0,1))); cpSplittingPlane *p = ((cpPolyShape*)shape)->planes;
	//ZL_Display::PushMatrix();
	//ZL_Display::Transform(body->transform.tx, body->transform.ty, body->transform.a, body->transform.b);
	ZL_Display::DrawQuad(p[0].v0.x, p[0].v0.y, p[1].v0.x, p[1].v0.y, p[2].v0.x, p[2].v0.y, p[3].v0.x, p[3].v0.y, col, col*ZLRGBA(1,1,1,0.5));
	//ZL_Display::PopMatrix();
}
#endif
