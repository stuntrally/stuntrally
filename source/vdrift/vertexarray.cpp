#include "pch.h"
#include "vertexarray.h"
#include "unittest.h"
#include "mathvector.h"
#include "quaternion.h"
#include "matrix4.h"

//#include <map>
//#include <cassert>

QT_TEST(vertexarray_test)
{
/*	VERTEXARRAY testarray;
	
	const float * ptr;
	int ptrnum;
	float somevec[3];
	somevec[0] = somevec[2] = 0;
	somevec[1] = 1000.0;
	testarray.SetNormals(somevec, 3);
	testarray.GetNormals(ptr, ptrnum);
	QT_CHECK_EQUAL(ptr[1], 1000.0);
	QT_CHECK_EQUAL(ptrnum, 3);
	testarray.SetNormals(NULL, 0);
	testarray.GetNormals(ptr, ptrnum);
	QT_CHECK_EQUAL(ptrnum, 0);
	QT_CHECK_EQUAL(ptr, NULL);
		
	//by similarity, the vertex and face assignment functions are OK if the above normal test is OK
	
	testarray.SetTexCoordSets(2);
	QT_CHECK_EQUAL(testarray.GetTexCoordSets(), 2);
	testarray.GetTexCoords(0, ptr, ptrnum);
	QT_CHECK_EQUAL(ptrnum, 0);
	QT_CHECK_EQUAL(ptr, NULL);
	testarray.SetTexCoords(1, somevec, 2);
	testarray.GetTexCoords(1, ptr, ptrnum);
	QT_CHECK_EQUAL(ptrnum, 2);
	QT_CHECK_EQUAL(ptr[1], 1000.0);
	
	testarray.Clear();
	testarray.SetNormals(somevec, 3);
	VERTEXARRAY otherarray(testarray);
	otherarray.GetNormals(ptr, ptrnum);
	QT_CHECK_EQUAL(ptr[1], 1000.0);
	QT_CHECK_EQUAL(ptrnum, 3);
	otherarray.Clear();
	otherarray.GetNormals(ptr, ptrnum);
	QT_CHECK_EQUAL(ptrnum, 0);
	otherarray = testarray;
	otherarray.GetNormals(ptr, ptrnum);
	QT_CHECK_EQUAL(ptr[1], 1000.0);
	QT_CHECK_EQUAL(ptrnum, 3);
	
	VERTEXARRAY addarray;
	otherarray.Clear();
	testarray.Clear();
	testarray.SetNormals(somevec, 3);
	otherarray.SetNormals(somevec, 3);
	addarray = testarray + otherarray;
	addarray.GetNormals(ptr, ptrnum);
	QT_CHECK_EQUAL(ptrnum, 6);
	QT_CHECK_EQUAL(ptr[1], 1000.0);
	QT_CHECK_EQUAL(ptr[4], 1000.0);
	
	testarray.Clear();
	VERTEXARRAY facearray1, facearray2;
	int someint[3];
	someint[0] = 0;
	someint[1] = 1;
	someint[2] = 2;
	facearray1.SetFaces(someint, 3);
	facearray2.SetFaces(someint, 3);
	testarray = facearray1 + facearray2;
	const int * ptri;
	testarray.GetFaces(ptri, ptrnum);
	QT_CHECK_EQUAL(ptrnum, 6);
	QT_CHECK_EQUAL(ptri[1], 1);
	QT_CHECK_EQUAL(ptri[4], 1);
	testarray.Clear();
	facearray1.SetVertices(somevec, 3);
	facearray2.SetVertices(somevec, 3);
	testarray = facearray1 + facearray2;
	testarray.GetFaces(ptri, ptrnum);
	QT_CHECK_EQUAL(ptrnum, 6);
	QT_CHECK_EQUAL(ptri[1], 1);
	QT_CHECK_EQUAL(ptri[4], 2);*/
}

void VERTEXARRAY::SetNormals(float * newarray, size_t newarraycount)
{
	//Tried to assign values that aren't in sets of 3
	assert(newarraycount % 3 == 0);
	
	if (newarraycount != normals.size())
	{
		normals.resize(newarraycount);
	}
	
	float * myarray = &(normals[0]);
	for (size_t i = 0; i < newarraycount; ++i)
	{
		myarray[i] = newarray[i];
	}
	
	/*for (int i = 0; i < newarraycount; ++i)
	{
		normals[i] = newarray[i];
	}*/
}

void VERTEXARRAY::SetVertices(float * newarray, size_t newarraycount)
{
	//Tried to assign values that aren't in sets of 3
	assert(newarraycount % 3 == 0);
	
	if (newarraycount != vertices.size())
	{
		//cout << "New size: " << newarraycount << " (was " << vertices.size() << ")" << endl;
		vertices.resize(newarraycount);
	}
	
	float * myarray = &(vertices[0]);
	for (size_t i = 0; i < newarraycount; ++i)
	{
		myarray[i] = newarray[i];
	}
	
	/*int i = 0;
	ITERVEC(float, vertices, it)
	{
		*it = newarray[i];
		++i;
	}*/
}

void VERTEXARRAY::SetFaces(int * newarray, size_t newarraycount)
{
	//Tried to assign values that aren't in sets of 3
	assert (newarraycount % 3 == 0);
	
	if (newarraycount != faces.size())
		faces.resize(newarraycount);
	
	
	int * myarray = &(faces[0]);
	for (size_t i = 0; i < newarraycount; ++i)
	{
		myarray[i] = newarray[i];
	}
	
	/*for (int i = 0; i < newarraycount; ++i)
	{
		faces[i] = newarray[i];
	}*/
}

void VERTEXARRAY::SetTexCoordSets(int newtcsets)
{
	texcoords.clear();
	texcoords.resize(newtcsets);
}

//set is zero indexed
void VERTEXARRAY::SetTexCoords(size_t set, float * newarray, size_t newarraycount)
{
	//Tried to assign a tex coord set beyond the allocated number of sets
	assert(set < texcoords.size());
	
	//Tried to assign values that aren't in sets of 2
	assert(newarraycount % 2 == 0);
	
	if (texcoords[set].size() != newarraycount)
		texcoords[set].resize(newarraycount);
	
	float * myarray = &(texcoords[set][0]);
	for (size_t i = 0; i < newarraycount; ++i)
	{
		myarray[i] = newarray[i];
	}
	
	/*for (int i = 0; i < newarraycount; ++i)
	{
		texcoords[set][i] = newarray[i];
	}*/
}

void VERTEXARRAY::GetNormals(const float * & output_array_pointer, int & output_array_num) const
{
	output_array_num = normals.size();
	output_array_pointer = normals.empty() ? NULL : &normals[0];
}

void VERTEXARRAY::GetVertices(const float * & output_array_pointer, int & output_array_num) const
{
	output_array_num = vertices.size();
	output_array_pointer = vertices.empty() ? NULL : &vertices[0];
}

void VERTEXARRAY::GetFaces(const int * & output_array_pointer, int & output_array_num) const
{
	output_array_num = faces.size();
	//output_array_pointer = faces.empty() ? NULL : &faces[0];
	if (faces.size() < 1)
		output_array_pointer = NULL;
	else
		output_array_pointer = &faces[0];
}

void VERTEXARRAY::GetTexCoords(size_t set, const float * & output_array_pointer, int & output_array_num) const
{
	//Tried to get a tex coord set beyond the allocated number of sets
	assert(set < texcoords.size());
	
	output_array_num = texcoords[set].size();
	output_array_pointer = texcoords[set].empty() ? NULL : &texcoords[set][0];
}

#define COMBINEVECTORS(vname) {out.vname.reserve(vname.size() + v.vname.size());out.vname.insert(out.vname.end(), vname.begin(), vname.end());out.vname.insert(out.vname.end(), v.vname.begin(), v.vname.end());}

VERTEXARRAY VERTEXARRAY::operator+ (const VERTEXARRAY & v) const
{
	VERTEXARRAY out;
	
	//out.normals.reserve(normals.size() + v.normals.size());
	//out.normals.insert(out.normals.end(), normals.begin(), normals.end());
	//out.normals.insert(out.normals.end(), v.normals.begin(), v.normals.end());
	
	int idxoffset = vertices.size()/3;
	
	COMBINEVECTORS(normals)
	COMBINEVECTORS(vertices)
	
	//COMBINEVECTORS(faces)
	out.faces.reserve(faces.size() + v.faces.size());
	out.faces.insert(out.faces.end(), faces.begin(), faces.end());
	//out.faces.insert(out.faces.end(), v.faces.begin(), v.faces.end());
	//size_t startat = out.faces.size();
	for (size_t i = 0; i < v.faces.size(); ++i)
	{
		//out.faces[faces.size() + i] = v.faces[i] + idxoffset;
		//out.faces[startat + i] = v.faces[i];
		out.faces.push_back(v.faces[i]+idxoffset);
	}
	
	int maxtcsets = GetTexCoordSets();
	if (v.GetTexCoordSets() > maxtcsets)
		maxtcsets = v.GetTexCoordSets();
	int tcsets1 = GetTexCoordSets();
	int tcsets2 = v.GetTexCoordSets();
	out.SetTexCoordSets(maxtcsets);
	//ITERVEC(vector <float>,texcoords,i)
	for (int i = 0; i < maxtcsets; ++i)
	{
		if (i >= tcsets1 && i < tcsets2)
		{
			out.texcoords[i] = v.texcoords[i];
		}
		else if (i < tcsets1 && i >= tcsets2)
		{
			out.texcoords[i] = texcoords[i];
		}
		else if (i < tcsets1 && i < tcsets2)
		{
			COMBINEVECTORS(texcoords[i])
		}
	}
	
	return out;
}

void VERTEXARRAY::Add(float * newnorm, int newnormcount, float * newvert, int newvertcount,
			int * newfaces, int newfacecount,
			float * newtc, int newtccount)
{
	int idxoffset = vertices.size()/3;
	
	//add normals
	{
		int newcount = newnormcount;
		int origsize = normals.size();
		
		//Tried to assign values that aren't in sets of 3
		assert(newcount % 3 == 0);
		
		int newsize = origsize + newcount;
		if (newcount > 0)
		{
			normals.resize(newsize);
		}
		if (normals.size() != 0)
		{
			float * myarray = &(normals[0]);
			for (int i = 0; i < newcount; ++i)
			{
				myarray[i+origsize] = newnorm[i];
			}
		}
	}
	
	//add verts
	{
		int newcount = newvertcount;
		int origsize = vertices.size();
		
		//Tried to assign values that aren't in sets of 3
		assert(newcount % 3 == 0);
		
		int newsize = origsize + newcount;
		if (newcount > 0)
		{
			vertices.resize(newsize);
		}
		if (vertices.size() != 0)
		{
			float * myarray = &(vertices[0]);
			for (int i = 0; i < newcount; ++i)
			{
				myarray[i+origsize] = newvert[i];
			}
		}
	}
	
	//add faces
	{
		int newcount = newfacecount;
		int origsize = faces.size();
		
		//Tried to assign values that aren't in sets of 3
		assert (newcount % 3 == 0);
			
		int newsize = origsize + newcount;
		if (newcount > 0)
		{
			faces.resize(newsize);
		}
		if (faces.size() != 0)
		{
			int * myarray = &(faces[0]);
			for (int i = 0; i < newcount; ++i)
			{
				myarray[i+origsize] = newfaces[i] + idxoffset;
			}
		}
	}
	
	//add tex coords
	{
		//This version of VERTEXARRAY::Add assumes texcoordsets == 1
		assert(GetTexCoordSets() == 1);
		
		int newcount = newtccount;
		int origsize = texcoords[0].size();
		
		//Tried to assign values that aren't in sets of 2
		assert(newcount % 2 == 0);
		
		int newsize = origsize + newcount;
		if (newcount > 0)
		{
			texcoords[0].resize(newsize);
		}
		if (texcoords.size() != 0)
		{
			float * myarray = &(texcoords[0][0]);
			for (int i = 0; i < newcount; ++i)
			{
				myarray[i+origsize] = newtc[i];
			}
		}
	}
}

void VERTEXARRAY::Transform(const MATRIX4 <float> & m)
{
	MATRIX4 <float> mat = m;

	// rotate + translate vertices
	assert(vertices.size() % 3 == 0);
	for(unsigned int i = 0; i < vertices.size(); i+=3)
	{
		mat.TransformVectorOut(vertices[i], vertices[i+1], vertices[i+2]);
	}

	// rotate normals
	mat[12] = 0; mat[13] = 0; mat[14] = 0;
	assert(normals.size() % 3 == 0);
	for(unsigned int i = 0; i < normals.size(); i+=3)
	{
		mat.TransformVectorOut(normals[i], normals[i+1], normals[i+2]);
	}
}

void VERTEXARRAY::SetToBillboard(float x1, float y1, float x2, float y2)
{
	/*glNormal3f( 0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 0, -scale,  -scale);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 0, -scale,  scale);
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 0,  scale,  scale);
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 0,  scale,  -scale);*/
	
	int bfaces[6];
	bfaces[0] = 0;
	bfaces[1] = 1;
	bfaces[2] = 2;
	bfaces[3] = 0;
	bfaces[4] = 2;
	bfaces[5] = 3;
	SetFaces(bfaces, 6);
	
	float normals[12];
	for (int i = 0; i < 12; i+=3)
	{
		normals[i] = 0;
		normals[i+1] = 0;
		normals[i+2] = 1;
	}
	SetNormals(normals, 12);
	
	float verts[12];
	
	/*verts[0] = verts[3] = verts[6] = verts[9] = 0.0;
	verts[1] = verts[2] = verts[4] = verts[11] = -scale;
	verts[5] = verts[7] = verts[8] = verts[10] = scale;*/
	
	//build this:
	//x1, y1, 0		1   2
	//x2, y1, 0
	//x2, y2, 0		4   3
	//x1, y2, 0
	verts[2] = verts[5] = verts[8] = verts[11] = 0.0;
	verts[0] = verts[9] = x1;
	verts[3] = verts[6] = x2;
	verts[1] = verts[4] = y1;
	verts[7] = verts[10] = y2;
	SetVertices(verts, 12);
	
	float tc[8];
	tc[0] = tc[1] = tc[3] = tc[6] = 0.0;
	tc[2] = tc[4] = tc[5] = tc[7] = 1.0;
	SetTexCoordSets(1);
	SetTexCoords(0, tc, 8);
}

void VERTEXARRAY::SetBox4(float* ftx, float* fty)  //x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
	int bfaces[6];
	bfaces[0] = 0;	bfaces[1] = 1;	bfaces[2] = 2;
	bfaces[3] = 0;	bfaces[4] = 2;	bfaces[5] = 3;
	SetFaces(bfaces, 6);
	
	float normals[12];
	for (int i = 0; i < 12; i+=3)
	{
		normals[i] = 0;
		normals[i+1] = 0;
		normals[i+2] = 1;
	}
	SetNormals(normals, 12);
	
	float verts[12];
	verts[0] = ftx[0];	verts[1] = fty[0];	verts[2] = 0.0;
	verts[3] = ftx[1];	verts[4] = fty[1];	verts[5] = 0.0;
	verts[6] = ftx[2];	verts[7] = fty[2];	verts[8] = 0.0;
	verts[9] = ftx[3];	verts[10]= fty[3];	verts[11] = 0.0;
	//verts[0] = x1;	verts[1] = y1;	verts[2] = 0.0;
	//verts[3] = x2;	verts[4] = y2;	verts[5] = 0.0;
	//verts[6] = x3;	verts[7] = y3;	verts[8] = 0.0;
	//verts[9] = x4;	verts[10]= y4;	verts[11] = 0.0;
	SetVertices(verts, 12);
	
	float tc[8];
	tc[0] = tc[1] = tc[3] = tc[6] = 0.0;
	tc[2] = tc[4] = tc[5] = tc[7] = 1.0;
	SetTexCoordSets(1);
	SetTexCoords(0, tc, 8);
}

void VERTEXARRAY::SetTo2DQuad(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, float z)
{
	float vcorners[12];
	float uvs[8];
	int bfaces[6];
	SetVertexData2DQuad(x1,y1,x2,y2,u1,v1,u2,v2, vcorners, uvs, bfaces);
	for (int i = 2; i < 12; i += 3)
		vcorners[i] = z;
	SetFaces(bfaces, 6);
	SetVertices(vcorners, 12);
	SetTexCoordSets(1);
	SetTexCoords(0, uvs, 8);
}

void VERTEXARRAY::SetVertexData2DQuad(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, float * vcorners, float * uvs, int * bfaces, int faceoffset) const
{
	vcorners[0] = x1;
	vcorners[1] = y1;
	vcorners[2] = 0;
	vcorners[3] = x2;
	vcorners[4] = y1;
	vcorners[5] = 0;
	vcorners[6] = x2;
	vcorners[7] = y2;
	vcorners[8] = 0;
	vcorners[9] = x1;
	vcorners[10] = y2;
	vcorners[11] = 0;

	uvs[0] = u1;
	uvs[1] = v1;
	uvs[2] = u2;
	uvs[3] = v1;
	uvs[4] = u2;
	uvs[5] = v2;
	uvs[6] = u1;
	uvs[7] = v2;
	
	bfaces[0] = faceoffset+0;
	bfaces[1] = faceoffset+2;
	bfaces[2] = faceoffset+1;
	bfaces[3] = faceoffset+0;
	bfaces[4] = faceoffset+3;
	bfaces[5] = faceoffset+2;
}

void VERTEXARRAY::SetTo2DButton(float x, float y, float w, float h, float sidewidth, bool flip)
{
	float vcorners[12*3];
	float uvs[8*3];
	int bfaces[6*3];
			
	//y1 = 1.0 - y1;
	//y2 = 1.0 - y2;
	
	MATHVECTOR<float, 2> corner1;
	MATHVECTOR<float, 2> corner2;
	MATHVECTOR<float, 2> dim;
	dim.Set(w,h);
	MATHVECTOR<float, 2> center;
	center.Set(x,y);
	corner1 = center - dim*0.5;
	corner2 = center + dim*0.5;
	
	float x1 = corner1[0];
	float y1 = corner1[1];
	float x2 = corner2[0];
	float y2 = corner2[1];
	
	if (flip)
	{
		float y3 = y1;
		y1 = y2;
		y2 = y3;
	}
			
	//left
	SetVertexData2DQuad(x1-sidewidth,y1,x1,y2, 0,0,0.5,1, vcorners, uvs, bfaces);
			
	//center
	SetVertexData2DQuad(x1,y1,x2,y2, 0.5,0,0.5,1, &(vcorners[12]), &(uvs[8]), &(bfaces[6]), 4);
			
	//right
	SetVertexData2DQuad(x2,y1,x2+sidewidth,y2, 0.5,0,1,1, &(vcorners[12*2]), &(uvs[8*2]), &(bfaces[6*2]), 8);
			
	SetFaces(bfaces, 6*3);
	SetVertices(vcorners, 12*3);
	SetTexCoordSets(1);
	SetTexCoords(0, uvs, 8*3);
}

void VERTEXARRAY::SetTo2DBox(float x, float y, float w, float h, float marginwidth, float marginheight, float clipx)
{
	const unsigned int quads = 9;
	float vcorners[12*quads];
	float uvs[8*quads];
	int bfaces[6*quads];
			
	//y1 = 1.0 - y1;
	//y2 = 1.0 - y2;
	
	MATHVECTOR<float, 2> corner1;
	MATHVECTOR<float, 2> corner2;
	MATHVECTOR<float, 2> dim;
	dim.Set(w,h);
	MATHVECTOR<float, 2> center;
	center.Set(x,y);
	corner1 = center - dim*0.5;
	corner2 = center + dim*0.5;
	MATHVECTOR<float, 2> margin;
	margin.Set(marginwidth, marginheight);
	
	float lxmax = std::max((corner1-margin)[0],std::min(clipx,corner1[0]));
	float cxmax = std::max(corner1[0],std::min(clipx,corner2[0]));
	float rxmax = std::max(corner2[0],std::min(clipx,(corner2+margin)[0]));
	float lumax = (lxmax-(corner1-margin)[0])/(corner1[0]-(corner1-margin)[0])*0.5;
	float rumax = (rxmax-corner2[0])/((corner2+margin)[0]-corner2[0])*0.5+0.5;
	
	//upper left
	SetVertexData2DQuad((corner1-margin)[0],(corner1-margin)[1],lxmax,corner1[1],
			    0,0,lumax,0.5, vcorners,uvs,bfaces);
	
	//upper center
	SetVertexData2DQuad(corner1[0],(corner1-margin)[1],cxmax,corner1[1],
			     0.5,0,0.5,0.5,
			     &(vcorners[12*1]),&(uvs[8*1]),&(bfaces[6*1]),4*1);
	
	//upper right
	SetVertexData2DQuad(corner2[0],(corner1-margin)[1],rxmax,corner1[1],
			    0.5,0,rumax,0.5,
			    &(vcorners[12*2]),&(uvs[8*2]),&(bfaces[6*2]),4*2);
	
	//center left
	SetVertexData2DQuad((corner1-margin)[0],corner1[1],lxmax,corner2[1],
			    0,0.5,lumax,0.5,
			    &(vcorners[12*3]),&(uvs[8*3]),&(bfaces[6*3]),4*3);
	
	//center center
	SetVertexData2DQuad(corner1[0],corner1[1],cxmax,corner2[1],
			    0.5,0.5,0.5,0.5,
			    &(vcorners[12*4]),&(uvs[8*4]),&(bfaces[6*4]),4*4);
	
	//center right
	SetVertexData2DQuad(corner2[0],corner1[1],rxmax,corner2[1],
			    0.5,0.5,rumax,0.5,
			    &(vcorners[12*5]),&(uvs[8*5]),&(bfaces[6*5]),4*5);
	
	//lower left
	SetVertexData2DQuad((corner1-margin)[0],corner2[1],lxmax,(corner2+margin)[1],
			    0,0.5,lumax,1,
			    &(vcorners[12*6]),&(uvs[8*6]),&(bfaces[6*6]),4*6);
	
	//lower center
	SetVertexData2DQuad(corner1[0],corner2[1],cxmax,(corner2+margin)[1],
			    0.5,0.5,0.5,1,
			    &(vcorners[12*7]),&(uvs[8*7]),&(bfaces[6*7]),4*7);
	
	//lower right
	SetVertexData2DQuad(corner2[0],corner2[1],rxmax,(corner2+margin)[1],
			    0.5,0.5,rumax,1,
			    &(vcorners[12*8]),&(uvs[8*8]),&(bfaces[6*8]),4*8);
	
	SetFaces(bfaces, 6*quads);
	SetVertices(vcorners, 12*quads);
	SetTexCoordSets(1);
	SetTexCoords(0, uvs, 8*quads);
}

void VERTEXARRAY::BuildFromFaces(const std::vector <FACE> & newfaces)
{
	std::map <VERTEXDATA, unsigned int> indexmap;
	Clear();
	texcoords.resize(1);
	
	for (std::vector <FACE>::const_iterator i = newfaces.begin(); i != newfaces.end(); ++i) //loop through input triangles
	{
		for (int v = 0; v < 3; ++v) //loop through vertices in triangle
		{
			const VERTEXDATA & curvertdata = i->GetVertexData(v); //grab vertex
			std::map <VERTEXDATA, unsigned int>::iterator result = indexmap.find(curvertdata);
			if (result == indexmap.end()) //new vertex
			{
				unsigned int newidx = indexmap.size();
				indexmap[curvertdata] = newidx;
				
				vertices.push_back(curvertdata.vertex.x);
				vertices.push_back(curvertdata.vertex.y);
				vertices.push_back(curvertdata.vertex.z);
				
				normals.push_back(curvertdata.normal.x);
				normals.push_back(curvertdata.normal.y);
				normals.push_back(curvertdata.normal.z);
				
				texcoords[0].push_back(curvertdata.texcoord.u);
				texcoords[0].push_back(curvertdata.texcoord.v);
				
				faces.push_back(newidx);
			}
			else //non-unique vertex
				faces.push_back(result->second);
		}
	}
	
	//std::cout << faces.size() << ", " << newfaces.size() << ", " << vertices.size() << ", " << normals.size() << ", " << texcoords[0].size() << std::endl;
	
	assert(faces.size()/3 == newfaces.size());
	assert(vertices.size()/3 == normals.size()/3 && normals.size()/3 == texcoords[0].size()/2);
	assert(vertices.size()/3 <= faces.size());
}

void VERTEXARRAY::Translate(float x, float y, float z)
{
	assert(vertices.size() % 3 == 0);
	for (std::vector <float>::iterator i = vertices.begin(); i != vertices.end(); i += 3)
	{
		float * vert = &*i;
		vert[0] += x;
		vert[1] += y;
		vert[2] += z;
    }
}

void VERTEXARRAY::Rotate(float a, float x, float y, float z)
{
	QUATERNION<float> q;
	q.Rotate(a,x,y,z);
	
	assert(vertices.size() % 3 == 0);
	for (std::vector <float>::iterator i = vertices.begin(); i != vertices.end(); i += 3)
	{
		float * vert = &*i;
		q.RotateVector(vert);
    }
}

QT_TEST(vertexarray_buldfromfaces_test)
{	
	std::vector <VERTEXARRAY::TRIFLOAT> verts;
	verts.push_back(VERTEXARRAY::TRIFLOAT(0,0,0));
	verts.push_back(VERTEXARRAY::TRIFLOAT(1.0,-1.0,-1.0));
	verts.push_back(VERTEXARRAY::TRIFLOAT(1.0,-1.0,1.0));
	verts.push_back(VERTEXARRAY::TRIFLOAT(-1.0,-1.0,1.0));
	verts.push_back(VERTEXARRAY::TRIFLOAT(-1.0,-1.0,-1.0));
	verts.push_back(VERTEXARRAY::TRIFLOAT(1.0,1.0,-1.0));
	verts.push_back(VERTEXARRAY::TRIFLOAT(1.0,1.0,1.0));
	verts.push_back(VERTEXARRAY::TRIFLOAT(-1.0,1.0,1.0));
	verts.push_back(VERTEXARRAY::TRIFLOAT(-1.0,1.0,-1.0));
	
	std::vector <VERTEXARRAY::TRIFLOAT> norms;
	norms.push_back(VERTEXARRAY::TRIFLOAT(0,0,0));
	norms.push_back(VERTEXARRAY::TRIFLOAT(0.0,0.0,-1.0));
	norms.push_back(VERTEXARRAY::TRIFLOAT(-1.0,-0.0,-0.0));
	norms.push_back(VERTEXARRAY::TRIFLOAT(-0.0,-0.0,1.0));
	norms.push_back(VERTEXARRAY::TRIFLOAT(-0.0,0.0,1.0));
	norms.push_back(VERTEXARRAY::TRIFLOAT(1.0,-0.0,0.0));
	norms.push_back(VERTEXARRAY::TRIFLOAT(1.0,0.0,0.0));
	norms.push_back(VERTEXARRAY::TRIFLOAT(0.0,1.0,-0.0));
	norms.push_back(VERTEXARRAY::TRIFLOAT(-0.0,-1.0,0.0));
	
	std::vector <VERTEXARRAY::TWOFLOAT> texcoords(1);
	texcoords[0].u = 0;
	texcoords[0].v = 0;
	
	std::vector <VERTEXARRAY::FACE> cubesides;
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[5],norms[1],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[1],norms[1],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[8],norms[1],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[1],norms[1],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[4],norms[1],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[8],norms[1],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[3],norms[2],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[7],norms[2],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[8],norms[2],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[3],norms[2],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[8],norms[2],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[4],norms[2],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[2],norms[3],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[6],norms[3],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[3],norms[3],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[6],norms[4],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[7],norms[4],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[3],norms[4],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[1],norms[5],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[5],norms[5],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[2],norms[5],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[5],norms[6],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[6],norms[6],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[2],norms[6],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[5],norms[7],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[8],norms[7],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[7],norms[7],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[5],norms[7],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[7],norms[7],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[6],norms[7],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[1],norms[8],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[2],norms[8],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[3],norms[8],texcoords[0])));
	cubesides.push_back(VERTEXARRAY::FACE(VERTEXARRAY::VERTEXDATA(verts[1],norms[8],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[3],norms[8],texcoords[0]),VERTEXARRAY::VERTEXDATA(verts[4],norms[8],texcoords[0])));
	
	VERTEXARRAY varray;
	varray.BuildFromFaces(cubesides);
	
	const float * tempfloat(NULL);
	const int * tempint(NULL);
	int tempnum;
	
	varray.GetNormals(tempfloat, tempnum);
	QT_CHECK(tempfloat != NULL);
	QT_CHECK_EQUAL(tempnum,72);
	
	varray.GetVertices(tempfloat, tempnum);
	QT_CHECK(tempfloat != NULL);
	QT_CHECK_EQUAL(tempnum,72);
	
	QT_CHECK_EQUAL(varray.GetTexCoordSets(),1);
	
	varray.GetTexCoords(0, tempfloat, tempnum);
	QT_CHECK(tempfloat != NULL);
	QT_CHECK_EQUAL(tempnum,48);
	
	varray.GetFaces(tempint, tempnum);
	QT_CHECK(tempfloat != NULL);
	QT_CHECK_EQUAL(tempnum,36);
}

