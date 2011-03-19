#include "stdafx.h"

#include "model.h"
//#include "opengl_utility.h"

//#include <fstream>

using std::vector;

void MODEL::GenerateListID(std::ostream & error_output)
{
	ClearListID();
	
//	listid = glGenLists(1);
//	glNewList (listid, GL_COMPILE);
	
//	glBegin(GL_TRIANGLES);
	
	int true_faces = mesh.GetNumFaces() / 3;
	
	//cout << "generating list from " << true_faces << " faces" << endl;
	
	// iterate through all of the faces (polygons)
	for(int j = 0; j < true_faces; j++)
	{
		// iterate though each vertex in the face
		for(int whichVertex = 0; whichVertex < 3; whichVertex++)
		{
			// get the 3D location for this vertex
			///vert array bounds are not checked but it is assumed to be of size 3
			std::vector <float> vert = mesh.GetVertex(j, whichVertex);
			
			// get the 3D normal for this vertex
			///norm array bounds are not checked but it is assumed to be of size 3
			std::vector <float> norm = mesh.GetNormal(j, whichVertex);
			
			assert (mesh.GetTexCoordSets() > 0);
			
			if (mesh.GetTexCoordSets() > 0)
			{
				// get the 2D texture coordinates for this vertex
				///tex array bounds are not checked but it is assumed to be of size 2
				std::vector <float> tex = mesh.GetTextureCoordinate(j, whichVertex, 0);
				
				//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, tex[0], tex[1]);
			}

			//glNormal3fv(&norm[0]);
			//glVertex3fv(&vert[0]);
		}
	}

//	glEnd();
	
//	glEndList ();
	
	generatedlistid = true;
	
//	OPENGL_UTILITY::CheckForOpenGLErrors("model list ID generation", error_output);
}

void MODEL::GenerateMeshMetrics()
{
	float maxv[3];
	float minv[3];
	bool havevals[6];
	for ( int n = 0; n < 6; n++ )
		havevals[n] = false;

	const float * verts;
	int vnum;
	mesh.GetVertices(verts, vnum);
	vnum = vnum / 3;
	for ( int v = 0; v < vnum; v++ )
	{
		MATHVECTOR <float, 3> temp;

		temp.Set ( verts + v*3 );
		
		//cout << verts[v*3] << "," << verts[v*3+1] << "," << verts[v*3+2] << endl;
	
		//cache for bbox stuff
		for ( int n = 0; n < 3; n++ )
		{
			if (!havevals[n])
			{
				maxv[n] = temp[n];
				havevals[n] = true;
			}
			else if (temp[n] > maxv[n])
				maxv[n] = temp[n];
			
			if (!havevals[n+3])
			{
				minv[n] = temp[n];
				havevals[n+3] = true;
			}
			else if (temp[n] < minv[n])
				minv[n] = temp[n];
		}

		float r = temp.Magnitude();
		MATHVECTOR <float, 2> tempxz;
		tempxz.Set(temp[0], temp[2]);
		float rxz = tempxz.Magnitude();
		if ( r > radius )
			radius = r;
		if ( rxz > radiusxz )
			radiusxz = rxz;
	}

	bboxmin.Set(minv[0], minv[1], minv[2]);
	bboxmax.Set(maxv[0], maxv[1], maxv[2]);
	
	MATHVECTOR <float, 3> center;
	center = (bboxmin + bboxmax)*0.5;
	radius = (bboxmin - center).Magnitude();
	
	MATHVECTOR <float, 3> minv_noy = bboxmin;
	minv_noy[1] = 0;
	center[1] = 0;
	radiusxz = (minv_noy - center).Magnitude();
	
	generatedmetrics = true;
}

void MODEL::ClearListID()
{
	//if (generatedlistid)
	//	glDeleteLists(listid, 1);
	generatedlistid = false;
}

bool MODEL::WriteToFile(const std::string & filepath)
{
	const std::string magic = "OGLVARRAYV01";
	std::ofstream fileout(filepath.c_str());
	if (!fileout)
		return false;
	
	fileout.write(magic.c_str(), magic.size());
	joeserialize::BinaryOutputSerializer s(fileout);
	return Serialize(s);
}

bool MODEL::ReadFromFile(const std::string & filepath, std::ostream & error_output, bool generatelistid)
{
	std::ifstream filein(filepath.c_str(), std::ios_base::binary);
	if (!filein)
	{
		error_output << "Can't find file: " << filepath << std::endl;
		return false;
	}
	//else std::cout << "File " << filepath << " exists!" << std::endl;
	
	const std::string magic = "OGLVARRAYV01";
	const int magicsize = 12;//magic.size();
	char fmagic[magicsize+1];
	filein.read(fmagic, magic.size());
	if (!filein)
	{
		error_output << "File magic read error: " << filepath << std::endl;
		return false;
	}
	
	fmagic[magic.size()] = '\0';
	std::string fmagicstr = fmagic;
	if (magic != fmagic)
	{
		error_output << "File magic is incorrect: \"" << magic << "\" != \"" << fmagic << "\" in " << filepath << std::endl;
		return false;
	}
	
	joeserialize::BinaryInputSerializer s(filein);
	if (!Serialize(s))
	{
		error_output << "Serialization error: " << filepath << std::endl;
		Clear();
		return false;
	}
	
	ClearListID();
	ClearMetrics();
	GenerateMeshMetrics();
	if (generatelistid)
	GenerateListID(error_output);
	
	return true;
}
