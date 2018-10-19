#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "SoundMgr.h"
#include "SoundBase.h"
#include "SoundBaseMgr.h"
#include <OgreDataStream.h>
#include <OgreException.h>
using namespace Ogre;


//  Init
//---------------------------------------------------------------------------------------
SoundMgr::SoundMgr()
	:disabled(true), sound_mgr(0)
{	}

bool SoundMgr::Init(std::string snd_device, bool reverb)
{
	sound_mgr = new SoundBaseMgr();

	bool ok = sound_mgr->Init(snd_device, reverb);
	if (!ok)
	{	LogO("@  SoundScript: Failed to create Sound Manager");  return false;  }

	disabled = sound_mgr->isDisabled();
	if (disabled)
	{	LogO("@  SoundScript: Sound Manager is disabled");  return false;  }

	LogO("@  SoundScript: Sound Manager started with " + toStr(sound_mgr->hw_sources_num)+" sources");
	return true;
}

SoundMgr::~SoundMgr()
{
	for (size_t i=0; i < v_templ.size(); ++i)
		delete v_templ[i];
	delete sound_mgr;
}


//  Update
//---------------------------------------------------------------------------------------
void SoundMgr::setCamera(Vector3 position, Vector3 direction, Vector3 up, Vector3 velocity)
{
	if (disabled)  return;
	sound_mgr->setCamera(position, direction, up, velocity);
}

SoundTemplate* SoundMgr::createTemplate(String name, String filename)
{
	// first, search if there is a template name collision
	if (templates.find(name) != templates.end())
		OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "SoundScript with the name " + name + " already exists.", "SoundScriptManager::createTemplate");

	SoundTemplate *ssi = new SoundTemplate(name, filename);
	templates[name] = ssi;
	v_templ.push_back(ssi);
	return ssi;
}


Sound* SoundMgr::createInstance(Ogre::String name, int car)
{
	//  search template
	if (templates.find(name) == templates.end())
		return NULL;

	SoundTemplate* templ = templates[name];

	Sound* inst = new Sound(car, templ, sound_mgr);
	
	String ss = name.substr(0,4);
	inst->set2D(ss=="hud/");  // set 2d

	//  start looped
	if (!inst->start_sound)
	{	inst->setGain(0.f);
		inst->start();
	}
	return inst;
}


//  Parse
//---------------------------------------------------------------------------------------
void SoundMgr::parseScript(FileStreamDataStream* stream)
{
	SoundTemplate* sst = 0;
	String line = "";  int cnt=0;

	LogO("@  SoundScript: Parsing"/*+stream->getName()*/);

	while (!stream->eof())
	{
		line = stream->getLine();
		//  ignore comments & blanks
		if (!(line.length() == 0 || line.substr(0,2) == "//"))
		{
			if (sst == 0)
			{
				//  no current SoundScript
				//  so first valid data should be a SoundScript name
				//LogO("@  SoundScriptManager: creating template "+line);
				//LogO("@  "+line);
				sst = createTemplate(line, stream->getName());
				if (!sst)
				{
					//  there is a name collision for this Sound Script
					LogO("@  Error, this sound script is already defined: "+line);
					skipToNextOpenBrace(stream);
					skipToNextCloseBrace(stream);
					continue;
				}
				skipToNextOpenBrace(stream);  ++cnt;
			}else
			{
				//  already in a ss
				if (line == "}")
					sst = 0;  // finished ss
				else
				{	//  attribute
					//  split params on space
					Ogre::StringVector veclineparams = StringUtil::split(line, "\t ", 0);

					if (!sst->setParameter(veclineparams))
						LogO("@  Bad SoundScript attribute line: '"+line);
				}
			}
	}	}
	LogO("@  SoundScript: Parsed: "+toStr(cnt)+" templates.");
}

void SoundMgr::skipToNextCloseBrace(FileStreamDataStream* stream)
{
	String line = "";

	while (!stream->eof() && line != "}")
	{
		line = stream->getLine();
	}
}

void SoundMgr::skipToNextOpenBrace(FileStreamDataStream* stream)
{
	String line = "";

	while (!stream->eof() && line != "{")
	{
		line = stream->getLine();
	}
}


//  utility
//---------------------------------------------------------------------------------------
void SoundMgr::setPaused(bool mute)
{
	sound_mgr->pauseAll(mute);
}

void SoundMgr::setMasterVolume(float vol)
{
	sound_mgr->setMasterVolume(vol);
}


//  Load script
//---------------------------------------------------------------------------------------
SoundTemplate::SoundTemplate(String name1, String filename1)
	:file_name(filename1), name(name1), free_sound(0)
	,has_start(false), has_stop(false)
	,unpitchable(false)
{
}

bool SoundTemplate::setParameter(Ogre::StringVector vec)
{
	if (vec.empty())  return false;

	if (vec[0] == "start")
	{
		if (vec.size() < 2)  return false;
		start_name  = vec[1];
		has_start   = true;
		return true;
	}
	else
	if (vec[0] == "stop")
	{
		if (vec.size() < 2)  return false;
		stop_name  = vec[1];
		has_stop   = true;
		return true;
	}
	else
	if (vec[0] == "sound")
	{
		if (vec.size() < 3)  return false;
		if (free_sound >= MAX_SOUNDS_PER_SCRIPT)
		{
			LogO("@  SoundScript: Reached MAX_SOUNDS_PER_SCRIPT limit (" + toStr(MAX_SOUNDS_PER_SCRIPT) + ")");
			return false;
		}
		sound_pitches[free_sound] = StringConverter::parseReal(vec[1]);  // unpitched = 0.0
		if (sound_pitches[free_sound] == 0)
			unpitchable = true;

		if (free_sound > 0 && !unpitchable && sound_pitches[free_sound] <= sound_pitches[free_sound - 1])
			return false;

		sound_names[free_sound] = vec[2];
		++free_sound;
		return true;
	}
	return false;
}


///  Sound
//---------------------------------------------------------------------------------------------------------
Sound::Sound(int car1, SoundTemplate* tpl, SoundBaseMgr* mgr1)
	:car(car1), templ(tpl), sound_mgr(mgr1)
	,start_sound(NULL), stop_sound(NULL)
	,lastgain(1.0f), is2D(false), engine(false)
{
	//  create sounds
	if (tpl->has_start)
		start_sound = sound_mgr->createSound(tpl->start_name, templ->name);
	
	if (tpl->has_stop)
		stop_sound = sound_mgr->createSound(tpl->stop_name, templ->name);
	
	for (int i=0; i < tpl->free_sound; i++)
		sounds[i] = sound_mgr->createSound(tpl->sound_names[i], templ->name);
	
	//if (tpl->free_sound > 0)
		setPitch(1.f);
	setGain(0.f);

	//LogO("@  Sound created: "+name+" "+toStr(tpl->free_sound));
}

Sound::~Sound()
{
	delete start_sound;
	delete stop_sound;
	
	if (templ)
	for (int i=0; i < templ->free_sound; i++)
		delete sounds[i];
}


//  Pitch
//---------------------------------------------------------------------
void Sound::setPitch(float value)
{
	//if (start_sound)  // only pitch looped
	if (templ->free_sound == 0)  return;

	int i, ii = templ->free_sound;
	for (int i=0; i < ii; ++i)
		if (sounds[i])
		{
			float fq = templ->sound_pitches[i];
			float p = value / fq, v = 1.f;
			if (p > 2.f) {  p = 2.f;   v = 0.f;  }
			else
			if (p < 0.5f){  p = 0.5f;  v = 0.f;  }
			else
			if (p > 1.f) {  v = 1.f - (p - 1.f);      if (v < 0.f)  v = 0.f;  }
			else
			if (p < 1.f) {  v = 1.f - (1.f - p)*2.f;  if (v < 0.f)  v = 0.f;  }
			
			if (engine && v < 0.001f)  v = 0.001f;  // engine always on
			if (ii == 1)  v = 1.f;
			
			pitch_gain[i] = v;
			sounds[i]->setGain(v * lastgain);
			sounds[i]->setPitch(p);
			//if (value > 100)  LogO(toStr(i)+" "+fToStr(p)+" "+fToStr(v));
		}
}

//  Gain
//---------------------------------------------------------------------
void Sound::setGain(float value)
{
	//if (fabs(lastgain - value) < 0.001f)
	//	return;
	
	if (start_sound)
		start_sound->setGain(value);

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->setGain(value * pitch_gain[i]);

	if (stop_sound)
		stop_sound->setGain(value);

	lastgain = value;
}

void Sound::setEngine(bool b)
{	engine = b;  }

void Sound::set2D(bool b)
{
	is2D = b;
	if (start_sound)
		start_sound->is2D = b;

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->is2D = b;

	if (stop_sound)
		stop_sound->is2D = b;
}

void Sound::seek(float pos)
{
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->seek(pos);
}

void Sound::setPosition(Vector3 pos, Vector3 velocity)
{
	if (start_sound)
	{	start_sound->setPosition(pos);
		start_sound->setVelocity(velocity);
	}
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
		{	sounds[i]->setPosition(pos);
			sounds[i]->setVelocity(velocity);
		}
	if (stop_sound)
	{	stop_sound->setPosition(pos);
		stop_sound->setVelocity(velocity);
	}
}

bool Sound::isAudible()
{
	if (start_sound)
		return start_sound->isPlaying();

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			return sounds[i]->isPlaying();

	if (stop_sound)
		return stop_sound->isPlaying();

	return false;
}

//  Play
//---------------------------------------------------------------------
void Sound::runOnce()
{
	if (start_sound)
		 if (start_sound->isPlaying())  return;
		else start_sound->play();

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
		{
			if (sounds[i]->isPlaying())
				continue;

			sounds[i]->setLoop(false);
			sounds[i]->play();
		}

	if (stop_sound)
		 if (stop_sound->isPlaying())  return;
		else stop_sound->play();
}

void Sound::start()
{
	if (start_sound)
	{	start_sound->stop();
		start_sound->play();
	}
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
		{	sounds[i]->setLoop(true);
			sounds[i]->play();
		}
}

void Sound::stop()
{
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->stop();

	if (stop_sound)
	{	stop_sound->stop();
		stop_sound->play();
	}
}

void Sound::kill()
{
	for (int i = 0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->stop();

	if (start_sound)
		start_sound->stop();

	if (stop_sound)
	{	stop_sound->stop();
		stop_sound->play();
	}
}

void Sound::setEnabled(bool e)
{
	if (start_sound)
		start_sound->setEnabled(e);

	if (stop_sound)
		stop_sound->setEnabled(e);

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->setEnabled(e);
}
