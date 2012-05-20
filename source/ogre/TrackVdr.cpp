#include "pch.h"
#include "common/Defines.h"
#include "common/RenderConst.h"
#include "common/MaterialGen/MaterialFactory.h"
#include "CarModel.h" // for CreateModel()
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../vdrift/track.h"
#include "SplitScreen.h"  //-

#include <boost/filesystem.hpp>
//#include <Ogre.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreStaticGeometry.h>
#include <OgreRenderWindow.h>
#include <OgrePixelFormat.h>
#include <OgreTexture.h>
using namespace Ogre;


///---------------------------------------------------------------------------------------------------------------
///  track
///---------------------------------------------------------------------------------------------------------------

void App::CreateVdrTrack()
{	
	//  materials  -------------
	std::string sMatCache = pSet->game.track + ".matdef", sMatOrig = "_" + sMatCache,		sPathCache = PATHMANAGER::GetShaderCacheDir() + "/" + sMatCache, sPathOrig = TrkDir() +"objects/"+ sMatOrig;	bool hasMatOrig = boost::filesystem::exists(sPathOrig), hasMatCache = boost::filesystem::exists(sPathCache);	bool bGenerate = 0;  // set 1 to force generate for new vdrift tracks
	if (!hasMatOrig && !hasMatCache || bGenerate)
	{
		String sMtrs;
		for (int i=0; i < pGame->track.ogre_meshes.size(); i++)
		{
			OGRE_MESH& msh = pGame->track.ogre_meshes[i];
			if (msh.sky /*&& ownSky*/)  continue;			if (!msh.newMtr)  continue;  //  create material if new

			bool found = true;
			TexturePtr tex = TextureManager::getSingleton().getByName(msh.material);
			if (tex.isNull())
			try{
				tex = TextureManager::getSingleton().load(msh.material, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);  }
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
		}		std::ofstream fileout(sPathCache.c_str());
		if (!fileout)  LogO("Error: Can't save vdrift track matdef!");
		fileout.write(sMtrs.c_str(), sMtrs.size());
		fileout.close();
		hasMatCache = true;
	}

	//  load .matdef
	LogO(String("Vdrift track .matdef  has Cache:")+ (hasMatCache?"yes":"no") + "  has Orig:" + (hasMatOrig?"yes":"no"));
	materialFactory->loadDefsFromFile(hasMatCache ? sMatCache : sMatOrig);
	materialFactory->generate(true);
	

	//  meshes  -------------
	//LogManager::getSingletonPtr()->logMessage( String("---------models----  ogre:") +
	//	toStr(pGame->track.ogre_meshes.size()) + " mod_lib:" + toStr(pGame->track.model_library.size()) );
	std::vector<Entity*> ents;
	for (int i=0; i < pGame->track.ogre_meshes.size(); i++)
	{
		OGRE_MESH& msh = pGame->track.ogre_meshes[i];
		if (msh.sky /*&& ownSky*/)  continue;
		if (!msh.found)  continue;

		//if (strstr(msh.material.c_str(), "tree")!=0)  continue;

		//LogManager::getSingletonPtr()->logMessage( String("---  model: ") +
		//	msh.name + " mtr:" + msh.material +
		//" v:" + toStr(msh.mesh->vertices.size()) + " f:" + toStr(msh.mesh->faces.size()) );

		//if (ownSky && msh.sky)
		if (!msh.sky)
		{
		ManualObject* m = CarModel::CreateModel(mSceneMgr, msh.material, msh.mesh, Vector3(0,0,0), false, true);
		//if (!m)  continue;
		if (msh.sky)
			m->setCastShadows(false);
		
		MeshPtr mp = m->convertToMesh("m"+toStr(i));
		Entity* e = mSceneMgr->createEntity(mp);

		ents.push_back(e);
		}
	}

	//  static geom  -------------
	StaticGeometry *sg = mSceneMgr->createStaticGeometry("track");
	sg->setRegionDimensions(Vector3::UNIT_SCALE * 1000);  // 1000
	sg->setOrigin(Vector3::ZERO);
	sg->setCastShadows(true);

	for (std::vector<Entity*>::iterator it = ents.begin(); it != ents.end(); ++it)
		sg->addEntity(*it, Vector3::ZERO);

	sg->build();
	//sg->dump("_track-sg.txt");
}



///---------------------------------------------------------------------------------------------------------------
//	 track racing line
///---------------------------------------------------------------------------------------------------------------

void App::CreateRacingLine()
{
	//void ROADPATCH::AddRacinglineScenenode(SCENENODE * node, ROADPATCH * nextpatch,	
	ManualObject* m = mSceneMgr->createManualObject();
	m->begin("track/Racingline", RenderOperation::OT_TRIANGLE_LIST);
	int ii = 0;

	const std::list <ROADSTRIP>& roads = pGame->track.GetRoadList();
	for (std::list <ROADSTRIP>::const_iterator it = roads.begin(); it != roads.end(); ++it)
	{
		const std::list <ROADPATCH>& pats = (*it).GetPatchList();
		for (std::list <ROADPATCH>::const_iterator i = pats.begin(); i != pats.end(); ++i)
		{
			const VERTEXARRAY* a = &((*i).racingline_vertexarray);
			if (!a)  continue;

			int verts = a->vertices.size();
			if (verts == 0)  continue;
			int faces = a->faces.size();

			for (int v = 0; v < verts; v += 3)
				m->position(a->vertices[v+0], a->vertices[v+2], -a->vertices[v+1]);

			for (int f = 0; f < faces; ++f)
				m->index(ii + a->faces[f]);

			ii += verts/3;
		}
	}
	m->setCastShadows(false);	
	m->end();
	ndLine = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ndLine->attachObject(m);
	//ndLine->setVisible(pSet->racingline);
}
		


//---------------------------------------------------------------------------------------------------------------
///	 track 2D minimap  -mesh, optym texture..
//---------------------------------------------------------------------------------------------------------------

void App::CreateMinimap()
{
	asp = float(mWindow->getWidth())/float(mWindow->getHeight());

	//  get track sizes
	minX=FLT_MAX; maxX=FLT_MIN;  minY=FLT_MAX; maxY=FLT_MIN;

	const std::list <ROADSTRIP>& roads = pGame->track.GetRoadList();
	for (std::list <ROADSTRIP>::const_iterator it = roads.begin(); it != roads.end(); ++it)
	{
		const std::list <ROADPATCH>& pats = (*it).GetPatchList();
		for (std::list <ROADPATCH>::const_iterator i = pats.begin(); i != pats.end(); ++i)
		{
			for (int iy=0; iy<4; ++iy)
			for (int ix=0; ix<4; ++ix)
			{
				const MATHVECTOR <float,3>& vec = (*i).GetPatch().GetPoint(ix,iy);

				Real x = vec[0], y = vec[2];
				if (x < minX)  minX = x;	if (x > maxX)  maxX = x;
				if (y < minY)  minY = y;	if (y > maxY)  maxY = y;
			}
		}
	}

	float fMapSizeX = maxX - minX, fMapSizeY = maxY - minY;  // map size
	float size = std::max(fMapSizeX, fMapSizeY);
	scX = 1.f / size;  scY = 1.f / size;

	ManualObject* m = mSceneMgr->createManualObject();
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
				const MATHVECTOR <float,3>& vec = (*i).GetPatch().GetPoint(x,y);
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

				Real c = std::min(1.f, std::max(0.3f, 1.f - 2.4f * powf( abs(norm.y)
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
	AxisAlignedBox aabInf;	aabInf.setInfinite();  m->setBoundingBox(aabInf);  // draw always
	m->setRenderingDistance(100000.f);
	m->setRenderQueueGroup(RQG_Hud2);  m->setVisibilityFlags(RV_Hud);

	float fHudSize = pSet->size_minimap;
	float marg = 1.f + 0.1f;  // from border
	float fMiniX = 1 - fHudSize * marg, fMiniY = 1 - fHudSize*asp * marg;

	int plr = mSplitMgr->mNumViewports;
	for (int i=0; i < plr; ++i)
	{
		ndMap[i] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(fMiniX,fMiniY,0));
		ndMap[i]->scale(fHudSize, fHudSize*asp, 1);
		ndMap[i]->attachObject(m);
	}
	
	//  car pos tri
	/*int plr = mSplitMgr->mNumViewports;
	for (int c=0; c < plr; ++c)
	{
		vMoPos[0][c] = Create2D("hud/CarPos", mSplitMgr->mGuiSceneMgr, 0.4f, true, true);
		vNdPos[0][c] = ndMap[0]->createChildSceneNode();
		vNdPos[0][c]->scale(fHudSize*1.5f, fHudSize*1.5f, 1);
		vNdPos[0][c]->attachObject(vMoPos[0][c]);  vNdPos[0][c]->setVisible(false);
	}
	ndMap[0]->setVisible(pSet->trackmap);*/
}



//---------------------------------------------------------------------------------------------------------------
///  3D bezier road  /test
//---------------------------------------------------------------------------------------------------------------

void App::CreateRoadBezier()
{
	ManualObject* m = mSceneMgr->createManualObject();
	m->begin("road", RenderOperation::OT_TRIANGLE_LIST);
	int ii=0;

	const std::list <ROADSTRIP>& roads = pGame->track.GetRoadList();
	for (std::list <ROADSTRIP>::const_iterator it = roads.begin(); it != roads.end(); ++it)
	{
		const std::list <ROADPATCH>& pats = (*it).GetPatchList();
		for (std::list <ROADPATCH>::const_iterator i = pats.begin(); i != pats.end(); ++i)
		{
			float p[16][3];  int a=0;
			for (int y=0; y<4; ++y)
			for (int x=0; x<4; ++x)
			{
				const MATHVECTOR <float, 3>& vec = (*i).GetPatch().GetPoint(x,y);
				p[a][0] = vec[2];  p[a][1] = vec[1];  p[a][2] = -vec[0];  a++;

				//float fx = (x+1)/4.f, fy = (y+1)/4.f;
				//const float s = 0.82f;
			}
			a=0;

			// normal
			Vector3 pos (p[a  ][0], p[a  ][1], p[a  ][2]);
			Vector3 posX(p[a+3][0], p[a+3][1], p[a+3][2]);   posX-=pos;  posX.normalise();
			Vector3 posY(p[a+12][0],p[a+12][1],p[a+12][2]);  posY-=pos;  posY.normalise();
			Vector3 norm = posX.crossProduct(posY);  norm.normalise();/**/

			for (int y=0; y<4; ++y)
			for (int x=0; x<4; ++x)
			{
				Vector3 pos(p[a][0], p[a][1], p[a][2]);  a++;
				m->position(pos);
				m->normal(norm);/**/
				m->textureCoord(y/3.f,x/3.f);
				if (x<3 && y<3)
				{
					int a = ii+x+y*4;
					m->index(a);	m->index(a+1);	m->index(a+4);
					m->index(a+5);	m->index(a+4);	m->index(a+1);
				}
			}
			ii += 16;
		}
	}
	m->end();
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(m);
}


