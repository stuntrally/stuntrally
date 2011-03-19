#ifndef _MODEL_JOE03_H
#define _MODEL_JOE03_H

//#include <string>
//#include <ostream>
#include "joepack.h"
#include "model.h"

// This holds the header information that is read in at the beginning of the file
struct JOEHeader
{ 
	int magic;                   // This is used to identify the file
	int version;                 // The version number of the file
	int num_faces;	            // The number of faces (polygons)
	int num_frames;               // The number of animation frames
};

// This is used to store the vertices that are read in for the current frame
struct JOEVertex
{
	float vertex[3];
};

// This stores the indices into the vertex and texture coordinate arrays
struct JOEFace
{
	short vertexIndex[3];
	short normalIndex[3];
	short textureIndex[3];
};

// This stores UV coordinates
struct JOETexCoord
{
	float u, v;
};

// This stores the frames vertices after they have been transformed
struct JOEFrame
{
	int num_verts;
	int num_texcoords;
	int num_normals;
	
	JOEFace * faces;
	JOEVertex * verts;
	JOEVertex * normals;
	JOETexCoord * texcoords;
};

// This holds all the information for our model/scene. 
struct JOEObject 
{
	JOEHeader info;
	JOEFrame * frames;
};

// This class handles all of the loading code
class MODEL_JOE03 : public MODEL
{
public:
	MODEL_JOE03() : JOE_MAX_FACES(32000),JOE_VERSION(3),MODEL_SCALE(1.0) {}
	virtual ~MODEL_JOE03() {Clear();}

	virtual bool Load(const std::string & strFileName, std::ostream & error_output) {return Load(strFileName, NULL, error_output);}
	virtual bool CanSave() const {return false;}
	bool Load(std::string strFileName, JOEPACK * pack, std::ostream & error_output);
	bool LoadFromHandle(FILE * f, JOEPACK * pack, std::ostream & error_output);

private:
	
	class VERT_ENTRY
	{
		public:
			VERT_ENTRY() : original_index(-1) {}
			int original_index;
			int norm_index;
			int tex_index;
	};
	
	const int JOE_MAX_FACES;
	const int JOE_VERSION;
	const float MODEL_SCALE;
	
	std::string modelpath;
	std::string modelname;
	
	int BinaryRead(void * buffer, unsigned int size, unsigned int count, FILE * f, JOEPACK * pack);
	
	void CorrectEndian(struct JOEFace * p, int num);
	void CorrectEndian(struct JOEVertex *p, int num);
	void CorrectEndian(struct JOETexCoord *p, int num);

	void ReadData(FILE *m_FilePointer, JOEPACK * pack, JOEObject * pObject); // This reads in the data from the MD2 file and stores it in the member variable
	
	bool NeedsNormalSwap(JOEObject * pObject);
};

#endif
