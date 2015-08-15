#include "pch.h"
#include "model_obj.h"
#include "unittest.h"
#include "vertexarray.h"

#include <algorithm>

#include <fstream>
using std::ifstream;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::ostream;
using std::endl;

#include <vector>
using std::vector;

///extract a formatted string from the output, ignoring comments
string ReadFromStream(std::istream & s)
{
	if (!s)
		return "";
	std::streampos curpos = s.tellg();
	assert(curpos >= 0);
	string str;
	s >> str;
	if (!s || str.empty())
		return "";
	if (str[0] == '#')
	{
		s.seekg(curpos);
		std::getline(s,str);
		return ReadFromStream(s);
	}
	return str;
}

///extract a repeating value, return true on success.
template <typename T>
bool ExtractRepeating(std::vector <T> & output_vector, unsigned int repeats, std::istream & s)
{
	if (!s)
		return false;
	for (unsigned int i = 0; i < repeats; ++i)
	{
		std::string strformat = ReadFromStream(s);
		if (strformat.empty())
			return false;
		std::stringstream reformat(strformat);
		T reformatted;
		reformat >> reformatted;
		output_vector.push_back(reformatted);
	}
	
	if (output_vector.size() != repeats)
		return false;
	
	return true;
}

bool ExtractTriFloat(std::vector <VERTEXARRAY::TRIFLOAT> & output_vector, const std::string & section, std::istream & s,  std::ostream & error_log, const std::string & filepath)
{
	std::vector <float> coords;
	if (!ExtractRepeating(coords, 3, s))
	{
		error_log << "Error reading " << section << " in " << filepath << endl;
		return false;
	}
	output_vector.push_back(VERTEXARRAY::TRIFLOAT(coords[0],coords[1],coords[2]));
	return true;
}

bool ExtractTwoFloat(std::vector <VERTEXARRAY::TWOFLOAT> & output_vector, const std::string & section, std::istream & s,  std::ostream & error_log, const std::string & filepath)
{
	std::vector <float> coords;
	if (!ExtractRepeating(coords, 2, s))
	{
		error_log << "Error reading " << section << " in " << filepath << endl;
		return false;
	}
	output_vector.push_back(VERTEXARRAY::TWOFLOAT(coords[0],coords[1]));
	return true;
}

bool BuildVertex(VERTEXARRAY::VERTEXDATA & outputvert, std::vector <VERTEXARRAY::TRIFLOAT> & verts, std::vector <VERTEXARRAY::TRIFLOAT> & normals, std::vector <VERTEXARRAY::TWOFLOAT> & texcoords, const std::string & facestr)
{
	if (std::count(facestr.begin(), facestr.end(), '/') != 2)
		return false;
	
	if (facestr.find("//") != std::string::npos)
		return false;
	
	std::string facestr2 = facestr;
	std::replace(facestr2.begin(), facestr2.end(), '/', ' ');
	std::stringstream s(facestr2);
	int v(-1),t(-1),n(-1);
	s >> v >> t >> n;
	if (v <= 0 || t <= 0 || n <= 0)
		return false;
	
	outputvert.vertex = verts[v-1];
	outputvert.normal = normals[n-1];
	outputvert.texcoord = texcoords[t-1];
	
	return true;
}

bool MODEL_OBJ::Load(const std::string & filepath, std::ostream & error_log, bool genlist)
{
	std::ifstream f(filepath.c_str());
	if (!f)
	{
		error_log << "Couldn't open object file: " << filepath << endl;
		return false;
	}
	
	std::vector <VERTEXARRAY::TRIFLOAT> verts;
	std::vector <VERTEXARRAY::TRIFLOAT> normals;
	std::vector <VERTEXARRAY::TWOFLOAT> texcoords;
	std::vector <VERTEXARRAY::FACE> faces;
	
	while (f)
	{
		string id = ReadFromStream(f);
		
		if (id == "v")
		{
			if (!ExtractTriFloat(verts, "vertices", f, error_log, filepath)) return false;
		}
		else if (id == "vn")
		{
			if (!ExtractTriFloat(normals, "normals", f, error_log, filepath)) return false;
		}
		else if (id == "vt")
		{
			if (!ExtractTwoFloat(texcoords, "texcoords", f, error_log, filepath)) return false;
			texcoords.back().v = 1.0-texcoords.back().v;
		}
		else if (id == "f")
		{
			std::vector <string> faceverts;
			if (!ExtractRepeating(faceverts, 3, f))
			{
				error_log << "Error reading faces in " << filepath << endl;
				return false;
			}
			VERTEXARRAY::VERTEXDATA newverts[3];
			for (int i = 0; i < 3; ++i)
			{
				if (!BuildVertex(newverts[i], verts, normals, texcoords, faceverts[i]))
				{
					error_log << "Error: obj file has faces without texture and normal data: " << faceverts[i] << " in " << filepath << endl;
					return false;
				}
			}
			VERTEXARRAY::FACE newface(newverts[0], newverts[1], newverts[2]);
			faces.push_back(newface);
		}
	}
	
	mesh.BuildFromFaces(faces);
	GenerateMeshMetrics();
	//if (genlist)
		//GenerateListID(error_log);
	
	return true;
}

void WriteRange(std::ostream & s, const std::vector <float> & v, int startidx, int endidx)
{
	assert(startidx >= 0 && startidx < (int) v.size() && startidx < endidx && startidx != endidx);
	assert(endidx <= (int) v.size());
	for (int i = startidx; i < endidx; ++i)
	{
		if (i != startidx)
			s << " ";
		s << v[i];
	}
}

void WriteVectorGroupings(std::ostream & s, const std::vector <float> & v, const std::string & id, int groupsize)
{
	assert(groupsize > 0);
	for (int i = 0; i < (int)v.size()/groupsize; ++i)
	{
		s << id << " ";
		WriteRange(s, v, i*groupsize, i*groupsize+groupsize);
		s << endl;
	}
}

void WriteFace(std::ostream & s, int index)
{
	s << index << "/" << index << "/" << index;
}

bool MODEL_OBJ::Save(const std::string & strFileName, std::ostream & error_output) const
{
	std::ofstream f(strFileName.c_str());
	if (!f)
	{
		error_output << "Error opening file for writing: " << error_output << endl;
		return false;
	}
	
	f << "# Model conversion utility by Joe Venzon" << endl << endl;
	
	WriteVectorGroupings(f, mesh.vertices, "v", 3);
	f << endl;
	WriteVectorGroupings(f, mesh.texcoords[0], "vt", 2);
	f << endl;
	WriteVectorGroupings(f, mesh.normals, "vn", 3);
	f << endl;
	
	for (int i = 0; i < (int)mesh.faces.size()/3; ++i)
	{
		f << "f ";
		for (int v = 0; v < 3; ++v)
		{
			WriteFace(f,mesh.faces[i*3+v]+1);
			f << " ";
		}
		f << endl;
	}
	
	return true;
}

