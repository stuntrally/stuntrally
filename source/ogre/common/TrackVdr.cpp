#include "pch.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "../../vdrift/track.h"
#include "GuiCom.h"
#include "CScene.h"
#ifndef SR_EDITOR
	#include "../CGame.h"
	#include "../CHud.h"
	#include "../../vdrift/game.h"
	#include "../SplitScreen.h"  //-
#else
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
	#include "../../editor/settings.h"
	#include "../../vdrift/pathmanager.h"
#endif
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <boost/filesystem.hpp>
#include <Ogre.h> //!
using namespace Ogre;


///---------------------------------------------------------------------------------------------------------------
///  track
///---------------------------------------------------------------------------------------------------------------

void App::CreateVdrTrack(std::string strack, TRACK* pTrack)
{	
	//  materials  -------------
	std::vector<OGRE_MESH>& meshes = pTrack->ogre_meshes;
	std::string sMatCache = strack + ".matdef", sMatOrig = "_" + sMatCache,
		sPathCache = PATHMANAGER::ShaderDir() + "/" + sMatCache, sPathOrig = gcom->TrkDir() +"objects/"+ sMatOrig;
	bool hasMatOrig = boost::filesystem::exists(sPathOrig), hasMatCache = boost::filesystem::exists(sPathCache);
	bool bGenerate = 0, gen = !hasMatOrig && !hasMatCache || bGenerate;  // set 1 to force generate for new vdrift tracks


	//TODO .mat ..rewrite this code for new system
#if 0
	if (gen)
	{
		String sMtrs;
		for (int i=0; i < meshes.size(); ++i)
		{
			OGRE_MESH& msh = meshes[i];
			if (msh.sky /*&& ownSky*/)  continue;
			if (!msh.newMtr)  continue;  //  create material if new

			bool found = true;
			TexturePtr tex = TextureManager::getSingleton().getByName(msh.material);
			if (tex.isNull())
			try{
				tex = TextureManager::getSingleton().load(msh.material, rgDef);  }
			catch(...){
				found = false;  }
			msh.found = found;  // dont create meshes for not found textures, test
			if (!found)  continue;

			#if 0  // use 0 for some tracks (eg.zandvoort) - have alpha textures for all!
			if (!tex.isNull() && tex->hasAlpha())
				msh.alpha = true;  // for textures that have alpha
			#endif

			if (msh.alpha)
				sMtrs += "["+msh.material+"]\n"+
					"	parent = 0vdrAlpha\n"+
					"	diffuseMap_512 = "+msh.material+"\n";
			else
				sMtrs += "["+msh.material+"]\n"+
					"	parent = 0vdrTrk\n"+
					"	diffuseMap_512 = "+msh.material+"\n";
		}

		std::ofstream fileout(sPathCache.c_str());
		if (!fileout)  LogO("Error: Can't save vdrift track matdef!");
		fileout.write(sMtrs.c_str(), sMtrs.size());
		fileout.close();
		hasMatCache = true;
	}
#endif
	

	//  meshes  -------------
	std::vector<Entity*> ents;
	static int ii = 0;  int i;
	for (i=0; i < meshes.size(); ++i)
	{
		OGRE_MESH& msh = meshes[i];
		if (msh.sky /*&& ownSky*/)  continue;
		if (!msh.found)  continue;

		//if (strstr(msh.material.c_str(), "tree")!=0)  continue;

		//LogO( String("---  model: ") + msh.name + " mtr:" + msh.material +
		//" v:" + toStr(msh.mesh->vertices.size()) + " f:" + toStr(msh.mesh->faces.size()) );

		//if (ownSky && msh.sky)
		if (!msh.sky)
		{
		ManualObject* m = CreateModel(mSceneMgr, msh.material, msh.mesh, Vector3(0,0,0), false, true);
		//if (!m)  continue;
		if (msh.sky)
			m->setCastShadows(false);
		
		MeshPtr mp = m->convertToMesh("m"+toStr(ii+i));
		Entity* e = mSceneMgr->createEntity(mp);

		ents.push_back(e);
		}
	}
	ii += i;

	//  static geom  -------------
	scn->vdrTrack = mSceneMgr->createStaticGeometry("track");
	scn->vdrTrack->setRegionDimensions(Vector3::UNIT_SCALE * 1000);  // 1000
	scn->vdrTrack->setOrigin(Vector3::ZERO);
	scn->vdrTrack->setCastShadows(true);

	for (std::vector<Entity*>::iterator it = ents.begin(); it != ents.end(); ++it)
		scn->vdrTrack->addEntity(*it, Vector3::ZERO);

	scn->vdrTrack->build();
	//mStaticGeom->dump("_track-sg.txt");
}


#ifndef SR_EDITOR

//---------------------------------------------------------------------------------------------------------------
///	 track 2D minimap
//---------------------------------------------------------------------------------------------------------------

ManualObject* CHud::CreateVdrMinimap()
{
	asp = float(app->mWindow->getWidth())/float(app->mWindow->getHeight());

	//  get track sizes
	minX=FLT_MAX; maxX=FLT_MIN;  minY=FLT_MAX; maxY=FLT_MIN;

	const std::list <ROADSTRIP>& roads = app->pGame->track.GetRoadList();
	for (std::list <ROADSTRIP>::const_iterator it = roads.begin(); it != roads.end(); ++it)
	{
		const std::list <ROADPATCH>& pats = (*it).GetPatchList();
		for (std::list <ROADPATCH>::const_iterator i = pats.begin(); i != pats.end(); ++i)
		{
			for (int iy=0; iy<4; ++iy)
			for (int ix=0; ix<4; ++ix)
			{
				const MATHVECTOR<float,3>& vec = (*i).GetPatch().GetPoint(ix,iy);

				Real x = vec[0], y = vec[2];
				if (x < minX)  minX = x;	if (x > maxX)  maxX = x;
				if (y < minY)  minY = y;	if (y > maxY)  maxY = y;
			}
		}
	}

	float fMapSizeX = maxX - minX, fMapSizeY = maxY - minY;  // map size
	float size = std::max(fMapSizeX, fMapSizeY);
	scX = 1.f / size;  scY = 1.f / size;

	ManualObject* m = app->mSceneMgr->createManualObject();
	m->begin("hud/Minimap", RenderOperation::OT_TRIANGLE_LIST);
	int ii = 0;

	for (std::list <ROADSTRIP>::const_iterator it = roads.begin(); it != roads.end(); ++it)
	{
		const std::list <ROADPATCH>& pats = (*it).GetPatchList();
		for (std::list <ROADPATCH>::const_iterator i = pats.begin(); i != pats.end(); ++i)
		{
			float p[16][3];  int a=0;
			for (int y=0; y<4; ++y)
			for (int x=0; x<4; ++x)
			{
				const MATHVECTOR<float,3>& vec = (*i).GetPatch().GetPoint(x,y);
				p[a][0] = vec[0];  p[a][1] = vec[2];  p[a][2] = vec[1];  a++;
			}
			a = 0;

			// normal
			Vector3 pos (p[a  ][2], -p[a  ][0], p[a  ][1]);
			Vector3 posX(p[a+3][2], -p[a+3][0], p[a+3][1]);   posX-=pos;  posX.normalise();
			Vector3 posY(p[a+12][2],-p[a+12][0],p[a+12][1]);  posY-=pos;  posY.normalise();
			Vector3 norm = posX.crossProduct(posY);  norm.normalise();/**/

			for (int y=0; y<4; ++y)
			for (int x=0; x<4; ++x)
			{
				Vector3 pos( (p[a][0] - minX)*scX*2-1,	// pos x,y = -1..1
							-(p[a][1] - minY)*scY*2+1, 0);  a++;
				m->position(pos);
				m->normal(norm);/**/

				Real c = std::min(1.f, std::max(0.3f, 1.f - 2.4f * powf( fabs(norm.y)
					/*norm.absDotProduct(vLi)*/, 0.7f) ));
				m->colour(ColourValue(c,c,c,1));

				m->textureCoord(x/3.f,y/3.f);
				if (x<3 && y<3)
				{
					int a = ii+x+y*4;
					m->index(a+0);	m->index(a+1);	m->index(a+4);
					m->index(a+1);	m->index(a+4);	m->index(a+5);
				}
			}
			ii += 16;
		}
	}
	m->end();
	m->setUseIdentityProjection(true);  m->setUseIdentityView(true);  // on hud
	m->setCastShadows(false);
	AxisAlignedBox aab;  aab.setInfinite();  m->setBoundingBox(aab);  // draw always
	m->setRenderingDistance(100000.f);
	m->setRenderQueueGroup(RQG_Hud2);  m->setVisibilityFlags(RV_Hud);
	return m;
}
#endif


//  utility - create VDrift model in Ogre
//-------------------------------------------------------------------------------------------------------
ManualObject* App::CreateModel(SceneManager* sceneMgr, const String& mat,
	class VERTEXARRAY* a, Vector3 vPofs, bool flip, bool track, const String& name)
{
	int verts = a->vertices.size();
	if (verts == 0)  return NULL;
	int tcs   = a->texcoords[0].size(); //-
	int norms = a->normals.size();
	int faces = a->faces.size();
	// norms = verts, verts % 3 == 0

	ManualObject* m;
	if (name == "")
		m = sceneMgr->createManualObject();
	else
		m = sceneMgr->createManualObject(name);
	m->begin(mat, RenderOperation::OT_TRIANGLE_LIST);

	int t = 0;
	if (track)
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(a->vertices[v+0], a->vertices[v+2], -a->vertices[v+1]);
			if (norms)
			m->normal(	a->normals [v+0], a->normals [v+2], -a->normals [v+1]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; ++f)
			m->index(a->faces[f]);
	}else
	if (flip)
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(a->vertices[v], a->vertices[v+1], a->vertices[v+2]);
			if (norms)
			m->normal(  a->normals [v], a->normals [v+1], a->normals [v+2]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; f += 3)
		{	m->index(a->faces[f+2]);  m->index(a->faces[f+1]);  m->index(a->faces[f]);	}
	}else
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(-a->vertices[v+1]+vPofs.x, -a->vertices[v+2]+vPofs.y, a->vertices[v]+vPofs.z);
			if (norms)
			m->normal(	-a->normals [v+1], -a->normals [v+2], a->normals [v]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; f += 3)
		{	m->index(a->faces[f+2]);  m->index(a->faces[f+1]);  m->index(a->faces[f]);	}
	}
	m->end();
	return m;
}


//----------------------------------------------------
bool App::IsVdrTrack()
{
	//  vdrift track has roads.trk
	String svdr = gcom->TrkDir()+"roads.trk";
	bool vdr = boost::filesystem::exists(svdr);
	return vdr;
}


#ifdef SR_EDITOR
btIndexedMesh GetIndexedMesh(const MODEL & model)
{
	const float * vertices;  int vcount;
	const int * faces;  int fcount;
	model.GetVertexArray().GetVertices(vertices, vcount);
	model.GetVertexArray().GetFaces(faces, fcount);
	
	assert(fcount % 3 == 0); //Face count is not a multiple of 3
	
	btIndexedMesh mesh;
	mesh.m_numTriangles = fcount / 3;
	mesh.m_triangleIndexBase = (const unsigned char *)faces;
	mesh.m_triangleIndexStride = sizeof(int) * 3;
	mesh.m_numVertices = vcount;
	mesh.m_vertexBase = (const unsigned char *)vertices;
	mesh.m_vertexStride = sizeof(float) * 3;
	mesh.m_vertexType = PHY_FLOAT;
	return mesh;
}

void App::DestroyVdrTrackBlt()
{
	//  remove old track
	if (trackObject)
	{
		world->removeCollisionObject(trackObject);
		
		delete trackObject->getCollisionShape();
		trackObject->setCollisionShape(NULL);
		
		delete trackObject;
		trackObject = NULL;
		
		delete trackMesh;
		trackMesh = NULL;
	}
}

void App::CreateVdrTrackBlt()
{
	DestroyVdrTrackBlt();
	
	//  setup new track
	trackMesh = new btTriangleIndexVertexArray();
	const std::list<TRACK_OBJECT> & objects = track->GetTrackObjects();
	for (std::list<TRACK_OBJECT>::const_iterator ob = objects.begin(); ob != objects.end(); ++ob)
	{
		//if (ob->GetSurface() != NULL)
		{
			MODEL & model = *ob->GetModel();
			btIndexedMesh mesh = GetIndexedMesh(model);
			trackMesh->addIndexedMesh(mesh);
		}
	}
	
	btCollisionShape * trackShape = new btBvhTriangleMeshShape(trackMesh, false);
	trackObject = new btCollisionObject();
	trackObject->setCollisionShape(trackShape);
	trackObject->setUserPointer(NULL);
	
	world->addCollisionObject(trackObject);
}
#endif
