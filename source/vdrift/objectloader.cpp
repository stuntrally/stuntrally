#include "pch.h"
#include "objectloader.h"

#include <string>
#include <fstream>
#include "texture.h"
#include "reseatable_reference.h"
#include "track.h"
#include "../ogre/OgreGame.h"

OBJECTLOADER::OBJECTLOADER(
	const std::string & ntrackpath,
	//SCENENODE & nsceneroot, 
	int nanisotropy,
	bool newdynamicshadowsenabled,
	std::ostream & ninfo_output,
	std::ostream & nerror_output,
	bool newcull,
	bool doagressivecombining)
: trackpath(ntrackpath),
	//sceneroot(nsceneroot),
	info_output(ninfo_output),
	error_output(nerror_output),
	error(false),
	numobjects(0),
	packload(false),
	anisotropy(nanisotropy),
	cull(newcull),
	params_per_object(17),
	expected_params(17),
	min_params(14),
	dynamicshadowsenabled(newdynamicshadowsenabled),
	agressivecombine(doagressivecombining)
{
	
}

bool OBJECTLOADER::BeginObjectLoad()
{
	CalculateNumObjects();
	if (numobjects == 0)  return true;//

	std::string objectpath = trackpath + "/objects";
	std::string objectlist = objectpath + "/list.txt";
	objectfile.open(objectlist.c_str());

	if (!GetParam(objectfile, params_per_object))
		return false;

	if (params_per_object != expected_params)
		info_output << "Track object list has " << params_per_object << " params per object, expected " << expected_params << ", this is fine, continuing" << std::endl;
	
	if (params_per_object < min_params)
	{
		error_output << "Track object list has " << params_per_object << " params per object, expected " << expected_params << std::endl;
		return false;
	}

	packload = pack.LoadPack(objectpath + "/objects.jpk");

	return true;
}

void OBJECTLOADER::CalculateNumObjects()
{
	objectpath = trackpath + "/objects";
	std::string objectlist = objectpath + "/list.txt";
	std::ifstream f(objectlist.c_str());

	int params_per_object;
	if (!GetParam(f, params_per_object))
	{
		numobjects = 0;
		return;
	}

	numobjects = 0;

	std::string junk;
	while (GetParam(f, junk))
	{
		for (int i = 0; i < params_per_object-1; i++)
			GetParam(f, junk);

		numobjects++;
	}

	//info_output << "!!!!!!! " << numobjects << endl;
}

bool OBJECTLOADER::GetSurfacesBool()
{
	info_output << "calling Get Surfaces Bool when we shouldn't!!! " << std::endl;
	if (params_per_object >= 17)
		return true;
	else
		return false;
}

std::pair <bool,bool> OBJECTLOADER::ContinueObjectLoad(	TRACK* track, 
	std::map <std::string, MODEL_JOE03> & model_library,
	std::map <std::string, TEXTURE_GL> & texture_library,
	std::list <TRACK_OBJECT> & objects,
	bool vertical_tracking_skyboxes,
 	const std::string & texture_size)
{
	std::string model_name;

	if (error)
		return std::pair <bool,bool> (true, false);

	if (!(GetParam(objectfile, model_name)))
	{
		info_output << "Track loaded: " << model_library.size() << " models, " << texture_library.size() << " textures, " << /*surfaces.size() << " surfaces" << */std::endl;
		//Optimize();
		//info_output << "Objects before optimization: " << unoptimized_scene.GetDrawableList().size() << ", objects after optimization: " << sceneroot.GetDrawableList().size() << std::endl;
		//unoptimized_scene.Clear();
		return std::pair <bool,bool> (false, false);
	}

	assert(objectfile.good());

	std::string diffuse_texture_name;
	bool mipmap;
	bool nolighting;
	bool skybox;
	int transparent_blend;
	float bump_wavelength;
	float bump_amplitude;
	bool driveable;
	bool collideable;
	float friction_notread;
	float friction_tread;
	float rolling_resistance;
	float rolling_drag;
	bool isashadow(false);
	int clamptexture(0);
	int surface_type(2);

	std::string otherjunk;

	GetParam(objectfile, diffuse_texture_name);
	GetParam(objectfile, mipmap);
	GetParam(objectfile, nolighting);
	GetParam(objectfile, skybox);
	GetParam(objectfile, transparent_blend);
	GetParam(objectfile, bump_wavelength);
	GetParam(objectfile, bump_amplitude);
	GetParam(objectfile, driveable);
	GetParam(objectfile, collideable);
	GetParam(objectfile, friction_notread);
	GetParam(objectfile, friction_tread);
	GetParam(objectfile, rolling_resistance);
	GetParam(objectfile, rolling_drag);
	
	if (params_per_object >= 15)
		GetParam(objectfile, isashadow);
	
	if (params_per_object >= 16)
		GetParam(objectfile, clamptexture);
	
	if (params_per_object >= 17)
		GetParam(objectfile, surface_type);
		
		
	for (int i = 0; i < params_per_object - expected_params; i++)
		GetParam(objectfile, otherjunk);

	MODEL * model(NULL);

	if (model_library.find(model_name) == model_library.end())
	{
		if (packload)
		{
			if (!model_library[model_name].Load(model_name, error_output, true, &pack))
			{
				error_output << "Error loading model: " << objectpath + "/" + model_name << " from pack " << objectpath + "/objects.jpk" << std::endl;
				return std::pair <bool, bool> (true, false); //fail the entire track loading
			}
		}
		else/**/
		{
			if (!model_library[model_name].Load(objectpath + "/" + model_name, /*NULL,*/ error_output))
			{
				error_output << "Error loading model: " << objectpath + "/" + model_name << std::endl;
				return std::pair <bool, bool> (true, false); //fail the entire track loading
			}
		}
		model = &model_library[model_name];
	}

	bool skip = false;
	bool bNewMtr = false;///
	
	if (dynamicshadowsenabled && isashadow)
		skip = true;

	if (texture_library.find(diffuse_texture_name) == texture_library.end())
	{
		bNewMtr = true;///
		TEXTUREINFO texinfo;
		texinfo.SetName(objectpath + "/" + diffuse_texture_name);
		texinfo.SetMipMap(mipmap || anisotropy); //always mipmap if anisotropy is on
		texinfo.SetAnisotropy(anisotropy);
		bool clampu = clamptexture == 1 || clamptexture == 2;
		bool clampv = clamptexture == 1 || clamptexture == 3;
		texinfo.SetRepeat(!clampu, !clampv);
		if (!texture_library[diffuse_texture_name].Load(texinfo, error_output, texture_size))
		{
			error_output << "Error loading texture: " << objectpath + "/" + diffuse_texture_name << ", skipping object " << model_name << " and continuing" << std::endl;
			skip = true; //fail the loading of this model only
		}
	}

	if (!skip)
	{
		reseatable_reference <TEXTURE_GL> miscmap1;
		std::string miscmap1_texture_name = diffuse_texture_name.substr(0,std::max(0,(int)diffuse_texture_name.length()-4));
		miscmap1_texture_name += std::string("-misc1.png");
		if (texture_library.find(miscmap1_texture_name) == texture_library.end())
		{
			TEXTUREINFO texinfo;
			std::string filepath = objectpath + "/" + miscmap1_texture_name;
			texinfo.SetName(filepath);
			texinfo.SetMipMap(mipmap);
			texinfo.SetAnisotropy(anisotropy);

			std::ifstream filecheck(filepath.c_str());
			if (filecheck)
			{
				if (!texture_library[miscmap1_texture_name].Load(texinfo, error_output, texture_size))
				{
					error_output << "Error loading texture: " << objectpath + "/" + miscmap1_texture_name << " for object " << model_name << ", continuing" << std::endl;
					texture_library.erase(miscmap1_texture_name);
					//don't fail, this isn't a critical error
				}
				else
					miscmap1 = texture_library[miscmap1_texture_name];
			}
		}
		else
			miscmap1 = texture_library.find(miscmap1_texture_name)->second;

		TEXTURE_GL * diffuse = &texture_library[diffuse_texture_name];
		/*DRAWABLE & d = unoptimized_scene.AddDrawable();
		d.AddDrawList(model->GetListID());
		d.SetDiffuseMap(diffuse);
		if (miscmap1)
			d.SetMiscMap1(&miscmap1.get());
		d.SetLit(!nolighting);
		d.SetPartialTransparency((transparent_blend==1));
		d.SetCull(cull && (transparent_blend!=2), false);
		d.SetRadius(model->GetRadius());
		d.SetObjectCenter(model->GetCenter());
		d.SetSkybox(skybox);
		if (skybox && vertical_tracking_skyboxes)
		{
			d.SetVerticalTrack(true);
		}*/
		
		TRACK_OBJECT object(model, diffuse, /*surfacePtr*/collideable || driveable );
		objects.push_back(object);

		///-------------  push for ogre
		//bool mipmap;	bool nolighting;
		//bool isashadow(false);
		//int clamptexture(0);

		OGRE_MESH om;
		om.found = true;
		om.sky = skybox;
		om.alpha = transparent_blend;

		om.newMtr = bNewMtr;
		om.name = model_name;
		om.material = diffuse_texture_name;
		om.mesh = &model->mesh;
		
		track->ogre_meshes.push_back(om);///
	}

	return std::pair <bool, bool> (false, true);
}

std::string booltostr(bool val)
{
	if (val)
		return "Y";
	else return "N";
}
