

/*
================================================
implementation of .map file reading/parsing

	*** this is very carmack influenced,	***
	***		but 100% original				***

================================================
*/

#include <memory.h>
#include <string.h>

#include "std_lib.h"
#include "map_file.h"
#include "Com_Vector.h"
#include "com_trace.h"

// required for filesystem
#include "Com_defs.h"
#include "I_file.h"
#include "I_filesystem.h"


// shaders
#include "../Renderer/Rast_main.h"
#include "../Renderer/ShaderManager.h"


//=======================================================

int					num_map_entities;
map_entity_t		map_entities[MAX_MAP_ENTITIES];

int					num_map_brushes;
map_brush_t			map_brushes[MAX_MAP_BRUSHES];

int					num_map_brush_sides;
map_brush_side_t	map_brush_sides[MAX_MAP_BRUSH_SIDES];

int					num_map_texinfos;
map_texinfo_t		map_texinfos[MAX_MAP_TEXINFOS];

int					num_keys;
key_t				keys[MAX_MAP_KEYS];

extern	int					num_planes;
extern	plane_t				planes[MAX_MAP_PLANES];

//=======================================================



/*
==============
plane_test
==============
*/
#define PTEST_NORM_EPSILON	0.00001
#define PTEST_D_EPSILON		0.01
bool plane_test(plane_t &p1, plane_t &p2)
{
	if ((fabs(p1.d - p2.d) < PTEST_D_EPSILON) &&
		(fabs(p1.norm.x - p2.norm.x) < PTEST_NORM_EPSILON) &&
		(fabs(p1.norm.y - p2.norm.y) < PTEST_NORM_EPSILON) &&
		(fabs(p1.norm.z - p2.norm.z) < PTEST_NORM_EPSILON))
		return true;
	return false;
}


/*
=============
get_plane - like q2, # and #^1 are opposites
=============
*/
#define PLANE_NORM_SNAP 0.00001
int get_plane(plane_t plane)
{

	if ((1 - fabs(plane.norm.x)) < PLANE_NORM_SNAP)
	{
		if (plane.norm.x > 0)
			plane.norm.x = 1;
		else
			plane.norm.x = -1;

		plane.norm.y = 0;
		plane.norm.z = 0;
	}

	else if ((1 - fabs(plane.norm.y)) < PLANE_NORM_SNAP)
	{
		if (plane.norm.y > 0)
			plane.norm.y = 1;
		else
			plane.norm.y = -1;

		plane.norm.x = 0;
		plane.norm.z = 0;
	}

	else if ((1 - fabs(plane.norm.z)) < PLANE_NORM_SNAP)
	{
		if (plane.norm.z > 0)
			plane.norm.z = 1;
		else
			plane.norm.z = -1;

		plane.norm.x = 0;
		plane.norm.y = 0;
	}

	// do we already have a close one?
	for (int p=0; p<num_planes; p++)
	{
		if (plane_test(plane, planes[p]))
			return p;
	}

	// make a new one
	if (num_planes >= MAX_MAP_PLANES-1)
		Error("too many planes!");
	planes[num_planes] = plane;
	num_planes++;

	// make opposing one
	VectorScale(&plane.norm, -1, &planes[num_planes].norm);
	planes[num_planes].d = -plane.d;
	num_planes++;

	return p;
}


//=======================================================



/*
============
token system similar to what carmack uses
============
*/
#define MAX_LINE_LENGTH 2048
char	line[MAX_LINE_LENGTH];
char	token[MAX_LINE_LENGTH];
int		token_ptr;
int		line_num;
FILE	*map_file;

bool	get_token(bool cross);


/*
===========
new_line
===========
*/
bool new_line(void)
{
	memset(line, 0, MAX_LINE_LENGTH);
	if (fgets(line, MAX_LINE_LENGTH, map_file) == NULL)
		return false;

	if (line[MAX_LINE_LENGTH-2])
		Error("line number %d too long", line_num);

	token_ptr = 0;
	line_num++;
	return true;
}

/*
============
get_token
============
*/
bool get_token(bool cross)
{
	// skip any space
	while ((line[token_ptr] == ' ') &&
		   (token_ptr < MAX_LINE_LENGTH))
		   token_ptr++;

	if (!line[token_ptr] || (line[0] == '\n'))	// at the end of a line
	{
		if (!cross)
			Error("unexpected end of line - %d", line_num);

		if (!new_line())
			return false;

		// reset and start next line
		token_ptr = 0;
		return get_token(true);
	}

	// copy the token out
	int p = 0;
	bool cont = (line[token_ptr] && (line[token_ptr] != ' ')) ? true : false;
	while (cont) 
	{
		token[p++] = line[token_ptr++];

		// check for quotes
		if (token[0] == '\"')
		{
			if (!line[token_ptr])
				Error("unexpected end of line in quotes - %d", line_num);

			if (token[p-1] == '\"' && p!=1)
				cont = false;
		}

		else if (!line[token_ptr] || (line[token_ptr] == ' '))
				cont = false;
	}
	token[p] = '\0';


// take care of "//" comments
	if ((token[0] == '/') && (token[1] == '/'))
	{
		new_line();
		return get_token(true);		// get first token on next line whether or not your supposed to cross
	}


	return true;
}


/*
============
parse_key
============
*/
void parse_key(void)
{
	// have to remove the "'s
	memcpy(keys[num_keys].name, &token[1], strlen(token)-2);
	keys[num_keys].name[strlen(token)-2] = 0;

	if (!get_token(false))
		Error("parse_key: unexpected end of line - %d", line_num);

	memcpy(keys[num_keys].value, &token[1], strlen(token)-2);
	keys[num_keys].value[strlen(token)-2] = 0;

	map_entities[num_map_entities].num_keys++;
	num_keys++;
}


/*
============
parse_brush_side - return contents
============
*/
int parse_brush_side(void)
{
	if (num_map_brush_sides == MAX_MAP_BRUSH_SIDES)
		Error("too many brush sides");


	vector_t ppts[3];


	// read the plane info
	for (int i=0; i<3; i++)
	{
	// opening '('
		if (token[0] != '(')
			Error("parse_brush: unexpected \"(\" - %d", line_num);

	// read our 3 points
		get_token(false);
		ppts[i].x = (float)atof(token);

		get_token(false);
		ppts[i].y = (float)atof(token);

		get_token(false);
		ppts[i].z = (float)atof(token);




	// closing ')'
		get_token(false);
		if (token[0] != ')')
			Error("parse_brush_side: expected \")\" not found - %d", line_num);

	// get the next token
		get_token(false);
	}

	// build our plane
	plane_t p;
	VectorSub(ppts[2], ppts[0], ppts[2]);
	VectorSub(ppts[1], ppts[0], ppts[1]);
	_CrossProduct(&ppts[2], &ppts[1], &p.norm);
	VectorNormalize(&p.norm);
	p.d = dot(p.norm, ppts[0]);

	map_brush_sides[num_map_brush_sides].plane = get_plane(p);

	// read the texdef
	map_brush_sides[num_map_brush_sides].texinfo = num_map_texinfos;
	sprintf(map_texinfos[num_map_texinfos].name, "%s/%s", "textures", token);

	get_token(false);
	map_texinfos[num_map_texinfos].shift[0] = atoi(token);

	get_token(false);
	map_texinfos[num_map_texinfos].shift[1] = atoi(token);

	get_token(false);
	map_texinfos[num_map_texinfos].rotation = atoi(token);

	get_token(false);
	map_texinfos[num_map_texinfos].scale[0] = (float)atof(token);

	get_token(false);
	map_texinfos[num_map_texinfos].scale[1] = (float)atof(token);


	// find contents of this side
	get_token(false);
	get_token(false);
	get_token(false);

//	get the contents/surface flags from the shader
	int contents = g_pShaders->GetContentFlags(map_texinfos[num_map_texinfos].name);
	map_brush_sides[num_map_brush_sides].flags = g_pShaders->GetSurfaceFlags(map_texinfos[num_map_texinfos].name);;


	num_map_texinfos++;
	num_map_brush_sides++;
	map_brushes[num_map_brushes].num_sides++;

	return contents;
}


/*
============
parse_brush
============
*/
void parse_brush(void)
{
	if (num_map_brushes == MAX_MAP_BRUSHES)
		Error("too many brushes");

	if (!get_token(true))
		Error("parse_brush: unexpected EOF");

	map_brushes[num_map_brushes].first_side = num_map_brush_sides;
	map_brushes[num_map_brushes].num_sides = 0;
	map_brushes[num_map_brushes].contents = 0;

	while (token[0] == '(')
	{
		map_brushes[num_map_brushes].contents |= parse_brush_side();
		get_token(true);
	}
	
	if (map_brushes[num_map_brushes].contents == 0)
		map_brushes[num_map_brushes].contents = CONTENTS_SOLID;


	map_entities[num_map_entities].num_brushes++;
	num_map_brushes++;
}



/*
============
parse_entity
============
*/
bool parse_entity(void)
{
	if (!get_token(true))
		return false;

	if (num_map_entities == MAX_MAP_ENTITIES)
		Error("too many entities");

	if (!strcmp("{", token))
		Error("parse_entity: expected \"{\" on line %d", line_num);

	map_entities[num_map_entities].first_brush = num_map_brushes;
	map_entities[num_map_entities].num_brushes = 0;
	map_entities[num_map_entities].first_key   = num_keys;
	map_entities[num_map_entities].num_keys    = 0;

	do
	{
		if (!get_token(true))
			Error("parse_entity: unexpected end of file");

	// quotes always start a key
		if (token[0] == '\"')
			parse_key();

	// open brace starts a brush
		else if (token[0] == '{')
			parse_brush();

	// close brace ends the entity
		else if (token[0] == '}')
			break;

	} while (1);

	num_map_entities++;
	return true;
}


/*
============
load_map
============
*/
bool load_map(char *path)
{
	v_printf("loading map file %s\n", path);

	// load shaders
	g_pShaders = new CShaderManager();


	map_file = fopen(path, "r");
	if (!map_file)
		return false;

	num_map_entities	= 
	num_map_brushes		=
	num_map_brush_sides =
	num_map_texinfos	=
	num_keys			= 0;

	line_num = 0;
	new_line();

	while (parse_entity())
	{
	}

	v_printf("%5d entities\n", num_map_entities);
	v_printf("%5d brushes\n", num_map_brushes);
	v_printf("%5d brush sides\n", num_map_brush_sides);
	v_printf("%5d texinfos\n", num_map_texinfos);
	v_printf("%5d keys\n", num_keys);

/*
	for (int e=0; e<num_map_entities; e++)
	{
		v_printf("entitiy %d\n", e);

		v_printf("keys:\n");
		for (int k=map_entities[e].first_key; k<(map_entities[e].first_key + map_entities[e].num_keys); k++)
		{
			v_printf("\"%s\" \"%s\"\n", keys[k].name, keys[k].value);
		}

		v_printf("brushes:\n");
		for (int b=map_entities[e].first_brush; b<(map_entities[e].first_brush + map_entities[e].num_brushes); b++)
		{
			v_printf("brush %d\n", b-map_entities[e].first_brush);

			v_printf("sides:\n");
			for (int s=map_brushes[b].first_side; s<(map_brushes[b].num_sides+map_brushes[b].first_side); s++)
			{
				v_printf("side %d:  ", s);
				v_printf("%s, %d, %d, %d, %f, %f\n",
							map_texinfos[map_brush_sides[s].texinfo].name,
							map_texinfos[map_brush_sides[s].texinfo].shift[0],
							map_texinfos[map_brush_sides[s].texinfo].shift[1],
							map_texinfos[map_brush_sides[s].texinfo].rotation,
							map_texinfos[map_brush_sides[s].texinfo].scale[0],
							map_texinfos[map_brush_sides[s].texinfo].scale[1]);

				v_printf("norm - (%f,%f,%f) d=%f\n", map_brush_sides[s].plane.norm.x,
													map_brush_sides[s].plane.norm.y,
													map_brush_sides[s].plane.norm.z,
													map_brush_sides[s].plane.d);

			}

		}
	}
*/
	fclose(map_file);

	delete g_pShaders;
	return true;
}


/*
===========
get_worldspawn
===========
*/
int get_worldspawn(void)
{
	for (int e=0; e<num_map_entities; e++)
	{
		for (int k=map_entities[e].first_key; k<(map_entities[e].first_key+map_entities[e].num_keys); k++)
		{
			if (strcmp(keys[k].name, "classname"))
				continue;
			if (!strcmp(keys[k].value, "worldspawn"))
				return e;
		}
	}

	Error("world_spawn not found!");
	return -1;
}
