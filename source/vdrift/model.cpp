#include "pch.h"
#include "model.h"

#include <fstream>

void MODEL::GenerateMeshMetrics()
{
	float maxv[3] = {0, 0, 0};
	float minv[3] = {0, 0, 0};
	bool havevals[6];
	for ( int n = 0; n < 6; ++n )
		havevals[n] = false;

	const float * verts;
	int vnum;
	mesh.GetVertices(verts, vnum);
	vnum = vnum / 3;
	for ( int v = 0; v < vnum; ++v )
	{
		MATHVECTOR<float,3> temp;

		temp.Set ( verts + v*3 );
		
		//cout << verts[v*3] << "," << verts[v*3+1] << "," << verts[v*3+2] << endl;
	
		//cache for bbox stuff
		for ( int n = 0; n < 3; ++n )
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
		MATHVECTOR<float, 2> tempxz;
		tempxz.Set(temp[0], temp[2]);
		float rxz = tempxz.Magnitude();
		if ( r > radius )
			radius = r;
		if ( rxz > radiusxz )
			radiusxz = rxz;
	}

	bboxmin.Set(minv[0], minv[1], minv[2]);
	bboxmax.Set(maxv[0], maxv[1], maxv[2]);
	
	MATHVECTOR<float,3> center;
	center = (bboxmin + bboxmax)*0.5;
	radius = (bboxmin - center).Magnitude();
	
	MATHVECTOR<float,3> minv_noy = bboxmin;
	minv_noy[1] = 0;
	center[1] = 0;
	radiusxz = (minv_noy - center).Magnitude();
	
	generatedmetrics = true;
}

void MODEL::ClearListID()
{
	//if (generatedlistid)
		//glDeleteLists(listid, 1);
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
	const bool verbose = false;
	
	if (verbose) std::cout << filepath << ": starting mesh read" << std::endl;
	
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
	
	if (verbose) std::cout << filepath << ": serializing" << std::endl;
	
	/*// read the entire file into the memfile stream
	std::streampos start = filein.tellg();
	filein.seekg(0,std::ios::end);
	std::streampos length = filein.tellg() - start;
	filein.seekg(start);
	std::vector <char> buffer(length);
	filein.read(&buffer[0],length);
	std::stringstream memfile;
	memfile.rdbuf()->pubsetbuf(&buffer[0],length);*/
	
	joeserialize::BinaryInputSerializer s(filein);
	//joeserialize::BinaryInputSerializer s(memfile);
	if (!Serialize(s))
	{
		error_output << "Serialization error: " << filepath << std::endl;
		Clear();
		return false;
	}
	
	if (verbose) std::cout << filepath << ": generating metrics" << std::endl;
	
	ClearListID();
	ClearMetrics();
	GenerateMeshMetrics();
	if (verbose) std::cout << filepath << ": generating list id" << std::endl;
	//if (generatelistid)
		//GenerateListID(error_output);
	
	if (verbose) std::cout << filepath << ": done" << std::endl;
	
	return true;
}

void MODEL::SetVertexArray(const VERTEXARRAY & newmesh)
{
	Clear();
	
	mesh = newmesh;
}

void MODEL::BuildFromVertexArray(const VERTEXARRAY & newmesh, std::ostream & error_output)
{
	SetVertexArray(newmesh);
	
	//generate metrics such as bounding box, etc
	GenerateMeshMetrics();
	
	//optimize into a static display list
	//GenerateListID(error_output);
}

void MODEL::Translate(float x, float y, float z)
{
	mesh.Translate(x, y, z);
}

void MODEL::Rotate(float a, float x, float y, float z)
{
	mesh.Rotate(a, x, y, z);
}
	
void MODEL::Scale(float x, float y, float z)
{
	//mesh.Scale(x,y,z);
}
