#include "Com_quake.h"


#if 0
/*
==============================================================================

  .BSP definitions

==============================================================================
*/


typedef struct                 // A Directory entry
{ long  offset;                // Offset to entry, in bytes, from start of file
  long  size;                  // Size of entry in file, in bytes
} dentry_t;


typedef struct                 // The BSP file header
{ long  version;               // Model version, must be 0x17 (23).
  dentry_t entities;           // List of Entities.
  dentry_t planes;             // Map Planes.
                               // numplanes = size/sizeof(plane_t)
  dentry_t miptex;             // Wall Textures.
  dentry_t vertices;           // Map Vertices.
                               // numvertices = size/sizeof(vertex_t)
  dentry_t visilist;           // Leaves Visibility lists.
  dentry_t nodes;              // BSP Nodes.
                               // numnodes = size/sizeof(node_t)
  dentry_t surfaces;           // Map Surfaces.
                               // numsurfaces = size/sizeof(surface_t)
  dentry_t lightmaps;          // Wall Light Maps.
  dentry_t boundnodes;         // BSP bound nodes, for Hulls.
                               // numbounds = size/sizeof(hullbound_t)
  dentry_t leaves;             // BSP Leaves.
                               // numlaves = size/sizeof(leaf_t)
  dentry_t lstsurf;            // List of Surfaces.
  dentry_t edges;              // Original surface Edges.
                               // numedges = Size/sizeof(edge_t)
  dentry_t lstedges;           // List of surfaces Edges.
  dentry_t hulls;              // List of Hulls.
                               // numhulls = Size/sizeof(hull_t)
} dheader_t;



typedef struct                 // Mip Texture
{ char name[16];               // Name of the texture.
  u_long width;                // width of picture, must be a multiple of 8
  u_long height;               // height of picture, must be a multiple of 8
  u_long offset1;              // offset to u_char Pix[width   * height]
  u_long offset2;              // offset to u_char Pix[width/2 * height/2]
  u_long offset4;              // offset to u_char Pix[width/4 * height/4]
  u_long offset8;              // offset to u_char Pix[width/8 * height/8]
} miptex_t;


/*
==========================================

==========================================
*/
bool LoadQuakeBSPTextures(CFile *file)
{
	if(!file)
	{
		dprintf("LoadQuakeBSPTextures:bad file passed\n");
		return false;
	}
	
	FILE * fp = file->GetFp();
	if(!fp)
	{
		dprintf("LoadQuakeBSPTextures:Couldnt get valid file pointer for :%s\n",file->filename);
		return false;
	}

	
    dheader_t bsp_header;
	long numtex;			//number of textures in the bsp file
    char tex_name[32];
	long mip_offset[256];
	u_long width;                // width of picture
    u_long height;               // height of picture
    u_long offset1;              // offset to u_char Pix[width   * height]
	CImageData	image;
	char outpath[256];
	char outfile[256];
	int  count = 0;

    fread(&bsp_header, sizeof(dheader_t), 1, fp);		//read header

	if(bsp_header.version != 29)
	{
		dprintf("Not a Quake1 bsp: %s\n", file);
		file->Close();
		return false;
	}

    fseek(fp, bsp_header.miptex.offset , SEEK_SET);		//seek to mip offsets
    fread(&numtex, sizeof(long), 1, fp);				//get num of textures
    
	if (numtex > 256)
       numtex = 256;

	//read all the offsets
    for (count=0; count<numtex; count++) 
	{    fread(&mip_offset[count], sizeof(long), 1, fp);
		 //fread(mip_offset+count, sizeof(long), 1, fp);
    }

	if(numtex)
	{
		memset(outpath,0,256);
		strcpy(outpath,file->filename);
		int len = strlen(file->filename) - 4;  //get rid of extension
		strcpy(outpath+len,"\\");
	}

	long headeroff=0;
	long dataoff=0;

	// read in the mips
    for (count=0; count<numtex; count++) 
	{
		headeroff= mip_offset[count] + bsp_header.miptex.offset;
		
		image.Reset();
		memset(tex_name,0,32);
		
		fseek(fp, headeroff , SEEK_SET);

		fread(tex_name, sizeof(char), 16, fp);
        fread(&width, sizeof(long), 1, fp);
        fread(&height, sizeof(long), 1, fp);
        fread(&offset1, sizeof(long), 1, fp);

		image.height = height;
		image.width = width;
		image.data = new unsigned char[width*height];

		dataoff = headeroff + offset1;

		//Seek to where mip1 starts and read it
		fseek(fp,dataoff,SEEK_SET);
		fread(image.data,width,height,fp);

		strcpy(outfile,"C:\\Void\\Stuff\\");
		strcat(outfile,outpath);
/*
BOOL CreateDirectory(
  LPCTSTR lpPathName,                         // pointer to directory path string
  LPSECURITY_ATTRIBUTES lpSecurityAttributes  // pointer to security descriptor
);
 */
		if(_mkdir(outfile) && errno != EEXIST)
		{
			dprintf("Couldnt create directory %s\n",outfile);
			continue;
		}

		strcat(outfile,tex_name);

		//Send it off for conversion
		dprintf("** Converting %s in %s, %d of %d\n",tex_name,file->filename,count+1,numtex);
		WriteTGAFromQ1PCX(outfile,&image);
	}
	file->Close();
    return true;
}
#endif