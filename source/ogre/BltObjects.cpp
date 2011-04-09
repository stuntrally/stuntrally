#include "stdafx.h"
#include "BltObjects.h"
#include "../vdrift/pathmanager.h"
using namespace Ogre;


BltShape::BltShape()
	:type(BLT_Sphere)
	,radius(1.f), height(1.f)
	,offset(0,0,0) //,rot(Degree;
	,friction(0.2f), restitution(0.9f)
{
	//rot = Quaternion::IDENTITY;
}

BltCollision::BltCollision()
{
	mesh = "";
	//shapes
}


//  Load  collision objects from xml
///--------------------------------------------------------------------------------------------------------------------------------------

bool BltObjects::LoadXml()
{
	//cols.clear();
	colsMap.clear();
	colsMapFind = colsMap.end();
	
	string file = PATHMANAGER::GetTreesPath() + "/collisions.xml";
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;
	
	//  collisions
	TiXmlElement *n, *m;  const char* a;
	m = root->FirstChildElement("object");
	while (m)
	{
		BltCollision col;
		a = m->Attribute("mesh");	if (a)  col.mesh = a;
		a = m->Attribute("ofs");	if (a)  col.offset = s2v(a);

		//  shapes
		n = m->FirstChildElement("shape");
		while (n)
		{
			BltShape shp;
			a = n->Attribute("type");  if (a)  {  string st = a;
			for (int t=0; t < BLT_ALL; ++t)
				if (st == sBLTshape[t])  shp.type = (eBLTshape)t;  }

			a = n->Attribute("ofs");	if (a)  shp.offset = s2v(a);
			a = n->Attribute("r");		if (a)  shp.radius = s2r(a);
			a = n->Attribute("h");		if (a)  shp.height = s2r(a);

			a = n->Attribute("frict");	if (a)  shp.friction = s2r(a);
			a = n->Attribute("restit");	if (a)  shp.restitution = s2r(a);
					
			//  add shape
			col.shapes.push_back(shp);
			n = n->NextSiblingElement("shape");
		}

		//  add collision
		//cols.push_back(col);
		colsMap[col.mesh] = col;  // map
		m = m->NextSiblingElement("object");
	}
	return true;
}


//  util  find col for mesh name
const BltCollision* BltObjects::Find(string mesh)
{
	colsMapFind = colsMap.find(mesh);
	bool hasCol = colsMapFind != colsMap.end();
	if (!hasCol)
		return NULL;
	else
		return &(colsMapFind->second);
}
