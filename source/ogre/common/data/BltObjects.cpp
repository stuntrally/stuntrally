#include "pch.h"
#include "../Def_Str.h"
#include "BltObjects.h"
#include "../../../vdrift/pathmanager.h"
#include "tinyxml2.h"
using namespace tinyxml2;


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

BltObjects::BltObjects()
{	}


//  Load  collision objects from xml
///--------------------------------------------------------------------------------------------------------------------------------------

bool BltObjects::LoadXml()
{
	colNone.clear();
	colsMap.clear();
	colsMapFind = colsMap.end();
	
	std::string name = "/trees/collisions.xml",  // user
		file = PATHMANAGER::DataUser() + name;
	if (!PATHMANAGER::FileExists(file))  // original
		file = PATHMANAGER::Data() + name;

	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;
	
	//  collisions
	XMLElement *n, *m;  const char* a;
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
			a = n->Attribute("type");  if (a)  {  std::string st = a;
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

	//  default params
	defPars.friction = 0.2f;
	defPars.restitution = 0.9f;
	n = root->FirstChildElement("default");
	if (n)
	{
		a = n->Attribute("frict");	if (a)  defPars.friction = s2r(a);
		a = n->Attribute("restit");	if (a)  defPars.restitution = s2r(a);
	}
	
	//  no collision
	n = root->FirstChildElement("none");
	if (n)
	{	m = n->FirstChildElement("object");
		while (m)
		{
			a = m->Attribute("mesh");
			if (a)
				colNone[std::string(a)] = true;
			m = m->NextSiblingElement("object");
		}
	}
	return true;
}


//  util  find col for mesh name
const BltCollision* BltObjects::Find(std::string mesh)
{
	colsMapFind = colsMap.find(mesh);
	bool hasCol = colsMapFind != colsMap.end();
	if (!hasCol)
		return NULL;
	else
		return &(colsMapFind->second);
}
