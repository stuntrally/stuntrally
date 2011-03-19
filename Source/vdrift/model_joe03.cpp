#include "stdafx.h"

#include "model_joe03.h"

//#include <string>
using std::string;

//#include <memory>
using std::auto_ptr;

//#include <sstream>
using std::stringstream;

//#include <iostream>
using std::ostream;

//#include <vector>
using std::vector;

//#include <mathvector.h>

#include "endian_utility.h"

int MODEL_JOE03::BinaryRead ( void * buffer, unsigned int size, unsigned int count, FILE * f, JOEPACK * pack )
{
	unsigned int bytesread = 0;
	
	if ( pack == NULL )
	{
		bytesread = fread ( buffer, size, count, f );
	}
	else
	{
		bytesread = pack->Pack_fread ( buffer, size, count );
	}
	
	assert(bytesread == count);
	
	return bytesread;
}

bool MODEL_JOE03::Load ( string filename, JOEPACK * pack, ostream & err_output )
{
	modelpath = filename;

	Clear();

	FILE * m_FilePointer = NULL;

	//open file
	bool fileinpack = false;
	if ( pack == NULL )
	{
		m_FilePointer = fopen ( filename.c_str(), "rb" );
	}
	else
		fileinpack = pack->Pack_fopen ( filename );

	// Make sure we have a valid file pointer (we found the file)
	if ( ( pack == NULL && !m_FilePointer ) || ( pack != NULL && !fileinpack ) )
	{
		// Display an error message and don't load anything if no file was found
		// throw EXCEPTION ( __FILE__, __LINE__, "Unable to find file "+filename );
		
		//print an error message?
		return false;
	}

	bool val = LoadFromHandle ( m_FilePointer, pack, err_output );

	// Clean up after everything
	if ( pack == NULL )
		fclose ( m_FilePointer );
	else
		pack->Pack_fclose();

	return val;
}

bool MODEL_JOE03::LoadFromHandle ( FILE * m_FilePointer, JOEPACK * pack, std::ostream & err_output )
{
	//create a new object
	auto_ptr <JOEObject> pObject (new JOEObject);

	// Read the header data and store it in our variable
	BinaryRead ( &pObject->info, sizeof ( JOEHeader ), 1, m_FilePointer, pack );

	pObject->info.magic = ENDIAN_SWAP_32 ( pObject->info.magic );
	pObject->info.version = ENDIAN_SWAP_32 ( pObject->info.version );
	pObject->info.num_faces = ENDIAN_SWAP_32 ( pObject->info.num_faces );
	pObject->info.num_frames = ENDIAN_SWAP_32 ( pObject->info.num_frames );

	// Make sure the version is what we expect or else it's a bad egg
	if ( pObject->info.version != JOE_VERSION )
	{
		// Display an error message for bad file format, then stop loading
		stringstream err;
		err << "Invalid file format (Version is " << pObject->info.version << " not " << JOE_VERSION << ": " << modelpath;
		pObject.reset();
		//throw EXCEPTION ( __FILE__, __LINE__, err.str() );
		//print error?
		return false;
	}

	if ( pObject->info.num_faces > JOE_MAX_FACES )
	{
		stringstream err;
		err << modelpath << " has " << pObject->info.num_faces << " faces (max " << JOE_MAX_FACES << ").";
		pObject.reset();
		
		//throw EXCEPTION ( __FILE__, __LINE__, err.str() );
		//print error?
		return false;
	}

	// Read in the model data
	ReadData ( m_FilePointer, pack, pObject.get() );

	//generate metrics such as bounding box, etc
	GenerateMeshMetrics();
	
	//optimize into a static display list
	GenerateListID(err_output);

	// Return a success
	return true;
}

void MODEL_JOE03::ReadData ( FILE *m_FilePointer, JOEPACK * pack, JOEObject * pObject )
{
	int num_frames = pObject->info.num_frames;
	int num_faces = pObject->info.num_faces;

	pObject->frames = new JOEFrame [num_frames];

	for ( int i = 0; i < num_frames; i++ )
	{
		pObject->frames[i].faces = new JOEFace [num_faces];

		BinaryRead ( pObject->frames[i].faces, sizeof ( JOEFace ), num_faces, m_FilePointer, pack );
		CorrectEndian ( pObject->frames[i].faces, num_faces );

		BinaryRead ( &pObject->frames[i].num_verts, sizeof ( int ), 1, m_FilePointer, pack );
		pObject->frames[i].num_verts = ENDIAN_SWAP_32 ( pObject->frames[i].num_verts );
		BinaryRead ( &pObject->frames[i].num_texcoords, sizeof ( int ), 1, m_FilePointer, pack );
		pObject->frames[i].num_texcoords = ENDIAN_SWAP_32 ( pObject->frames[i].num_texcoords );
		BinaryRead ( &pObject->frames[i].num_normals, sizeof ( int ), 1, m_FilePointer, pack );
		pObject->frames[i].num_normals = ENDIAN_SWAP_32 ( pObject->frames[i].num_normals );

		pObject->frames[i].verts = new JOEVertex [pObject->frames[i].num_verts];
		pObject->frames[i].normals = new JOEVertex [pObject->frames[i].num_normals];
		pObject->frames[i].texcoords = new JOETexCoord [pObject->frames[i].num_texcoords];

		BinaryRead ( pObject->frames[i].verts, sizeof ( JOEVertex ), pObject->frames[i].num_verts, m_FilePointer, pack );
		CorrectEndian ( pObject->frames[i].verts, pObject->frames[i].num_verts );
		BinaryRead ( pObject->frames[i].normals, sizeof ( JOEVertex ), pObject->frames[i].num_normals, m_FilePointer, pack );
		CorrectEndian ( pObject->frames[i].normals, pObject->frames[i].num_normals );
		BinaryRead ( pObject->frames[i].texcoords, sizeof ( JOETexCoord ), pObject->frames[i].num_texcoords, m_FilePointer, pack );
		CorrectEndian ( pObject->frames[i].texcoords, pObject->frames[i].num_texcoords );
	}

	//cout << "!!! loading " << modelpath << endl;
	
	//go do scaling
	for (int i = 0; i < num_frames; i++)
	{
		for ( int v = 0; v < pObject->frames[i].num_verts; v++ )
		{
			MATHVECTOR <float, 3> temp;

			temp.Set ( pObject->frames[i].verts[v].vertex );
			temp = temp * MODEL_SCALE;

			for (int n = 0; n < 3; n++)
				pObject->frames[i].verts[v].vertex[n] = temp[n];
		}
	}
	
	if (NeedsNormalSwap(pObject))
	{
		for (int i = 0; i < num_frames; i++)
		{
			for ( int v = 0; v < pObject->frames[i].num_normals; v++ )
			{
				std::swap(pObject->frames[i].normals[v].vertex[1],
					  pObject->frames[i].normals[v].vertex[2]);
				pObject->frames[i].normals[v].vertex[1] = -pObject->frames[i].normals[v].vertex[1];
			}
		}
		//std::cout << "!!! swapped normals !!!" << std::endl;
	}
	
	//assert(!NeedsNormalFlip(pObject));

	/*//make sure vertex ordering is consistent with normals
	for (i = 0; i < pObject->info.num_faces; i++)
	{
		short vi[3];
		VERTEX tri[3];
		VERTEX norms[3];
		for (unsigned int v = 0; v < 3; v++)
		{
			vi[v] = GetFace(i)[v];
			tri[v].Set(GetVert(vi[v]));
			norms[v].Set(GetNorm(GetNormIdx(i)[v]));
		}
		VERTEX norm;
		for (unsigned int v = 0; v < 3; v++)
			norm = norm + norms[v];
		norm = norm.normalize();
		VERTEX tnorm = (tri[2] - tri[0]).cross(tri[1] - tri[0]);
		if (norm.dot(tnorm) > 0)
		{
			short tvi = pObject->frames[0].faces[i].vertexIndex[1];
			pObject->frames[0].faces[i].vertexIndex[1] = pObject->frames[0].faces[i].vertexIndex[2];
			pObject->frames[0].faces[i].vertexIndex[2] = tvi;
		
			tvi = pObject->frames[0].faces[i].normalIndex[1];
			pObject->frames[0].faces[i].normalIndex[1] = pObject->frames[0].faces[i].normalIndex[2];
			pObject->frames[0].faces[i].normalIndex[2] = tvi;
		
			tvi = pObject->frames[0].faces[i].textureIndex[1];
			pObject->frames[0].faces[i].textureIndex[1] = pObject->frames[0].faces[i].textureIndex[2];
			pObject->frames[0].faces[i].textureIndex[2] = tvi;
		}
	}*/
	
	//build unique vertices
	//cout << "building unique vertices...." << endl;
	int frame(0);
	
	typedef size_t size_type;
	
	vector <VERT_ENTRY> vert_master ((size_type)(pObject->frames[frame].num_verts));
	vert_master.reserve(pObject->frames[frame].num_verts*2);
	
	vector <int> v_faces((size_type)(pObject->info.num_faces*3));
	
	for (int i = 0; i < pObject->info.num_faces; i++)
	{
		for (int v = 0; v < 3; v++)
		{
			VERT_ENTRY & ve = vert_master[pObject->frames[frame].faces[i].vertexIndex[v]];
			if (ve.original_index == -1) //first entry
			{
				ve.original_index = pObject->frames[frame].faces[i].vertexIndex[v];
				ve.norm_index = pObject->frames[frame].faces[i].normalIndex[v];
				ve.tex_index = pObject->frames[frame].faces[i].textureIndex[v];
				//if (ve.tex_index < 0)
				assert(ve.tex_index >= 0);
				
				v_faces[i*3+v] = pObject->frames[frame].faces[i].vertexIndex[v];
				//cout << "(first) face " << i << " vert " << v << " index: " << v_faces[i*3+v] << endl;
				
				//cout << "first entry: " << ve.original_index << "," << ve.norm_index << "," << ve.tex_index << endl;
			}
			else
			{
				//see if we match the pre-existing entry
				if (ve.norm_index == pObject->frames[frame].faces[i].normalIndex[v] &&
					ve.tex_index == pObject->frames[frame].faces[i].textureIndex[v])
				{
					v_faces[i*3+v] = pObject->frames[frame].faces[i].vertexIndex[v];
					assert(ve.tex_index >= 0);
					//cout << "(matched) face " << i << " vert " << v << " index: " << v_faces[i*3+v] << endl;
					
					//cout << "matched entry: " << ve.original_index << "," << ve.norm_index << "," << ve.tex_index << endl;
				}
				else
				{
					//create a new entry
					vert_master.push_back(VERT_ENTRY());
					vert_master.back().original_index = pObject->frames[frame].faces[i].vertexIndex[v];
					vert_master.back().norm_index = pObject->frames[frame].faces[i].normalIndex[v];
					vert_master.back().tex_index = pObject->frames[frame].faces[i].textureIndex[v];
					
					assert(vert_master.back().tex_index >= 0);
					
					v_faces[i*3+v] = vert_master.size()-1;
					//cout << "(new) face " << i << " vert " << v << " index: " << v_faces[i*3+v] << endl;
					
					//cout << "new entry: " << vert_master.back().original_index << "," << vert_master.back().norm_index << "," << vert_master.back().tex_index << " (" << ve.original_index << "," << ve.norm_index << "," << ve.tex_index << ")" << endl;
				}
			}
		}
	}
	
	/*for (int i = 0; i < vert_master.size(); i++)
	{
		if (vert_master[i].original_index < 0)
			std::cout << i << ", " << pObject->frames[frame].num_verts << ", " << vert_master.size() << std::endl;
		assert(vert_master[i].original_index >= 0);
	}*/
	
	float newvertnum = vert_master.size();
	/*std::cout << modelpath << " (" << pObject->info.num_faces << ") used to have " << pObject->frames[frame].num_verts << " vertices, " << 
			pObject->frames[frame].num_normals << " normals, " << pObject->frames[frame].num_texcoords 
			<< " tex coords, now it has " << newvertnum << " combo verts (combo indices)" << std::endl;*/
	
	//now, fill up the vertices, normals, and texcoords
	vector <float> v_vertices((size_type)(newvertnum*3));
	vector <float> v_texcoords((size_type)(newvertnum*2));
	vector <float> v_normals((size_type)(newvertnum*3));
	for (int i = 0; i < newvertnum; ++i)
	{
		if (vert_master[i].original_index >= 0)
		{
			for (int d = 0; d < 3; d++)
				v_vertices[i*3+d] = pObject->frames[frame].verts[vert_master[i].original_index].vertex[d];
			
			for (int d = 0; d < 3; d++)
				v_normals[i*3+d] = pObject->frames[frame].normals[vert_master[i].norm_index].vertex[d];
			
			//std::cout << i << ", " << vert_master[i].tex_index << ", " << vert_master.size() << ", " << pObject->frames[frame].num_texcoords << std::endl;
			//assert(vert_master[i].tex_index >= 0);
			//assert(vert_master[i].tex_index < pObject->frames[frame].num_texcoords);
			if (vert_master[i].tex_index < pObject->frames[frame].num_texcoords)
			{
				v_texcoords[i*2+0] = pObject->frames[frame].texcoords[vert_master[i].tex_index].u;
				v_texcoords[i*2+1] = pObject->frames[frame].texcoords[vert_master[i].tex_index].v;
			}
			else
			{
				v_texcoords[i*2+0] = 0;
				v_texcoords[i*2+1] = 0;
			}
		}
	}
	
	/*for (int i = 0; i < newvertnum; i++)
		cout << v_vertices[i*3] << "," << v_vertices[i*3+1] << "," << v_vertices[i*3+2] << endl;*/
	
	//assign to our mesh
	mesh.SetFaces(&v_faces[0], v_faces.size());
	mesh.SetVertices(&v_vertices[0], v_vertices.size());
	mesh.SetNormals(&v_normals[0], v_normals.size());
	mesh.SetTexCoordSets(1);
	mesh.SetTexCoords(0, &v_texcoords[0], v_texcoords.size());
}

void MODEL_JOE03::CorrectEndian ( struct JOEFace * p, int num )
{
	int i;

	for ( i = 0; i < num; i++ )
	{
		int d;
		for ( d = 0; d < 3; d++ )
		{
			p[i].vertexIndex[d] = ENDIAN_SWAP_16 ( p[i].vertexIndex[d] );
			p[i].normalIndex[d] = ENDIAN_SWAP_16 ( p[i].normalIndex[d] );
			p[i].textureIndex[d] = ENDIAN_SWAP_16 ( p[i].textureIndex[d] );
		}
	}
}

void MODEL_JOE03::CorrectEndian ( struct JOEVertex *p, int num )
{
	int i;

	for ( i = 0; i < num; i++ )
	{
		int d;
		for ( d = 0; d < 3; d++ )
		{
			p[i].vertex[d] = ENDIAN_SWAP_FLOAT ( p[i].vertex[d] );
		}
	}
}

void MODEL_JOE03::CorrectEndian ( struct JOETexCoord *p, int num )
{
	int i;

	for ( i = 0; i < num; i++ )
	{
		p[i].u = ENDIAN_SWAP_FLOAT ( p[i].u );
		p[i].v = ENDIAN_SWAP_FLOAT ( p[i].v );
	}
}

///fix invalid normals (my own fault, i suspect.  the DOF converter i wrote may have flipped Y & Z normals)
bool MODEL_JOE03::NeedsNormalSwap(JOEObject * pObject)
{
	bool need_normal_flip = false;
	for (int f = 0; f < pObject->info.num_frames; f++)
	{
		int normal_flip_count = 0;
		for (int i = 0; i < pObject->info.num_faces; i++)
		{
			MATHVECTOR <float,3> tri[3];
			MATHVECTOR <float,3> norms[3];
			for (unsigned int v = 0; v < 3; v++)
			{
				assert(pObject->frames[f].faces[i].vertexIndex[v] < pObject->frames[f].num_verts);
				assert(pObject->frames[f].faces[i].normalIndex[v] < pObject->frames[f].num_normals);
				tri[v].Set(pObject->frames[f].verts[pObject->frames[f].faces[i].vertexIndex[v]].vertex);
				norms[v].Set(pObject->frames[f].normals[pObject->frames[f].faces[i].normalIndex[v]].vertex);
			}
			MATHVECTOR <float,3> norm;
			for (unsigned int v = 0; v < 3; v++)
				norm = norm + norms[v];
			MATHVECTOR <float,3> tnorm = (tri[2] - tri[0]).cross(tri[1] - tri[0]);
			if (tnorm.Magnitude() > 0.0001 && norm.Magnitude() > 0.0001)
			{
				norm = norm.Normalize();
				tnorm = tnorm.Normalize();
				if (norm.dot(tnorm) < 0.5 && norm.dot(tnorm) > -0.5)
				{
					normal_flip_count++;
					//std::cout << norm.dot(tnorm) << std::endl;
					//std::cout << norm << " -- " << tnorm << std::endl;
				}
			}
		}
		
		if (normal_flip_count > pObject->info.num_faces/4)
			need_normal_flip = true;
	}
	return need_normal_flip;
}
