#ifndef VOID_GAME_ANIM_DEFS
#define VOID_GAME_ANIM_DEFS

enum
{
	STAND_BEGIN = 0,
	STAND_END = 39,
	
	RUN_BEGIN = 40,
	RUN_END = 45,

	ATTACK_BEGIN = 46,
	ATTACK_END = 53,

	PAIN1_BEGIN = 54,
	PAIN1_END = 57,

	PAIN2_BEGIN = 58,
	PAIN2_END = 61,

	PAIN2_BEGIN = 62,
	PAIN2_END = 65,

	JUMP_BEGIN = 66,
	JUMP_END = 71,

	FLIP_BEGIN = 72,
	FLIP_END = 83,

	SALUTE_BEGIN = 84,
	SALUTE_END = 94,

	TAUNT_BEGIN = 95,
	TAUNT_END = 111,

	WAVE_BEGIN = 112,
	WAVE_END = 122,

	POINT_BEGIN = 123,
	POINT_END = 134,

	CROUCH_STAND_BEGIN = 135,
	CROUCH_STAND_END = 153,

	CROUCH_WALK_BEGIN = 154,
	CROUCH_WALK_END = 159,

	CROUCH_ATTACK_BEGIN = 160,
	CROUCH_ATTACK_END = 168,

	CROUCH_PAIN_BEGIN = 169,
	CROUCH_PAIN_END = 172,

	CROUCH_DEATH_BEGIN = 173,
	CROUCH_DEATH_END = 177,

	DEATH1_BEGIN = 178,
	DEATH1_END = 183,

	DEATH2_BEGIN = 184,
	DEATH2_END = 189,

	DEATH3_BEGIN = 190,
	DEATH3_END = 197,
};

/*
================================================
Store preset animation frames and sequences.
Can attach a sequence to the 
================================================
*/

struct AnimSeq
{
	AnimSeq(int begin, int end) : frameBegin(begin), frameEnd(end) {}
	AnimSeq(const AnimSeq &anim) : frameBegin(anim.frameBegin) , frameEnd(anim.frameEnd) {}
	AnimSeq & operator = (const AnimSeq &anim)
	{
		frameBegin = anim.frameBegin;
		frameEnd = anim.frameEnd;
		return *this;
	}

	int frameBegin;
	int frameEnd;
};

#endif