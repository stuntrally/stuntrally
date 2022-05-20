#include "pch.h"
#include "../Def_Str.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "tinyxml.h"
#include "tinyxml2.h"
#include <OgreSceneNode.h>
#include "../vdrift/game.h"  // for surfaces map
using namespace std;
using namespace Ogre;
using namespace tinyxml2;


//  Save
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::SaveXml(String file)
{
	TiXmlDocument xml;	TiXmlElement root("scene");

	TiXmlElement ver("ver");
		int v = SET_VER;
		ver.SetAttribute("num",		toStrC( v ));
		ver.SetAttribute("baseTrk",	baseTrk.c_str());
		ver.SetAttribute("secEd",	toStrC( secEdited ));
	root.InsertEndChild(ver);


	TiXmlElement car("car");
		car.SetAttribute("tires",	asphalt ? "1":"0");
		if (damageMul != 1.f)
			car.SetAttribute("damage",	toStrC( damageMul ));
		if (!td.road1mtr)
			car.SetAttribute("road1mtr", td.road1mtr ? "1":"0");
		if (noWrongChks)
			car.SetAttribute("noWrongChks", noWrongChks ? "1":"0");

		if (denyReversed)
			car.SetAttribute("denyRev",	"1");
		if (gravity != 9.81f)
			car.SetAttribute("gravity",	toStrC( gravity ));
	root.InsertEndChild(car);


	TiXmlElement st("start");
		string s = toStr(startPos[0][0])+" "+toStr(startPos[0][1])+" "+toStr(startPos[0][2]);
		st.SetAttribute("pos",	s.c_str());

		s = toStr(startRot[0][0])+" "+toStr(startRot[0][1])+" "+toStr(startRot[0][2])+" "+toStr(startRot[0][3]);
		st.SetAttribute("rot",	s.c_str());

		s = toStr(startPos[1][0])+" "+toStr(startPos[1][1])+" "+toStr(startPos[1][2]);
		st.SetAttribute("pos2",	s.c_str());

		s = toStr(startRot[1][0])+" "+toStr(startRot[1][1])+" "+toStr(startRot[1][2])+" "+toStr(startRot[1][3]);
		st.SetAttribute("rot2",	s.c_str());
	root.InsertEndChild(st);


	TiXmlElement snd("sound");
		snd.SetAttribute("ambient",		sAmbient.c_str());
		snd.SetAttribute("reverbs",		sReverbs.c_str());
	root.InsertEndChild(snd);
	

	TiXmlElement sky("sky");
		sky.SetAttribute("material",	skyMtr.c_str());
		if (rainEmit > 0 && rainName != "")
		{	sky.SetAttribute("rainName",	rainName.c_str());
			sky.SetAttribute("rainEmit",	toStrC( rainEmit ));
		}
		if (rain2Emit > 0 && rain2Name != "")
		{	sky.SetAttribute("rain2Name",	rain2Name.c_str());
			sky.SetAttribute("rain2Emit",	toStrC( rain2Emit ));
		}
		if (windAmt != 0.f)
			sky.SetAttribute("windAmt",	toStrC( windAmt ));
		if (skyYaw != 0.f)
			sky.SetAttribute("skyYaw",	toStrC( skyYaw ));
	root.InsertEndChild(sky);

	TiXmlElement fog("fog");
		fog.SetAttribute("color",		fogClr.Save().c_str() );
		fog.SetAttribute("color2",		fogClr2.Save().c_str() );
		fog.SetAttribute("linStart",	toStrC( fogStart ));
		fog.SetAttribute("linEnd",		toStrC( fogEnd ));
	root.InsertEndChild(fog);

	TiXmlElement fogH("fogH");
		fogH.SetAttribute("color",		fogClrH.Save().c_str() );
		fogH.SetAttribute("height",		toStrC( fogHeight ));
		fogH.SetAttribute("dens",		toStrC( fogHDensity ));
		fogH.SetAttribute("linStart",	toStrC( fogHStart ));
		fogH.SetAttribute("linEnd",		toStrC( fogHEnd ));
		if (fHDamage > 0.f)
			fogH.SetAttribute("dmg",	toStrC( fHDamage ));
	root.InsertEndChild(fogH);

	TiXmlElement li("light");
		li.SetAttribute("pitch",		toStrC( ldPitch ));
		li.SetAttribute("yaw",			toStrC( ldYaw ));
		li.SetAttribute("ambient",		lAmb.Save().c_str() );
		li.SetAttribute("diffuse",		lDiff.Save().c_str() );
		li.SetAttribute("specular",		lSpec.Save().c_str() );
	root.InsertEndChild(li);
	

	TiXmlElement fls("fluids");
		for (int i=0; i < fluids.size(); ++i)
		{
			const FluidBox* fb = &fluids[i];
			TiXmlElement fe("fluid");
			fe.SetAttribute("name",		fb->name.c_str() );
			fe.SetAttribute("pos",		toStrC( fb->pos ));
			fe.SetAttribute("rot",		toStrC( fb->rot ));
			fe.SetAttribute("size",		toStrC( fb->size ));
			fe.SetAttribute("tile",		toStrC( fb->tile ));
			fls.InsertEndChild(fe);
		}
	root.InsertEndChild(fls);


	TiXmlElement ter("terrain");
		ter.SetAttribute("size",		toStrC( td.iVertsX ));
		ter.SetAttribute("triangle",	toStrC( td.fTriangleSize ));
		ter.SetAttribute("errNorm",		fToStr( td.errorNorm, 2,4 ).c_str());
		if (td.normScale != 1.f)
			ter.SetAttribute("normSc",		toStrC( td.normScale ));
		if (td.emissive)
			ter.SetAttribute("emissive",	td.emissive ? 1 : 0);
		if (td.specularPow != 32.f)
			ter.SetAttribute("specPow",		toStrC( td.specularPow ));
		if (td.specularPowEm != 2.f)
			ter.SetAttribute("specPowEm",	toStrC( td.specularPowEm ));

		const TerLayer* l;
		for (int i=0; i < 6; ++i)
		{
			l = &td.layersAll[i];
			TiXmlElement tex("texture");
			tex.SetAttribute("on",		l->on ? 1 : 0);
			tex.SetAttribute("file",	l->texFile.c_str());
			tex.SetAttribute("fnorm",	l->texNorm.c_str());
			tex.SetAttribute("scale",	toStrC( l->tiling ));
			tex.SetAttribute("surf",	l->surfName.c_str());
			#define setDmst()  \
				tex.SetAttribute("dust",	toStrC( l->dust ));  \
				tex.SetAttribute("dustS",	toStrC( l->dustS )); \
				tex.SetAttribute("mud",		toStrC( l->mud ));   \
				tex.SetAttribute("smoke",	toStrC( l->smoke )); \
				tex.SetAttribute("tclr",	l->tclr.Save().c_str() );
			setDmst();
			if (l->fDamage > 0.f)
				tex.SetAttribute("dmg",	toStrC( l->fDamage ));

			tex.SetAttribute("angMin",	toStrC( l->angMin ));
			tex.SetAttribute("angMax",	toStrC( l->angMax ));
			tex.SetAttribute("angSm",	toStrC( l->angSm ));
			tex.SetAttribute("hMin",	toStrC( l->hMin ));
			tex.SetAttribute("hMax",	toStrC( l->hMax ));
			tex.SetAttribute("hSm",		toStrC( l->hSm ));

			tex.SetAttribute("nOn",		l->nOnly ? 1 : 0);
			if (l->triplanar)  tex.SetAttribute("triplanar", 1);

			tex.SetAttribute("noise",	toStrC( l->noise ));
			tex.SetAttribute("n_1",		toStrC( l->nprev ));
			tex.SetAttribute("n2",		toStrC( l->nnext2 ));

			TiXmlElement noi("noise");
			for (int n=0; n < 2; ++n)
			{	string sn = toStr(n), s;
				s = "frq"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nFreq[n] ));
				s = "oct"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nOct[n] ));
				s = "prs"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nPers[n] ));
				s = "pow"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nPow[n] ));
			}
			tex.InsertEndChild(noi);
			ter.InsertEndChild(tex);
		}
		for (int i=0; i < 4; ++i)
		{
			l = &td.layerRoad[i];
			TiXmlElement tex("texture");
			tex.SetAttribute("road",	toStrC(i+1));
			tex.SetAttribute("surf",	l->surfName.c_str());
			setDmst();
			ter.InsertEndChild(tex);
		}
		
		TiXmlElement par("par");
			par.SetAttribute("dust",	sParDust.c_str());
			par.SetAttribute("mud",		sParMud.c_str());
			par.SetAttribute("smoke",	sParSmoke.c_str());
		ter.InsertEndChild(par);

	root.InsertEndChild(ter);
	

	TiXmlElement pgd("paged");
		pgd.SetAttribute("densGrass",	toStrC( densGrass ));
		pgd.SetAttribute("densTrees",	toStrC( densTrees ));
		//  grass
		pgd.SetAttribute("grPage",		toStrC( grPage ));
		pgd.SetAttribute("grDist",		toStrC( grDist ));
		pgd.SetAttribute("grDensSmooth",toStrC( grDensSmooth ));

		//  trees
		pgd.SetAttribute("trPage",		toStrC( trPage ));
		pgd.SetAttribute("trDist",		toStrC( trDist ));
		pgd.SetAttribute("trDistImp",	toStrC( trDistImp ));
		pgd.SetAttribute("trRdDist",	toStrC( trRdDist  ));

		int i;
		for (int i=0; i < ciNumGrLay; ++i)
		{
			const SGrassLayer& g = grLayersAll[i];
			TiXmlElement grl("grass");
			grl.SetAttribute("on",		g.on ? 1 : 0);
			grl.SetAttribute("mtr",		g.material.c_str());
			grl.SetAttribute("clr",		g.colorMap.c_str());
			grl.SetAttribute("dens",	toStrC( g.dens ));
			grl.SetAttribute("chan",	toStrC( g.iChan ));

			grl.SetAttribute("minSx",	toStrC( g.minSx ));
			grl.SetAttribute("maxSx",	toStrC( g.maxSx ));
			grl.SetAttribute("minSy",	toStrC( g.minSy ));
			grl.SetAttribute("maxSy",	toStrC( g.maxSy ));

			grl.SetAttribute("swayDistr",	toStrC( g.swayDistr ));
			grl.SetAttribute("swayLen",		toStrC( g.swayLen ));
			grl.SetAttribute("swaySpeed",	toStrC( g.swaySpeed ));
			pgd.InsertEndChild(grl);
		}

		for (i=0; i < 4; ++i)
		{
			const SGrassChannel& g = grChan[i];
			TiXmlElement gch("gchan");
			gch.SetAttribute("amin",	toStrC( g.angMin ));
			gch.SetAttribute("amax",	toStrC( g.angMax ));
			gch.SetAttribute("asm",		toStrC( g.angSm ));

			gch.SetAttribute("hmin",	toStrC( g.hMin ));
			gch.SetAttribute("hmax",	toStrC( g.hMax ));
			gch.SetAttribute("hsm",		toStrC( g.hSm ));

			gch.SetAttribute("ns",		toStrC( g.noise ));
			gch.SetAttribute("frq",		toStrC( g.nFreq ));
			gch.SetAttribute("oct",		toStrC( g.nOct ));
			gch.SetAttribute("prs",		toStrC( g.nPers ));
			gch.SetAttribute("pow",		toStrC( g.nPow ));

			gch.SetAttribute("rd",		toStrC( g.rdPow ));
			pgd.InsertEndChild(gch);
		}

		for (i=0; i < ciNumPgLay; ++i)
		{
			const PagedLayer& l = pgLayersAll[i];
			TiXmlElement pgl("layer");
			pgl.SetAttribute("on",			l.on ? 1 : 0);
			pgl.SetAttribute("name",		l.name.c_str());
			pgl.SetAttribute("dens",		toStrC( l.dens ));
			pgl.SetAttribute("minScale",	toStrC( l.minScale ));
			pgl.SetAttribute("maxScale",	toStrC( l.maxScale ));

			pgl.SetAttribute("ofsY",		toStrC( l.ofsY ));
			pgl.SetAttribute("addTrRdDist",	toStrC( l.addRdist ));
			pgl.SetAttribute("maxRdist",	toStrC( l.maxRdist ));
			pgl.SetAttribute("windFx",		toStrC( l.windFx ));
			pgl.SetAttribute("windFy",		toStrC( l.windFy ));

			pgl.SetAttribute("maxTerAng",	toStrC( l.maxTerAng ));
			pgl.SetAttribute("minTerH",		toStrC( l.minTerH ));
			pgl.SetAttribute("maxTerH",		toStrC( l.maxTerH ));
			pgl.SetAttribute("maxDepth",	toStrC( l.maxDepth ));
			pgd.InsertEndChild(pgl);
		}
	root.InsertEndChild(pgd);


	TiXmlElement cam("cam");
		cam.SetAttribute("pos",		toStrC( camPos ));
		cam.SetAttribute("dir",		toStrC( camDir ));
	root.InsertEndChild(cam);


	TiXmlElement objs("objects");
		for (i=0; i < objects.size(); ++i)
		{
			const Object* o = &objects[i];
			TiXmlElement oe("o");
			oe.SetAttribute("name",		o->name.c_str() );

			string s = toStr(o->pos[0])+" "+toStr(o->pos[1])+" "+toStr(o->pos[2]);
			oe.SetAttribute("pos",		s.c_str());

			s = toStr(o->rot[0])+" "+toStr(o->rot[1])+" "+toStr(o->rot[2])+" "+toStr(o->rot[3]);
			oe.SetAttribute("rot",		s.c_str());

			if (o->scale != Vector3::UNIT_SCALE)  // dont save default
				oe.SetAttribute("sc",	toStrC( o->scale ));
			objs.InsertEndChild(oe);
		}
	root.InsertEndChild(objs);


	TiXmlElement emts("emitters");
		for (i=0; i < emitters.size(); ++i)
		{
			const SEmitter* e = &emitters[i];
			TiXmlElement oe("e");
			oe.SetAttribute("name",		e->name.c_str() );

			oe.SetAttribute("pos",	toStrC(e->pos));
			oe.SetAttribute("sc",	toStrC(e->size));
			oe.SetAttribute("up",	toStrC(e->up));
			oe.SetAttribute("rot",	toStrC(e->rot));
			oe.SetAttribute("rate",	toStrC(e->rate));
			oe.SetAttribute("st",	e->stat ? 1 : 0);

			emts.InsertEndChild(oe);
		}
	root.InsertEndChild(emts);


	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}
