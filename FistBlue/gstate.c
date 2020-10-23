/* gstate.c */

#include "sf2.h"

#include "stagegfx.h"
#include "gstate.h"
#include "structs.h"
#include "player.h"
#include "particle.h"

#include "lib.h"
#include "gfxlib.h"

#include "gemu.h"

#ifdef REDHAMMER
#include <stdio.h>
#include <stdlib.h>
#endif

extern CPSGFXEMU gemu;

extern Game g;
extern int scroll_cursor;

ScrollState gstate_Scroll1;		/* b52 - bd1 */
ScrollState gstate_Scroll2;		/* bd2 - c51 */
ScrollState gstate_Scroll3;		/* c52 - cd1 */
RowScrollState gstate_RowScroll;	/* cd2 - d51 */

static void _GSInitUpdateMethods(void);
static void _GSInitDimensions(void);
static void _GSInitDispEna(void);
static void draw_n_rows(unsigned short *gfx_p, const u16 *tile_p, short count);
static void _GSDrawScroll2C(ScrollState *gs, unsigned short *gfx_p, const u16 *tile_p, CP cp);

static const u16 *_GSLookupScroll1(ScrollState *gs, CP cp);
static const u16 *_GSLookupScroll2(ScrollState *gs, CP cp);
static const u16 *_GSLookupScroll3(ScrollState *gs, CP cp);

static void _GSFillScroll2(ScrollState *gs);
static void _GSStageScroll1(short d0);
static void _GSStageScroll2(short d0);
static void _GSStageScroll3(short d0);
static void _GSDrawScroll1B(ScrollState *gs, u16 *gfx_p, const u16 *a1, CP cp);
static void _GSMaintScroll1Y(ScrollState *gstate);

static void _GSMaintScroll2(ScrollState *gstate);
static void _GSFillScroll3(ScrollState *gs);
static void _GSInitScroll1(ScrollState *gstate);		/* init scroll1 */

static void gstate_update_scroll1(ScrollState *gs);
static void gstate_update_scroll2(ScrollState *gs);
static void gstate_update_scroll3(ScrollState *gs);
static void _GSUpdateRowScroll(RowScrollState *gs, short *a0, short *a1);

static void _GSDrawScroll3A(ScrollState *gs, u16 *gfx_p, const u16 *tilep, CP cp);

#define SCR3X_PERSP 0
#define SCR3X_NONE  2

#define SCR3Y_NONE  0
#define SCR3Y_2Y    2
#define SCR3Y_0_75  4
#define SCR3Y_0_50  6
#define SCR3Y_0_875 8
#define SCR3Y_1_25  10
#define SCR3Y_2Y_MINUS_OFFSET 12

#define SCR2X_FIGHT 0
#define SCR2X_NONE  2

#pragma mark ---- Layer Initialisers ----

static void _GSInitUpdateMethods(void) {
	static const char data_83900[16][6] = {	// Scroll update methods
		{ 0x2, 0x4, 0x0, 0x0, SCR3X_PERSP, 0x4,  },  /* RYU JAPAN */
		{ 0x0, 0x2, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x4, 0x4, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x0, 0x4, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x6, 0x6, 0x0, 0x0, SCR3X_PERSP, 0xc,  },
		{ 0x6, 0x6, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x0, 0x2, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x0, 0xe, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x0, 0x2, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x0, 0x4, 0x0, 0x0, SCR3X_PERSP, 0xa,  },
		{ 0x0, 0x4, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x0, 0x2, 0x0, 0x0, SCR3X_PERSP, 0x2,  },
		{ 0x6, 0x0, 0x2, 0x2, SCR3X_NONE, 0x2,  },
		{ 0x6, 0x2, 0x2, 0x0, SCR3X_NONE, 0x2,  },
		{ 0x4, 0x2, 0x2, 0x0, SCR3X_NONE, 0x2,  },
		{ 0x6, 0x2, 0x2, 0x0, SCR3X_NONE, 0x2,  },
	};
	
    gstate_Scroll1.XUpdateMethod = data_83900[g.CurrentStage][0];
    gstate_Scroll1.YUpdateMethod = data_83900[g.CurrentStage][1];
    gstate_Scroll2.XUpdateMethod = data_83900[g.CurrentStage][2];
    gstate_Scroll2.YUpdateMethod = data_83900[g.CurrentStage][3];
    gstate_Scroll3.XUpdateMethod = data_83900[g.CurrentStage][4];
    gstate_Scroll3.YUpdateMethod = data_83900[g.CurrentStage][5];
}  
static void _GSInitDimensions(void) {
	
	static const u16 data_83960[16][4] = {		// Max dimensions of stages
		// MinX   MaxX     MinY   MaxY
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },	// 384 pixel difference, 2 screenwidths
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0100, 0x0280, 0x0000, 0x0010,  },
		{ 0x0000, 0x0000, 0x0000, 0x0010,  },
		{ 0x0000, 0x0000, 0x0000, 0x0010,  },
		{ 0x0000, 0x0000, 0x0000, 0x0010,  },
		{ 0x0000, 0x0000, 0x0000, 0x0010,  },
	};
	static const u16 data_839e0[16]={            /* ground planes stupid, same for all levels */
		0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028,
		0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028,  
	};
	
	g.StageMinX = data_83960[g.CurrentStage][0];
    g.StageMaxX = data_83960[g.CurrentStage][1];
    g.StageMinY = data_83960[g.CurrentStage][2];
    g.StageMaxY = data_83960[g.CurrentStage][3];

	g.GroundPlaneY = data_839e0[g.CurrentStage];
}
static void _GSInitDispEna(void) {
	static const u16 data_83ac0[20]={		// DISPENAs
		0x0b40, 0x06c0, 0x0e40, 0x0e40, 0x0e40, 0x0b40, 0x12c0, 0x0780, 0x0780, 0x3240,
		0x0e40, 0x0780, 0x06c0, 0x06c0, 0x0e40, 0x0e40, 0x302e, 0x0006, 0x0440, 0x00c0,
	};
	
    short temp = g.CPS.DispEna & 0x1b;
    g.CPS.DispEna = temp | data_83ac0[g.CurrentStage];
}



#pragma mark ---- Coordinate Calculators ----

static CP _GSCoordOffsetScr1 (ScrollState *gs, short offset) {	// 83d68 checked
    CP cp;
    static const short data_83d84[4][2] = {
		{   -64,   352,  },
		{   416,   352,  },
		{   -64,  -128,  },
		{   -64,   352,  },
	};
	
    offset &= 0xf;
    cp.x =   gs->XPI + data_83d84[offset/4][0];
    cp.y = ~(gs->YPI + data_83d84[offset/4][1]);
    
    return cp;
}
static CP _GSCoordOffsetScr2 (ScrollState *gs, short offset) {
	
	static const short data_83db0[4][2] = {
		{  -144,   384,  },
		{   512,   384,  },
		{  -144,  -144,  },
		{  -144,   384,  },
	};
	
    CP cp;
    offset &= 0xf;
    cp.x = gs->XPI + data_83db0[offset/4][0];
    cp.y = ~(gs->YPI + data_83db0[offset/4][1]);
    
    return cp;
}
static CP _GSCoordOffsetScr3 (ScrollState *gs, short offset) {
	static const short data_83ddc[4][2] = {
		{  -160,   384,  },
		{   512,   384,  },
		{  -160,  -160,  },
		{  -160,   384,  },
	};
	
    CP cp;
    offset &= 0x000f;
    cp.x =   gs->XPI + data_83ddc[offset/4][0];
    cp.y = ~(gs->YPI + data_83ddc[offset/4][1]);
    
    return cp;
}
    
static u16 *_GSCoordsScroll1(CP cp){			//83dec checked 
	return gemu.Tilemap_Scroll1[	
			((cp.y & 0x100) << 3) +			/*      yxxx x00y yy00   */
			((cp.y & 0xe0) >> 3)  +	    
			((cp.x << 2) & 0x0780)
	];
}
static u16 *_GSCoordsScroll2(CP cp){
#ifdef CPS							
    return  (void *)(CPS_VIDEO_SCROLL2 + 
                ((cp.y & 0x300) << 4) +
                ((cp.y & 0xf0) >> 2)  +			
                ((cp.x << 2) & 0xfc0)
    );
#else
	return gemu.Tilemap_Scroll2[
				((cp.y & 0x300) << 2) |
				((cp.y & 0x0f0) >> 4) |
				(cp.x & 0x3f0)
								];
#endif
}    
static void *_GSCoordsScroll3(CP cp){
#ifdef CPS
    return  (void *)(CPS_VIDEO_SCROLL3 + 
                ((cp.y & 0x700) << 3) +
                ((cp.y & 0xe0) >> 3)  +
                (cp.x & 0x7e0)
    );
#else
	return gemu.Tilemap_Scroll3[ 
				((cp.y & 0x700) << 1) |
				((cp.y & 0x0e0) >> 5) |
				((cp.x >> 2) & 0x1f8) 
					];
#endif
} 

#pragma mark ---- Tilemap Lookup ----

static short _GSCalcBlockIndex(ScrollState *gs, CP cp) {   
	/* 0x83ee6 x and y div by 256 was cacl_gstate_2022 */
    short index;
    if (gs->SpecialStage) {
        index = ((cp.y & 0x700) >> 4) + ((cp.x & 0x700) >> 7);	// 0YYY XXX0
    } else {
        index = ((cp.y & 0x100) >> 5) + ((cp.x & 0x300) >> 7);	// YXX0
    }
    gs->Index        = index;
    gs->InitialIndex = index;
    return RHWordOffset(gs->TileMaps, index / 2);// gs->TileMaps[index/2];
}

static const u16 *_GSLookupScroll1(ScrollState *gs, CP cp) {	/* 83e5c */
	/* No ATTRs */
    const u16 *temp2;
    	
    temp2 = data_d6800[ _GSCalcBlockIndex(gs, cp) ]; 

	/* xxx yyy0 64 vctors  */
	gs->YCoarse = (cp.y & 0xe0) >> 4;
    gs->XCoarse = (cp.x & 0xe0) >> 1;

    temp2 += (gs->YCoarse + gs->XCoarse)/2;
    return temp2;
}

static const u16 *_GSLookupScroll2(ScrollState *gs, CP cp) {	/* 83e8a */
    const u16 *temp2;
    
    temp2 = data_e0000[_GSCalcBlockIndex(gs, cp)];
	
    /* xx xxyy yy00  256 tiles + attr in u16 */
	gs->YCoarse = (cp.y & 0xf0) >> 2;
    gs->XCoarse = (cp.x & 0xf0) << 2;
    
	
	temp2 += (gs->YCoarse + gs->XCoarse)/2;
    return temp2;
}

static const u16 *_GSLookupScroll3(ScrollState *gs, CP cp) {	/* 83eba */
    const u16 *temp2;
    
    temp2 = data_d8000[_GSCalcBlockIndex(gs, cp)];
    gs->YCoarse = (cp.y & 0xe0) >> 3;
    gs->XCoarse = (cp.x & 0xe0);		/* xxxy yy00 */
    temp2 += (gs->YCoarse + gs->XCoarse)/2;
    return temp2;
}

#pragma mark ---- Scroll Maint ----

static void _gstate_against_right(ScrollState *gstate, int d1) {         // sf2ua:832f2
	int d3;
	/* missing GPCollDetect redundant */
	if (d1 > 6) {
		d1 = 6;
	}
	d3 = gstate->XPI;
	gstate->XPI += d1;
	if (gstate->XPI > g.StageMaxX ) {
		gstate->XPI = g.StageMaxX;
		gstate->XOff = g.StageMaxX - d3;
	} else {
		gstate->XOff = d1;
	}
	gstate->x0024 = 4;
}

static void _gstate_against_left(ScrollState *gstate, int d0) {		// sf2ua:83334   d0 is negative
	int d3;
	/* missing GPCollDetect redundant */
	if (d0 < -6) {
		d0 = -6;
	}
	d3 = gstate->XPI;
	gstate->XPI += d0;
	if (gstate->XPI < g.StageMinX ) {
		gstate->XPI = g.StageMinX;
		gstate->XOff = g.StageMinX - d3;
	} else {
		gstate->XOff = d0;
	}
	gstate->x0024 = 0xc;
}

static void _GSMaintScroll3X(ScrollState *gs) {		// 83658
	gs->x0024 = gstate_Scroll2.x0024;
	switch (gs->XUpdateMethod) {
		case SCR3X_PERSP:		// scroll 2 + Zdepth
			gs->XPI = gstate_Scroll2.XPI + (gstate_RowScroll.Scroll3Parallax.part.integer - (SCREEN_WIDTH / 2));
			break;
		case SCR3X_NONE:
			// do nothing
			break;
		FATALDEFAULT;
	}
}

static void _GSMaintScroll3Y(ScrollState *gs) {		// 8368c
	if (g.ScreenWobbleMagnitude) {
		return;
	}
    gs->x0025 = gstate_Scroll2.x0025;
    switch (gs->YUpdateMethod) {
		case 0x2:
			gs->YPI = gstate_Scroll2.YPI;
			break;
		case 0x0:
			break;
		case 0x4:
			gs->Y.full += gstate_Scroll2.YOff * 0xc000 ; /* 0.75 */
			break;
		case 6:
			gs->Y.full += gstate_Scroll2.YOff * 0x8000;  /* 0.5 */
			break;
		case 8:
			gs->Y.full += gstate_Scroll2.YOff * 0xe000;  /* 0.875 */
			break;
		case 10:
			gs->Y.full += gstate_Scroll2.YOff * 0x14000; /* 1.25 */
			break;
		case 12:
			gs->YPI += gstate_Scroll2.YPI - gs->YOff;
			gs->YOff = 0;
			break;
    }
}

static void update_scroll2_X (ScrollState *gstate) {     /* 0x83270 */
	short leftmost_x, rightmost_x;
	short d0;
	Player *left, *right;
	switch (gstate->XUpdateMethod) {
		case SCR2X_FIGHT:
			if (g.Player1.XPI <= g.Player2.XPI) {
				leftmost_x    = g.Player1.XPI;
				rightmost_x   = g.Player2.XPI;
				left  = PLAYER1;
				right = PLAYER2;
			} else {
				leftmost_x    = g.Player2.XPI;
				rightmost_x   = g.Player1.XPI;
				left  = PLAYER2; 
				right = PLAYER1;
			}
			leftmost_x  -= left ->Size;
			rightmost_x += right->Size;

			if ((rightmost_x - leftmost_x) < 256) {
				if (rightmost_x - gstate->XPI  >= (SCREEN_WIDTH - 64)) {
					_gstate_against_right(gstate,rightmost_x - gstate->XPI - (SCREEN_WIDTH - 64));
				} else if (leftmost_x - gstate->XPI < 64) {
					_gstate_against_left(gstate,leftmost_x - gstate->XPI - 64);
				}
			} else if ((rightmost_x - leftmost_x) < SCREEN_WIDTH) {             // XXX checkme
				d0=(left->XPI - left->Size + right->XPI + right->Size)/2;
				d0-=gstate->XPI;
				d0-= SCREEN_WIDTH / 2;
				if (d0<0) {
					_gstate_against_left(gstate, d0);
				} else {
					_gstate_against_right(gstate, d0);					
				}
			}
			break;
		case SCR2X_NONE:
			/* empty method, nothing happens */
			break;
		FATALDEFAULT;
	}
}
static void update_scroll2_Y (ScrollState *gstate) { /* 0x3=83376 */
	switch (gstate->YUpdateMethod) {
		case 0:
			if (g.x0ade || g.ScreenWobbleMagnitude) { return; }
			
			/* XXX not done */
			
			break;
		case 2:
			// do nothing
			break;
		FATALDEFAULT;
	}
}

#define QUARTER_PIXEL 0x4000

static void _GSMaintScroll1X(ScrollState *gs) {	// 834d0
    short int temp;
	
    gs->x0024 = gstate_Scroll2.x0024;
    switch (gs->XUpdateMethod) {
		case 0:         // follow parallax from linescroll
			temp = gs->X.part.integer;
			gs->XPI = gstate_Scroll2.XPI + gstate_RowScroll.Scroll1Parallax.part.integer - (SCREEN_WIDTH / 2);
			g.x8b14 = gs->XPI - temp;
			break;
		case 2:
			gs->X.full += QUARTER_PIXEL;		/* plus 0.25 */
			if(gs->mode1 == 0 && gs->XPI >= 0x200) {
				gs->mode1 += 2;				
				gs->YPI += 0x100;
			}
			gs->x0024 = 4;
			gs->XPI  -= g.x8c02;
			break;
		case 4:
			gs->X.full += QUARTER_PIXEL;
			gs->x0024   = 4;
			gs->XPI -= g.x8c02;
			break;
		case 6:           /* does nothing */
			break;
		FATALDEFAULT;
    }
}
static void _GSMaintScroll1Y(ScrollState *gstate) {    /* 83558 */
    if(g.ScreenWobbleMagnitude != 0)  {return;}
    
    gstate->x0025 = gstate_Scroll2.x0025;
    switch (gstate->YUpdateMethod) {
		case 2:
			gstate->YPI = gstate_Scroll2.YPI;
			break;
		case 0:
			break;
		case 4:
			gstate->Y.full += gstate_Scroll2.YOff * 128;   /* XXX all need <<4 */
			break;
		case 6:
			gstate->Y.full += gstate_Scroll2.YOff * 320;
			break;
		case 8:
			gstate->Y.full += gstate_Scroll2.YOff * 288;
			break;
		case 10:
			gstate->Y.full += gstate_Scroll2.YOff * 224;
			break;
		case 12:
			gstate->Y.full += gstate_Scroll2.YOff * 160;
			break;
		case 14:
			gstate->YPI += gstate_Scroll2.YOff;
			break;
    }
}


static void _GSMaintScroll3(ScrollState *gs) {   /* 835ec */
	switch (gs->mode0) {
		case 0:
			switch (gs->mode1) {
				case 0:
					NEXT(gs->mode1)
					gs->x001e = gs->x001f = gs->x0024 = gs->x0025 = 0;
					break;
				case 2:
					NEXT(gs->mode1)
					_GSFillScroll3(gs);
					break;
				case 4:
					NEXT(gs->mode1)
					break;
				case 6:
					NEXT(gs->mode0)
					gs->mode1 = 0;
					break;
			}
			break;
		case 2:
			_GSMaintScroll3X(gs);
			_GSMaintScroll3Y(gs);
			g.CPS.Scroll3X = gs->XPI;
			g.CPS.Scroll3Y = gs->YPI;
			break;
    }
}

#define SCROLL_MAINT_INIT   0
#define SCROLL_MAINT_NORM   2

static void _GSMaintScroll2(ScrollState *gstate){      /* 831ca was nextlevel_dosetups */
	switch (gstate->mode0) {
		case SCROLL_MAINT_INIT:
			switch (gstate->mode1) {
				case 0x0:
					gstate->mode1 +=2 ;
					_GSStageScroll1(g.CurrentStage);
					_GSStageScroll2(g.CurrentStage);
					_GSStageScroll3(g.CurrentStage);
					
					_GSInitDimensions();
					_GSInitUpdateMethods();
					_GSInitDispEna();
					
					gstate-> x001e = 0;
					gstate-> x001f = 0;
					gstate-> x0024 = 0;
					gstate-> x0025 = 0;
					g.x8a4b = 0;		
					/* CPS code ends with a BTST !? 0x83222 */
					break;
				case 0x2:
					gstate->mode1 += 2;
					break;
				case 0x4:
					gstate->mode1 += 2;
					_GSFillScroll2(gstate); /*  83ae0 fill scroll2 tilemap from ROM */
					break;
				case 0x6:
					gstate->mode0 += 2;
					gstate->mode1  = 0;
					g.GSInitComplete = TRUE;
					break;
			}
			break;
		case SCROLL_MAINT_NORM:
			gstate->XOff = 0;
			gstate->YOff = 0;
			update_scroll2_X(gstate);   /*   sub_83270 player horiz scrolling */
			update_scroll2_Y(gstate);		/* sub_83376 same for vert */
//            if(g.OnBonusStage) {    // XXX
              if(1) {
				g.CPS.Scroll2X = gstate->XPI;
			} else {
                // compensate for Rowscroll
				g.CPS.Scroll2X = gstate->XPI - (SCREEN_WIDTH / 2);
			}
			g.CPS.Scroll2Y = gstate->YPI;
    }
}
static void _GSMaintScroll1(ScrollState *gs) {		// 8343a

    switch (gs->mode0) {
		case 0:
			switch (gs->mode1) {
				case 0:
					NEXT(gs->mode1)
					_GSInitScroll1(gs);
					break;
				case 2:
					NEXT(gs->mode1)
					gs->x001e = gs->x001f = gs->x0024 = gs->x0025 = 0;
					gstate_update_scroll1(gs);
					break;
				case 4:
					NEXT(gs->mode1)
					break;
				case 6:
					NEXT(gs->mode0)
					gs->mode1 = 0;
					break;
				FATALDEFAULT;
			}
			break;
		case 2:
			_GSMaintScroll1X(gs);
			_GSMaintScroll1Y(gs);
			gstate_update_scroll1(gs);
			break;
		FATALDEFAULT;
    }
}
#define BUMP_2BE(increase)											\
	{																\
		int offset = (g.x02be - gemu.RowScroll2) * sizeof(short);	\
		offset += (increase);										\
		offset &= 0xffff3000;										\
		g.x02be = gemu.RowScroll2 + (offset / sizeof(short));		\
	}																\


static void _GSMaintRowScroll(RowScrollState *ss) {	/* 84480 */
	short *a1;

    switch (ss->mode0) {
		case 0:
			/* 84496 */
			NEXT(ss->mode0);
			ss->CenterX    = 448;	// not actually zdepth, but Scr2X where parallax is zero
            // in other words, center of the stage
			ss->ss_0010         = 630;          // ~FP(0.0096) pixels per pixel per row
			ss->GroundRow       = 984 * 2;
			g.CPS.RowScrollBase = 0x9200;
#ifdef CPS
			g.x02be = (short *)0x921000;
#endif
#ifdef REDHAMMER
			g.x02be = gemu.RowScroll2;
            if (ss->planes) {
                free(ss->planes);
                ss->planes = NULL;
            }
            
            switch (g.CurrentStage) {
                case STAGE_INDIA_DHALSIM:
                    ss->planes = calloc(sizeof(RHTilePlane), 2);
                    if (!ss->planes) {
                        FBPanic(1);
                    }
                    ss->nPlanes = 2;
                    ss->planes[0] = (RHTilePlane){  0,  63, -24, 40 };
                    ss->planes[1] = (RHTilePlane){ 64, 256,  40, 40 };
                    break;
                default:
                    
                    /* XXX remove me */
                    ss->planes = calloc(sizeof(RHTilePlane), 2);
                    ss->planes[0] = (RHTilePlane){  0,  63, -24, 40 };
                    ss->planes[1] = (RHTilePlane){ 64, 256,  40, 40 };
                    break;
            }
#endif
			ss->x0024 = 0;
			/* returns BTST #0, %d1 why???  */
			break;
		case 2:
			/* 84546 */
			g.CPS.RowScrollBase = (g.CPS.RowScrollBase + 0x10) & 0xfff30;

            // XXX double buffer rowscroll
            //BUMP_2BE(0x1000);
			
			a1 = &ss->ss_001c[ss->x0024 / 4];
            // xxx lookup 84564 - 2e
			ss->x0024 = (ss->x0024 + 4) & 0xc;		
			ss->YPI = gstate_Scroll2.YPI;
			g.CPS.RowScrollOffset = ss->YPI;
			if (a1[0] != gstate_Scroll2.XPI) {
				a1[0] = gstate_Scroll2.XPI;
				_GSUpdateRowScroll(ss, g.x02be, a1);
			}
			break;
		FATALDEFAULT;
	}
}

static void _GSStageScroll1 (short d0) {	/* 83730 */
	int bonus;					/* not actually bonus, but VS Screen, etc */
	//bonus = data_8375c[d0];
	bonus = d0 >= 0x10 ? TRUE : FALSE;		/* more compact than a U16 LUT */
	
    gstate_Scroll1.SpecialStage = bonus * 2;
    gstate_Scroll1.Offset  = RH2DWord(0x83834, 4, bonus / 2, 0); /* 0x8 or 0x10 */
    gstate_Scroll1.OffMask = RH2DWord(0x83834, 4, bonus / 2, 1); /* 0xe or 0x7e */
    gstate_Scroll1.gs_001a = RH2DWord(0x83834, 4, bonus / 2, 2); /* 0x6 or 0xe */
    gstate_Scroll1.gs_001c = RH2DWord(0x83834, 4, bonus / 2, 3); /* 0x8 or 0x70 */

    gstate_Scroll1.TileMaps = RH1DLong(0x89ebc, d0);
}
static void _GSStageScroll2(short d0) {
	short bonus;		/* not actually bonus but special stages */
    //bonus = data_837b4[g.CurrentStage];     /* same as 0x8375c */
	bonus = d0 >= 0x10 ? 2: 0;

    gstate_Scroll2.SpecialStage = bonus;
    gstate_Scroll2.Offset  = RH2DWord(0x83834, 4, bonus / 2, 0); /* 0x8 or 0x10 */
    gstate_Scroll2.OffMask = RH2DWord(0x83834, 4, bonus / 2, 1); /* 0xe or 0x7e */
    gstate_Scroll2.gs_001a = RH2DWord(0x83834, 4, bonus / 2, 2); /* 0x6 or 0xe */
    gstate_Scroll2.gs_001c = RH2DWord(0x83834, 4, bonus / 2, 3); /* 0x8 or 0x70 */

    
    gstate_Scroll2.TileMaps = RH1DLong(0x89f0c, d0);
}

static void _GSStageScroll3(short d0) {	/* 837e0 */
    int bonus;
    bonus = d0 >= 0x10 ? 2 : 0 ;

    gstate_Scroll3.SpecialStage = bonus;		// not actually bonus but scrolldisable
    gstate_Scroll3.Offset  = RH2DWord(0x83834, 4, bonus / 2, 0); /* 0x8 or 0x10 */
    gstate_Scroll3.OffMask = RH2DWord(0x83834, 4, bonus / 2, 1); /* 0xe or 0x7e */
    gstate_Scroll3.gs_001a = RH2DWord(0x83834, 4, bonus / 2, 2); /* 0x6 or 0xe */
    gstate_Scroll3.gs_001c = RH2DWord(0x83834, 4, bonus / 2, 3); /* 0x8 or 0x70 */
    
    gstate_Scroll3.TileMaps = RH1DLong(0x89f5c, d0);
}

/* 
static void gstate_nextlevel_scroll1 (void) {   // 0x8372c 
	_GSStageScroll1(g.CurrentStage);
}
static void gstate_nextlevel_scroll2 (void) {	// 0x83784 
	_GSStageScroll2(g.CurrentStage);
}
static void gstate_nextlevel_scroll3 (void) {
	_GSStageScroll3(g.CurrentStage);
}
 */


#pragma mark ---- Tilemap Fillers ----
static void _GSFillScroll2(ScrollState *gs) {  /* 0x83ae0 fill scroll2 from tilemap */
    int i,j;
    CPSCOORD gfx_p;
	
    g.CPS.Scroll2X = gs->XPI - (SCREEN_WIDTH / 2);
    g.CPS.Scroll2Y = gs->YPI;
    SCR2_CURSOR_SET(gfx_p, 0, 32);         /* starting at tile 0x800:   
                              0x906000 = 0x904000 + (0x800 * 4) */
    
	for (i=0; i<8; i++) {
        for (j=0; j<256; j++) {
            SCR2_DRAW_TILE(
                gfx_p,
                data_e0000[ RHWordOffset(gs->TileMaps, i) ][j*2],
                data_e0000[ RHWordOffset(gs->TileMaps, i) ][(j*2)+1]
            );
            SCR2_CURSOR_BUMP(gfx_p, 0, 1);
        }
    }
}

static void _GSFillScroll3(ScrollState *gs) {        /* 0x83b2a fill scroll3 from tilemap */
    int i,j;
    CPSCOORD gfx_p;
    
    g.CPS.Scroll3X = gs->XPI;
    g.CPS.Scroll3Y = gs->YPI;
    
	SCR3_CURSOR_SET(gfx_p, 0, 24);	
	
	for (i=0; i<4; i++) {
        for (j=0; j<64; j++) {
            SCR3_DRAW_TILE(gfx_p, data_d8000[ RHWordOffset(gs->TileMaps, i) ][ j*2 ],data_d8000[ RHWordOffset(gs->TileMaps, i) ][ (j*2) +1]);
            SCR3_CURSOR_BUMP(gfx_p, 0, 1);
        }
    }
    	
    SCR3_CURSOR_SET(gfx_p, 0, 56);         /* starting at tile y=56 */

    for(i=4;i<8; i++) {
        for(j=0; j<64; j++) {
            SCR3_DRAW_TILE(gfx_p, data_d8000[ RHWordOffset(gs->TileMaps, i) ][ j*2 ], data_d8000[ RHWordOffset(gs->TileMaps, i) ][ (j*2)+1 ]);
            SCR3_CURSOR_BUMP(gfx_p, 0, 1);
        }
    }    
}
static void _GSInitScroll1(ScrollState *gstate) {	/* 83b78 checked init scroll1 */
	CP cp;		
	short i;
	const u16 *a1;
	u16 *gfx_p;
	
	g.CPS.Scroll1X = gstate->XPI;
	g.CPS.Scroll1Y = gstate->YPI;
	
	cp.x = gstate->XPI - 0x40;
	cp.y = ~(gstate->YPI + 0x160);
	
	for (i=0; i<16; ++i) {
		gfx_p = _GSCoordsScroll1(cp);
		a1 = _GSLookupScroll1(gstate, cp);
		_GSDrawScroll1B(gstate, gfx_p, a1, cp);
		cp.x += 0x20;
	}
}

// u16       yxxx xxxy yyyy[2]		2y x 64y x 32y x 2words x u16  tiles 8x8
// void 00yx xxxx xyyy yy00			rowmask 0x1f80
static const u16 *_GSRealignScr1a(ScrollState *gs, u16 **gfx_p) {	// 84052 checked
	int element1, element2;
	
	*gfx_p -= 8;
	
	element2 = element1 = (*gfx_p - gemu.Tilemap_Scroll1[0]) * sizeof(u16);
	//element2 = *gfx_p;
	
	element1 &= 0xcf80;	// top of this column
	element2 += 0x2000;
	element2 &= 0x3000;
	
	*gfx_p = gemu.Tilemap_Scroll1[0] + ((element1 | element2) / sizeof(u16));
	
	gs->Index = (gs->Index + gs->Offset) & gs->OffMask;
	return &data_d6800[ RHWordOffset(gs->TileMaps, gs->Index / 2) ][ gs->XCoarse/2 ];
}

static const u16 *_GSRealignScroll2A(ScrollState *gs, u16 **gfx_p) {
	// 840e4 scroll2
	u32 d0;
	u32 d1;
	int offset;
	
#ifdef CPS
#warning Scroll cursor arithmetic not optimal for CPS
#endif
	*gfx_p -= 2;
	offset = (*gfx_p - BMAP_SCROLL2) * sizeof(u16);
	d0 = offset & 0xffffcfc3;
	d1 = (offset + 0x1000) & 0x3000;
	d0 |= d1;
	*gfx_p = (u16 *)BMAP_SCROLL2 + (d0 / sizeof(u16));
	
	gs->Index = (gs->Index + gs->Offset) & gs->OffMask;
	return &data_e0000[ RHWordOffset(gs->TileMaps, gs->Index / 2) ][gs->XCoarse/2];
}
static const u16 *skyscraper_realign(ScrollState *gs, u16 **gfx_p) {			// 84384
	u32 d0;
	u32 d1;
	int offset;
	
#ifdef CPS
#warning Scroll cursor arithmetic not optimal for CPS
#endif
	*gfx_p -= 0x20;
	offset = (*gfx_p - BMAP_SCROLL2) * sizeof(u16); 
	d0 = offset & 0xfffff000;
	d1 = (offset + 0x40) & 0xfff;
	d0 |= d1;
	*gfx_p = (u16 *)BMAP_SCROLL2 + (d0 / sizeof(u16));
	
	gs->InitialIndex = ((gs->InitialIndex + 2) & gs->gs_001a) | (gs->InitialIndex & gs->gs_001c);	
	return &data_e0000[ RHWordOffset(gs->TileMaps, gs->InitialIndex / 2) ][gs->YCoarse/2];
}

//void decode_scr3_coord(int offset) {
//	int x = (offset & 0x3e0) >> 5;
//	int y = ((offset & 0x1c00) >> 8) + ((offset & 0x1c) >> 2);
//	printf("X:%d Y:%d ",x, y);
//}

static const u16 *realign_scr3a(ScrollState *gs, u16 **gfx_p) {	
	// 84178 for realigning scroll3 cursor
	u32 d0;
	u32 d1;
	int offset;
#ifdef CPS
#warning Scroll cursor arithmetic not optimal for CPS 
#endif
	*gfx_p -= 2;
	offset = (*(gfx_p) - BMAP_SCROLL3) * sizeof(u16); 
	d0 = offset & 0xffffc7e3;
	d1 = (offset + 0x800) & 0x3800;
	d0 |= d1;
	*gfx_p = (u16 *)BMAP_SCROLL3 + (d0 / sizeof(u16));
	
	gs->Index = (gs->Index + gs->Offset) & gs->OffMask;
	return &data_d8000[ RHWordOffset(gs->TileMaps, gs->Index / 2) ][ gs->XCoarse/2 ];
}
static const u16 *realign_scr3b(ScrollState *gs, u16 **gfx_p) {		// 8442a for scroll3
	u32 d0;
	u32 d1;
	int offset;
#ifdef CPS
#warning Scroll cursor arithmetic not optimal for CPS 
#endif	
	*gfx_p -= 16;
	offset = (*gfx_p - BMAP_SCROLL3) * sizeof(u16); 
	d0 = offset & 0xfffff800;
	d1 = (offset + 0x20) & 0x7ff;
	d0 |= d1;
	*gfx_p = (u16 *)BMAP_SCROLL3+(d0 / sizeof(u16));
	
	gs->Index = (gs->Index + gs->Offset) & gs->OffMask;
	
	return &data_d8000[ RHWordOffset(gs->TileMaps, gs->Index / 2) ][ gs->XCoarse/2 ];
}

static void _GSDrawScroll1A(const u16 **tilep_a1, u16 **gfx_p_a0, int numrows_d0) {		/* 83f70 */
    int x,y,youter;
    u16 *target;
	
    const u8 *cursor;
    for(youter=0;youter<=numrows_d0; youter++) {
		cursor = data_d0000[ (*tilep_a1)[youter]];	
		for (x=0; x<4; x++) {
			for(y=0;y<4; y++) {        /* unrolled */
				target = *gfx_p_a0 + (y * 2) + (x * 64);	
				SCR1_DRAW_TILE(target, (cursor[0] << 8) + cursor[1], cursor[2] );
				cursor += 3;
			}
		}
		*gfx_p_a0 += 8;
    }
}
static void _GSDrawScroll1B(ScrollState *gs, u16 *gfx_p, const u16 *a1, CP cp) {  /* 83f40 */
    int d3;
    int d0;
	
    d0 = ((~cp.y) & 0xe0) >> 5;
    d3 = d0;
    _GSDrawScroll1A(&a1, &gfx_p, d0);
	
    a1 = _GSRealignScr1a(gs, &gfx_p);
    d0 = 14 - d3;
    if(d0 <= 7) {
        _GSDrawScroll1A(&a1, &gfx_p, d0);
    } else {
		d3 = d0;
		_GSDrawScroll1A(&a1, &gfx_p, 7);
		a1 = _GSRealignScr1a(gs, &gfx_p);
		_GSDrawScroll1A(&a1, &gfx_p, d3-8);
	}
}

static void _GSDrawScroll2A(ScrollState *gs, u16 *gfx_p, const u16 *tilep, CP cp) {  /* 840a4 */
	short d0 = (cp.y & 0xf0) >> 4;
	short d3 = d0;
	for (; d0>0; d0--) {			/* inlined 840dc */
		*gfx_p++ = *tilep++;
		*gfx_p++ = *tilep++;
	}
	tilep = _GSRealignScroll2A(gs, &gfx_p);
	for (d0=0xf; d0>0; d0--) {			/* inlined 840dc */
		*gfx_p++ = *tilep++;
		*gfx_p++ = *tilep++;
	}
	tilep = _GSRealignScroll2A(gs, &gfx_p);
	d3 += 0x10;
	d0 = 0x20 - d3;
	if (d0 > 0xf) {
		for (; d0>0; d0--) {			/* inlined 840dc */
			*gfx_p++ = *tilep++;
			*gfx_p++ = *tilep++;
		}	
	} else {
		for (d0 = 0xf; d0 > 0; d0--) {			/* inlined 840dc */
			*gfx_p++ = *tilep++;
			*gfx_p++ = *tilep++;
		}
		tilep = _GSRealignScroll2A(gs, &gfx_p);
		*gfx_p++ = *tilep++;
		*gfx_p++ = *tilep++;
	}
}

inline static void draw_n_rows(u16 *gfx_p, const u16 *tile_p, short n_cols) {			// 84374
	int i;
    for(i=0; i<=n_cols; i++) {
        SCR2_DRAW_TILE(gfx_p, *tile_p, *tile_p+1);
        SCR2_CURSOR_BUMP(gfx_p,  1, 0);
        SCR2_CURSOR_BUMP(tile_p, 1, 0);
    }
}

static void _GSDrawScroll2C(ScrollState *gs, u16 *gfx_p, const u16 *tile_p, CP cp) {		// 84336
    short d2, d0;
    d2 = ((~cp.x) & 0xf0) >> 4;
    
    draw_n_rows(gfx_p, tile_p, d2);
    
    tile_p=skyscraper_realign(gs, &gfx_p);
    draw_n_rows(gfx_p, tile_p, 15);
    
    tile_p=skyscraper_realign(gs, &gfx_p);
    d2 += 17;
    d0 = 41 - d2;
    if(d0 < 15) {
        draw_n_rows(gfx_p, tile_p, d0);
    } else {
		draw_n_rows(gfx_p, tile_p, 15);
		tile_p=skyscraper_realign(gs, &gfx_p);
		draw_n_rows(gfx_p, tile_p, d0 - 16);
	}
}

static void sub_84170(int lines, u16 **gfx_p, const u16 **tilep) {
	for (; lines >= 0; --lines) {		
		*(*gfx_p)++ = *(*tilep)++;
		*(*gfx_p)++ = *(*tilep)++;
	}	
}


static void _GSDrawScroll3A(ScrollState *gs, u16 *gfx_p, const u16 *tilep, CP cp) {  /* 84138 for Scroll3 was funky2_draw*/
	short d0 = ((~cp.y)  & 0xe0) >> 5;		// y / 32
	short d3 = d0;
	sub_84170(d0, &gfx_p, &tilep);

	tilep = realign_scr3a(gs, &gfx_p);
	sub_84170(7, &gfx_p, &tilep);

	tilep = realign_scr3a(gs, &gfx_p);
	d3 += 0x8;
	d0 = 0x10 - d3;
	if (d0 < 8) {
		sub_84170(d0, &gfx_p, &tilep);
	} else {
		sub_84170(7, &gfx_p, &tilep);
		tilep = realign_scr3a(gs, &gfx_p);
		sub_84170(1, &gfx_p, &tilep);
	}
}
static void _GSDrawScroll3B(ScrollState *gs, u16 *gfx_p, const u16 *tilep, CP cp) {  /* 843dc was funky3_draw*/
	short d0 = (cp.y & 0xe0) >> 5;	// blocks of 32
	short d2 = d0;
	for (; d0 >= 0; --d0) {			
		gfx_p[0] = tilep[0];
		gfx_p[1] = tilep[1];
		gfx_p += 0x10;
		tilep += 0x10;
	}
	tilep = realign_scr3b(gs, &gfx_p);
	for (d0 = 0; d0 < 8; ++d0) {			
		gfx_p[0] = tilep[0];
		gfx_p[1] = tilep[1];
		gfx_p += 0x10;
		tilep += 0x10;
	}
	tilep = realign_scr3b(gs, &gfx_p);

	d0 = 0x15 - d2 - 9;
	if (d0 < 8) {
		for (; d0>0; d0--) {			
			gfx_p[0] = tilep[0];
			gfx_p[1] = tilep[1];
			gfx_p += 0x10;
			tilep += 0x10;
		}	
	} else {
		d2 = d0;
		for (d0 = 0; d0 < 8; ++d0) {
			gfx_p[0] = tilep[0];
			gfx_p[1] = tilep[1];
			gfx_p += 0x10;
			tilep += 0x10;
		}
		
		tilep = realign_scr3b(gs, &gfx_p);
		for (d0 = d2 - 8; d0 >= 0; --d0) {			
			gfx_p[0] = tilep[0];
			gfx_p[1] = tilep[1];
			gfx_p += 0x10;
			tilep += 0x10;
		}	
	}
}

static void gstate_update_scroll1 (ScrollState *gs) {			//83498
    CP cp;
		
    g.CPS.Scroll1X = gs->XPI;
    g.CPS.Scroll1Y = gs->YPI;
    
    if((gs->XPI & 0x20) ^ gs->x001e) {		// does it need refilling?
		gs->x001e ^= 0x20;
    
		cp = _GSCoordOffsetScr1(gs, gs->x0024);
		_GSDrawScroll1B(gs, _GSCoordsScroll1(cp), _GSLookupScroll1(gs, cp), cp);
	}
}

static void gstate_update_scroll2 (ScrollState *gs) {
    short temp;
    CP cp;

    g.CPS.Scroll2X = gs->XPI;
    g.CPS.Scroll2Y = gs->YPI;
    	
    temp = gs->XPI & 0x10;
    temp ^= gs->x001e;
    if (temp == 0) {
        gs->x001e ^= 0x10;
    }
    
    cp = _GSCoordOffsetScr2(gs, gs->x0024);
    _GSDrawScroll2A(gs, _GSCoordsScroll2(cp), _GSCoordsScroll2(cp),cp);
    
    temp  = gs->YPI & 0x10;
    temp ^= gs->x001f;
    if(temp == 0) {
        gs->x001f ^= 0x10;
        
    }
    
    cp = _GSCoordOffsetScr2(gs, gs->x0024);
    _GSDrawScroll2C(gs, _GSCoordsScroll2(cp), _GSCoordsScroll2(cp), cp);     /* seems to only be used on attract building */
}

static void gstate_update_scroll3 (ScrollState *gs) {		//83d06
    short temp;
    CP cp;

    g.CPS.Scroll3X = gs->X.part.integer;
    g.CPS.Scroll3Y = gs->Y.part.integer;
    
    temp  = gs->XPI & 0x20;
    temp ^= gs->x001e;
    if(temp == 0) {
        gs->x001e ^= 0x20;
		cp = _GSCoordOffsetScr3(gs, gs->x0024);
		_GSDrawScroll3A(gs, _GSCoordsScroll3(cp), _GSLookupScroll3(gs, cp), cp);
    }
    	
    
    temp  = gs->YPI & 0x20;
    temp ^= gs->x001f;
    if(temp == 0) {
        gs->x001f ^= 0x20;
		cp = _GSCoordOffsetScr3(gs, gs->x0024);
		// XXX _GSDrawScroll3B(gs, _GSCoordsScroll3(cp), _GSLookupScroll3(gs, cp),cp);
    }
}

#pragma mark Linescroll

#define LINESCROLL_SET_DEC(count)				\
for (i=0; i<(count); ++i) {						\
*cursor++ = AccumOffset.part.integer;			\
AccumOffset.full -= Offset;					\
}												\

#define LINESCROLL_SET_INC_BACK(count)			\
for (i=0; i<(count); ++i) {						\
AccumOffset.full += Offset;					\
*--cursor = AccumOffset.part.integer;			\
}												\

#define LINESCROLL_SET_DEC_BACK(count)			\
for (i=0; i<(count); ++i) {						\
AccumOffset.full -= Offset;					\
*--cursor = AccumOffset.part.integer;			\
}												\

#define LINESCROLL_SET(count)					\
for (i=0; i<(count); ++i) {						\
*cursor++ = AccumOffset.part.integer;			\
}												\

#define LINESCROLL_SET_BACK(count)				\
for (i=0; i<(count); ++i) {						\
*--cursor = AccumOffset.part.integer;			\
}												\

#define LINESCROLL_SET_BACK_CONST(count,const)	\
for (i=0; i<(count); ++i) {						\
*--cursor = const;								\
}												\

#define LINESCROLL_INC(count);					\
for	(i=0; i<(count); ++i) {						\
AccumOffset.full += Offset;					\
}												\

#define GSTATE_PIXEL    (1 << 16)             // fixed precision 1.0


/*!
 %a0: pointer to linescroll array
 %a1: pointer to position_x
 */
static void _GSUpdateRowScroll(RowScrollState *ss, short *a0, short *a1) { /* 84592 */
	int Offset;					// %d0
	int i;
	FIXED16_16 AccumOffset;		// %d1
    short *cursor;              // %a2
    short *a3;          // %a3
	int d3;             // %d3 type should be u32
    
    Offset = ss->ss_0010 * (ss->CenterX - a1[0]);
    cursor = a0 + ((u32)ss->GroundRow / 2);
    AccumOffset.full = (SCREEN_WIDTH / 2) * GSTATE_PIXEL;
#ifdef REDHAMMER
    ss->RowOffset = Offset;
#endif
    
	switch (g.CurrentStage) {
		case STAGE_JAPAN_RYU:
			LINESCROLL_SET_DEC(12);
			LINESCROLL_SET(12);

			a3 = cursor;
			cursor = a0 + ((u32)ss->GroundRow / 2);
			AccumOffset.full = (SCREEN_WIDTH / 2) * GSTATE_PIXEL;

			LINESCROLL_SET_INC_BACK(24);
			LINESCROLL_INC(16);
			d3 = AccumOffset.full;
			for (i=0; i<16; ++i) {
				d3 += Offset;
			}
			ss->Scroll3Parallax.full = d3;
			for (i=0; i<8; ++i) {
				d3 += Offset;
			}
			*a3 = d3;                   // a3 is the lowermost row, 256
			for (i=0; i<8; ++i) {
				d3 += Offset;
			}
			ss->Scroll1Parallax.full = d3;
			LINESCROLL_SET_BACK(208);
			break;
		case STAGE_JAPAN_EHONDA:
			LINESCROLL_SET_DEC(24);
			cursor = a0 + ((u32)ss->GroundRow / 2);
			AccumOffset.full = (SCREEN_WIDTH / 2) * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(24);
			ss->Scroll1Parallax.part.integer = AccumOffset.part.integer;
			LINESCROLL_SET_INC_BACK(16);
			LINESCROLL_SET_BACK(16);
            
			LINESCROLL_SET_BACK_CONST(5, ss->Scroll1Parallax.part.integer);     // front of bath
            
			ss->Scroll3Parallax.full = AccumOffset.full;
            
			AccumOffset.full = ss->Scroll1Parallax.full;
			LINESCROLL_SET_INC_BACK(15);                // top of bath
			AccumOffset.full = ss->Scroll3Parallax.full;
			LINESCROLL_SET_BACK(92);
			LINESCROLL_SET_INC_BACK(64);
			break;
		case STAGE_BRAZIL_BLANKA:
			LINESCROLL_SET_DEC(24);
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = (SCREEN_WIDTH / 2) * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(24);
			ss->Scroll3Parallax.full = AccumOffset.full;
			LINESCROLL_INC(16);
			ss->Scroll1Parallax.full = AccumOffset.full;
			LINESCROLL_SET_BACK(208);
			break;
		case STAGE_USA_GUILE:
			LINESCROLL_SET_DEC(24);
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = (SCREEN_WIDTH / 2) * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(8);
			ss->Scroll3Parallax.full = AccumOffset.full;
			LINESCROLL_SET_INC_BACK(18);
			d3 = AccumOffset.full;
			for (i=0; i<16; ++i) {
				d3 += Offset;
			}
			ss->Scroll1Parallax.full = d3;
			LINESCROLL_SET_BACK(126);
			LINESCROLL_SET_DEC_BACK(80);
			break;
		case STAGE_USA_KEN:
			LINESCROLL_SET_DEC(16);
			ss->Scroll1Parallax.full = AccumOffset.full;
			LINESCROLL_SET(8);
			
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = (SCREEN_WIDTH / 2) * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(40);
			ss->Scroll3Parallax.full = AccumOffset.full;
			LINESCROLL_INC(16);
			LINESCROLL_SET_BACK(192);
			break;
		case STAGE_CHINA_CHUNLI:
			LINESCROLL_SET_DEC(24);
			ss->Scroll1Parallax.full = AccumOffset.full;
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = (SCREEN_WIDTH / 2) * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(40);
			ss->Scroll3Parallax.full = AccumOffset.full;
			LINESCROLL_SET_BACK(192);
			break;
		case STAGE_USSR_ZANGIEF:
			LINESCROLL_SET_DEC(24);
			ss->Scroll1Parallax.full = AccumOffset.full;
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = 192 * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(34);
			d3 = AccumOffset.full;
			for (i=0; i<16; ++i) {
				d3 += Offset;
			}
			ss->Scroll3Parallax.full = d3;
			LINESCROLL_SET_BACK(192);
			break;
		case STAGE_INDIA_DHALSIM:
#ifdef CPS
			AccumOffset.full = 192 * GSTATE_PIXEL;
			LINESCROLL_SET_DEC(24);
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = 192 * GSTATE_PIXEL;
            ss->ss_0014.full = AccumOffset.full;
			LINESCROLL_SET_INC_BACK(24);
			ss->ss_0018.full = AccumOffset.full;
			LINESCROLL_SET_INC_BACK(16);
			LINESCROLL_SET_BACK(192);
#else
            ss->Scroll1Parallax.full = 192 * GSTATE_PIXEL;
            ss->Scroll3Parallax.full = 192 * GSTATE_PIXEL + (24 * Offset);
#endif
            break;
		case STAGE_THAILAND_BISON:
			LINESCROLL_SET_DEC(24);
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = 192 * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(8);
			ss->Scroll1Parallax.full = AccumOffset.full;
			LINESCROLL_SET_INC_BACK(16);
			ss->Scroll3Parallax.full = AccumOffset.full;
			LINESCROLL_SET_INC_BACK(16);
			d3 = AccumOffset.full;
			for (i=0; i<32; ++i) {
				d3 += Offset;
			}
			LINESCROLL_SET_BACK(128);
			AccumOffset.full = d3;
			LINESCROLL_SET_DEC_BACK(64);
			break;
		case STAGE_THAILAND_SAGAT:
			LINESCROLL_SET_DEC(24);
			ss->Scroll3Parallax.full = AccumOffset.full;
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = 192 * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(24);
			d3 = AccumOffset.full;
			for (i=0; i<16; ++i) {
				d3 += Offset;
			}
			ss->Scroll1Parallax.full = d3;
			LINESCROLL_SET_BACK(208);
			break;
		case STAGE_USA_BALROG:
			LINESCROLL_SET_DEC(24);
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = 192 * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(13);
			LINESCROLL_SET_BACK(11);            // Light bulbs at edge of floor
			LINESCROLL_INC(20);
			ss->Scroll3Parallax.full = AccumOffset.full;
			LINESCROLL_INC(20);
			d3 = AccumOffset.full;
			for (i=0; i<36; ++i) {
				d3 += Offset;
			}
			ss->Scroll1Parallax.full = d3;
			LINESCROLL_SET_BACK(192);
			break;
		case STAGE_SPAIN_VEGA:
			LINESCROLL_SET_DEC(24);
			cursor = a0 + ((u32)ss->GroundRow / 2);			
			AccumOffset.full = 192 * GSTATE_PIXEL;
			LINESCROLL_SET_INC_BACK(4);
			ss->Scroll1Parallax.full = AccumOffset.full;
			LINESCROLL_SET_INC_BACK(12);
			LINESCROLL_SET_BACK(59);
			LINESCROLL_INC(16);
			ss->Scroll3Parallax.full = AccumOffset.full
			LINESCROLL_INC(16);
			LINESCROLL_SET_BACK(149);
			break;
		case 12:
		case 13:
		case 14:
		case 15:
			/* do nothing */
			break;
		FATALDEFAULT;
	}
//    printf("x %4d skew %4d floor %f wall %f\n",
//           *a1,
//           ss->ss_0010,
//           (float)ss->ss_0014.full / 0x10000,
//           (float)ss->ss_0018.full / 0x10000);
}

    
#pragma mark Publics

/**
 @brief Update the tilemaps
 @see sf2ua/0x8318a
 */
void TMUpdate(void) {
    debughook(4);
    if(g.OnBonusStage) {
		/* 831b2 */
        _GSMaintScroll2(&gstate_Scroll2);
        _GSMaintScroll1(&gstate_Scroll1);
        _GSMaintScroll3(&gstate_Scroll3);
    } else {
		/* 83196 */
        _GSMaintScroll2(&gstate_Scroll2);
        _GSMaintRowScroll(&gstate_RowScroll);  
        _GSMaintScroll1(&gstate_Scroll1);
        _GSMaintScroll3(&gstate_Scroll3);
    }
}

/**
 @brief Initialize the tilemaps for the current stage
 @see sf2ua/0x83716
 */
void TMInitForStage(void){			/* 83716 setup tilemaps & palettes from g.Pallete1 */
	_GSStageScroll1(g.Stage);	
	_GSStageScroll2(g.Stage);
	_GSStageScroll3(g.Stage);
}

/**
 @brief Move to the center of the stage arrangement
 @see sf2ua/0x83860
 */
void TMGotoCenter(void) {
    gstate_Scroll1.XPI = RH2DWord(0x83a00, 6, g.CurrentStage, 0);
    gstate_Scroll1.YPI = RH2DWord(0x83a00, 6, g.CurrentStage, 1);
    gstate_Scroll2.XPI = RH2DWord(0x83a00, 6, g.CurrentStage, 2);
    gstate_Scroll2.YPI = RH2DWord(0x83a00, 6, g.CurrentStage, 3);
    gstate_Scroll3.XPI = RH2DWord(0x83a00, 6, g.CurrentStage, 4);
    gstate_Scroll3.YPI = RH2DWord(0x83a00, 6, g.CurrentStage, 5);
}

/**
 @brief Parallax is handled by some other mechanism, just update scrolls 2 & 3
 */
void TMUpdateScrollsAttractSequence(void) {
    gstate_update_scroll2(&gstate_Scroll2);
    gstate_update_scroll3(&gstate_Scroll3);
}

void TMSetupScroll2(ScrollState *gs) {			// 83c3c
    CP			cp;
	int			i;
	
	g.CPS.Scroll2X = gs->XPI;
	g.CPS.Scroll2Y = gs->YPI;
	
    cp.x = gs->XPI -  144;
    cp.y = ~(gs->YPI + 384);
	
	for (i=0x29; i >= 0; --i) {			// 768 pixels
		_GSDrawScroll2A(gs, _GSCoordsScroll2(cp), _GSLookupScroll2(gs, cp), cp);
		cp.x += 16;
	}
}
void TMSetupScroll3(ScrollState *gs) {			// 83cd2 was setup_scroll3
    CP			cp;
	int			i;
	
	g.CPS.Scroll3X = gs->XPI;
	g.CPS.Scroll3Y = gs->YPI;
	
    cp.x = gs->XPI -  160;
    cp.y = ~(gs->YPI + 384);
	
	for (i=0x15; i >= 0; --i) {			// 0x16 x 32 = 704 pixels
		_GSDrawScroll3A(gs, _GSCoordsScroll3(cp), _GSLookupScroll3(gs, cp), cp);
		cp.x += 32;
	}
}
