#include "../Exe/Source/Sys_cons.h"
#include "Com_mem.h"

enum EHunkFlags
{
	E_MEMINUSE = 1
};

typedef struct
{
    uint  flags;	//flags
	char* name;		//name
    uint* basePtr;	//base pointer
	uint  curSize;  //size
    uint  maxSize;  //max size
}Hunk_t;

static Hunk_t 	hunkPool[COM_MAXHUNKS];
static uint		usedHunks;


/*
==========================================
Util funcs to print hunk stats
==========================================
*/
void CMemPrintHunk(int argc, char ** argv)
{
	if(argc == 1)
	{
		ComPrintf("=== HUNK STATS ===\n");
		for(int i=0;i<COM_MAXHUNKS;i++)
		{
			if((hunkPool[i].flags & E_MEMINUSE) &&
			   (hunkPool[i].basePtr) &&
			   (hunkPool[i].name))
			{
				ComPrintf("%s : size %d : used %d\n",
					hunkPool[i].name, hunkPool[i].maxSize, hunkPool[i].curSize);
			}
		}
	}
}

/*
==========================================
Constructor/Destructor
==========================================
*/

CMemManager::CMemManager(int defhunksize)
{
	memset(hunkPool,0,sizeof(Hunk_t)*COM_MAXHUNKS);
	usedHunks = 0;
	CreateHunk("default",defhunksize);
}

CMemManager::~CMemManager()
{	
	for(int i=0;i<COM_MAXHUNKS;i++)
	{
		if((hunkPool[i].flags & E_MEMINUSE) ||
			 (hunkPool[i].basePtr))
				::VirtualFree(hunkPool[i].basePtr, 0, MEM_RELEASE);
		if(hunkPool[i].name)
			delete [] hunkPool[i].name;
	}
	memset(hunkPool,0,sizeof(Hunk_t)*COM_MAXHUNKS);
	usedHunks = 0;
}


/*
==========================================
Init func
==========================================
*/
bool CMemManager::Init()
{
//	if(g_pCons)
//		g_pCons->RegisterCFunc("mem_print",&CMemPrintHunk);
	return true;
}


/*
==========================================
Create Hunk
==========================================
*/
uint CMemManager::CreateHunk(const char * name, uint size)
{
	if(usedHunks >= COM_MAXHUNKS)
		return -1;

	if(!size)
		size = COM_DEFAULTHUNKSIZE;
	
	for(int i=0;i<COM_MAXHUNKS;i++)
	{
		//found a free hunk
		if(!(hunkPool[i].flags & E_MEMINUSE))
		{
			//Try to reserve a block of memory of the given size
			hunkPool[i].basePtr = (uint*)VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
			
			//Failed to reserve it. Error !
			if(hunkPool[i].basePtr == NULL)
			{
/*
				if(g_pCons)
					ComPrintf("CMemManager::CreateHunk: Unable to reserve <%d> bytes for <%s>\n",
										size, name);
*/
				return -1;
			}

			//Fill other info then return id
            hunkPool[i].flags = E_MEMINUSE;
            hunkPool[i].curSize = 0;
            hunkPool[i].maxSize = size;
			hunkPool[i].name = new char[strlen(name)+1];
			strcpy(hunkPool[i].name, name);

			usedHunks ++;
			return i;
		}
	}
	return -1;
}

/*
==========================================
Alloc from the given hunk
consider it as the default hunk if no id is specified
==========================================
*/
void* CMemManager::AllocHunk(uint size,uint id)
{
	if(id >= COM_MAXHUNKS)
		return 0;

	if(!(hunkPool[id].flags & E_MEMINUSE) ||
		(hunkPool[id].basePtr == 0))
	{
//		if(g_pCons)
//			ComPrintf("CMemManager::AllocHunk: Hunk is unreserved, can't alloc\n");
		return 0;
	}

	if(hunkPool[id].curSize + size >= hunkPool[id].maxSize)
	{
//		if(g_pCons)
//			ComPrintf("CMemManager::AllocHunk: Unable to alloc <%d> bytes for <%s>\n",
//								size, hunkPool[id].name);
		return 0;
	}

	//Alloc it now
	void * buf = VirtualAlloc(hunkPool[id].basePtr, size, MEM_COMMIT, PAGE_READWRITE);
	if(buf != 0)
		hunkPool[id].curSize +=  size;
	return buf;
}

/*
==========================================
Free the given Hunk entirely
==========================================
*/
void CMemManager::FreeHunk(uint id)
{
	if(id >= COM_MAXHUNKS)
		return;

	if(!(hunkPool[id].flags & E_MEMINUSE) ||
		(hunkPool[id].basePtr == 0))
	{
//		if(g_pCons)
//			ComPrintf("CMemManager::AllocHunk: Hunk is empty, can't free\n");
		return;
	}

	::VirtualFree(hunkPool[id].basePtr, 0, MEM_RELEASE);
	hunkPool[id].basePtr = 0;
	hunkPool[id].curSize = 0;
	hunkPool[id].maxSize = 0;
	hunkPool[id].flags = 0;
	delete [] hunkPool[id].name;
	usedHunks --;
}


/*
==========================================
Reset the given Hunk
==========================================
*/
void CMemManager::ResetHunk(uint id)
{
	if(id >= COM_MAXHUNKS)
		return;
	hunkPool[id].curSize = 0;
}


/*
==========================================
Get Hunks Max Size
==========================================
*/
uint CMemManager::GetHunkMaxSize(uint id)
{
	if(id >= COM_MAXHUNKS)
		return 0;
	return hunkPool[id].maxSize;
}

/*
==========================================
Get Hunks Max Size
==========================================
*/
uint CMemManager::GetHunkCurSize(uint id)
{
	if(id >= COM_MAXHUNKS)
		return 0;
	return hunkPool[id].curSize;
}

/*
==========================================
Get Hunks Base Address
==========================================
*/
void* CMemManager::GetHunkAddr(uint id)
{
	if(id >= COM_MAXHUNKS)
		return 0;
	return hunkPool[id].basePtr;
}


/*
==========================================
Get Hunks Name 
==========================================
*/
const char * CMemManager::GetHunkName(uint id)
{
	if(id >= COM_MAXHUNKS)
		return 0;
	return hunkPool[id].name;
}


/*
==========================================
Get Hunk id by name
==========================================
*/
uint CMemManager::GetHunkByName(const char * name)
{
	if(!name)
		return -1;
	for(int i=0;i<COM_MAXHUNKS;i++)
	{
		if(!strcmp(hunkPool[i].name,name))
			return i;
	}
	return -1;
}






#if 0

/*
==============================================================================

						ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/

typedef struct memblock_s
{
	int		size;           // including the header and possibly tiny fragments
	int     tag;            // a tag of 0 is a free block
	int     id;        		// should be ZONEID
	struct	memblock_s       *next, *prev;
	int		pad;			// pad to 64 bit boundary
}memblock_t;

typedef struct
{
	int			size;		// total bytes malloced, including header
	memblock_t	blocklist;	// start / end cap for linked list
	memblock_t	*rover;
}memzone_t;


memzone_t	*mainzone;

/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone (memzone_t *zone, int size)
{
	memblock_t	*block;
	
	// set the entire zone to one free block

	zone->blocklist.next = 	zone->blocklist.prev = 	block = 
	(memblock_t *)( (byte *)zone + sizeof(memzone_t) );

	zone->blocklist.tag = 1;	// in use block
	zone->blocklist.id = 0;
	zone->blocklist.size = 0;
	zone->rover = block;
	
	block->prev = block->next = &zone->blocklist;
	block->tag = 0;			// free block
	block->id = ZONEID;
	block->size = size - sizeof(memzone_t);
}


/*
========================
Z_Free
========================
*/
void Z_Free (void *ptr)
{
	memblock_t	*block, *other;
	
	if (!ptr)
	{
//		Sys_Error ("Z_Free: NULL pointer");
	}

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
	{
//		Sys_Error ("Z_Free: freed a pointer without ZONEID");
	}
	if (block->tag == 0)
	{
//		Sys_Error ("Z_Free: freed a freed pointer");
	}

	block->tag = 0;		// mark as free
	
	other = block->prev;
	if (!other->tag)
	{	
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == mainzone->rover)
			mainzone->rover = other;
		block = other;
	}
	
	other = block->next;
	if (!other->tag)
	{	// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		if (other == mainzone->rover)
			mainzone->rover = block;
	}
}


/*
========================
Z_Malloc
========================
*/
void *Z_Malloc (int size)
{
	void	*buf;
	
Z_CheckHeap ();	// DEBUG
	buf = Z_TagMalloc (size, 1);
	if (!buf)
		Sys_Error ("Z_Malloc: failed on allocation of %i bytes",size);
	Q_memset (buf, 0, size);

	return buf;
}

void *Z_TagMalloc (int size, int tag)
{
	int		extra;
	memblock_t	*start, *rover, *new, *base;

	if (!tag)
		Sys_Error ("Z_TagMalloc: tried to use a 0 tag");

//
// scan through the block list looking for the first free block
// of sufficient size
//
	size += sizeof(memblock_t);	// account for size of block header
	size += 4;					// space for memory trash tester
	size = (size + 7) & ~7;		// align to 8-byte boundary
	
	base = rover = mainzone->rover;
	start = base->prev;
	
	do
	{
		if (rover == start)	// scaned all the way around the list
			return NULL;
		if (rover->tag)
			base = rover = rover->next;
		else
			rover = rover->next;
	} while (base->tag || base->size < size);
	
//
// found a block big enough
//
	extra = base->size - size;
	if (extra >  MINFRAGMENT)
	{	// there will be a free fragment after the allocated block
		new = (memblock_t *) ((byte *)base + size );
		new->size = extra;
		new->tag = 0;			// free block
		new->prev = base;
		new->id = ZONEID;
		new->next = base->next;
		new->next->prev = new;
		base->next = new;
		base->size = size;
	}
	
	base->tag = tag;				// no longer a free block
	
	mainzone->rover = base->next;	// next allocation will start looking here
	
	base->id = ZONEID;

// marker for memory trash testing
	*(int *)((byte *)base + base->size - 4) = ZONEID;

	return (void *) ((byte *)base + sizeof(memblock_t));
}


/*
========================
Z_Print
========================
*/
void Z_Print (memzone_t *zone)
{
	memblock_t	*block;
	
	Con_Printf ("zone size: %i  location: %p\n",mainzone->size,mainzone);
	
	for (block = zone->blocklist.next ; ; block = block->next)
	{
		Con_Printf ("block:%p    size:%7i    tag:%3i\n",
			block, block->size, block->tag);
		
		if (block->next == &zone->blocklist)
			break;			// all blocks have been hit	
		if ( (byte *)block + block->size != (byte *)block->next)
			Con_Printf ("ERROR: block size does not touch the next block\n");
		if ( block->next->prev != block)
			Con_Printf ("ERROR: next block doesn't have proper back link\n");
		if (!block->tag && !block->next->tag)
			Con_Printf ("ERROR: two consecutive free blocks\n");
	}
}


/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap (void)
{
	memblock_t	*block;
	
	for (block = mainzone->blocklist.next ; ; block = block->next)
	{
		if (block->next == &mainzone->blocklist)
			break;			// all blocks have been hit	
		if ( (byte *)block + block->size != (byte *)block->next)
			Sys_Error ("Z_CheckHeap: block size does not touch the next block\n");
		if ( block->next->prev != block)
			Sys_Error ("Z_CheckHeap: next block doesn't have proper back link\n");
		if (!block->tag && !block->next->tag)
			Sys_Error ("Z_CheckHeap: two consecutive free blocks\n");
	}
}






#endif
