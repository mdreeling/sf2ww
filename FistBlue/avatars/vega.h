/*
 *  vega.h
 *  GLUTBasics
 *
 *  Created by Ben on 13/02/11.
 *  Copyright 2011 Ben Torkington. All rights reserved.
 *
 */

struct vegathrow {
	char	move;
	char	success;
};

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct UserData_Vega {
	u8	x0080;
	
	char	x0090;
	short	x0092;
	short	x0094;
	char	x0096;
} ;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
void PLCBCompAttackVega(Player *ply);
