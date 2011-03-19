#include "stdafx.h"

#include "replay.h"
#include "unittest.h"


void REPLAY::Save(std::ostream & outstream)
{
	joeserialize::BinaryOutputSerializer serialize_output(outstream);
	
	//the convention is that input frames come first, then state frames.
	joeserialize::Serializer & s = serialize_output;
	s.Serialize("inputframes", inputframes);
	s.Serialize("stateframes", stateframes);
	
	inputframes.clear();
	stateframes.clear();
}

void REPLAY::SaveHeader(std::ostream & outstream)
{
	//write the file format version data manually.  if the serialization functions were used, a variable length string would be written instead, which isn't exactly what we want
	version_info.Save(outstream);
	
	joeserialize::BinaryOutputSerializer serialize_output(outstream);
	Serialize(serialize_output);
}

bool REPLAY::Load(std::istream & instream)
{
	//peek to ensure we're not at the EOF
	instream.peek();
	if (instream.eof())
		return false;
	
	std::vector <INPUTFRAME> newinputframes;
	std::vector <STATEFRAME> newstateframes;
	
	joeserialize::BinaryInputSerializer serialize_input(instream);
	
	//the convention is that input frames come first, then state frames.
	joeserialize::Serializer & s = serialize_input;
	s.Serialize("inputframes", newinputframes);
	s.Serialize("stateframes", newstateframes);
	
	//append the frames to the list
	inputframes.insert(inputframes.end(), newinputframes.begin(), newinputframes.end());
	stateframes.insert(stateframes.end(), newstateframes.begin(), newstateframes.end());
	
	return true;
}

bool REPLAY::LoadHeader(std::istream & instream, std::ostream & error_output)
{
	REPLAYVERSIONING stream_version;
	stream_version.Load(instream);
	
	if (!(stream_version == version_info))
	{
		error_output << "Stream version " << stream_version.format_version << "/" << stream_version.inputs_supported << " does not match expected version " << version_info.format_version << "/" << version_info.inputs_supported << endl;
		return false;
	}
	
	joeserialize::BinaryInputSerializer serialize_input(instream);
	Serialize(serialize_input);
	
	return true;
}

void REPLAY::GetReadyToRecord()
{
	frame = 0;
	replaymode = RECORDING;
	inputframes.clear();
	stateframes.clear();
	inputbuffer.clear();
	inputbuffer.resize(CARINPUT::GAME_ONLY_INPUTS_START_HERE, 0);
}

void REPLAY::StartRecording(const std::string & newcartype, const std::string & newcarpaint, const std::string & carfilename, const std::string & trackname, ostream & error_log)
{
	track = trackname;
	cartype = newcartype;
	carpaint = newcarpaint;
	
	GetReadyToRecord();
	
	ifstream carfilestream(carfilename.c_str());
	if (!carfilestream)
		error_log << "Unable to open car file to put into replay: " << carfilename << endl;
	while (carfilestream)
	{
		string newline;
		std::getline(carfilestream, newline);
		carfile.append(newline);
		carfile.push_back('\n');
	}
}

void REPLAY::GetReadyToPlay()
{
	frame = 0;
	replaymode = PLAYING;
	inputbuffer.clear();
	inputbuffer.resize(CARINPUT::GAME_ONLY_INPUTS_START_HERE, 0);
	
	//set the playback position at the beginning
	cur_inputframe = 0;
	cur_stateframe = 0;
}

bool REPLAY::StartPlaying(const std::string & replayfilename, std::ostream & error_output)
{
	GetReadyToPlay();
	
	//open the file
	ifstream replaystream(replayfilename.c_str(), ifstream::binary);
	if (!replaystream)
	{
		error_output << "Error loading replay file: " << replayfilename << endl;
		return false;
	}
	
	//load the header info from the file
	if (!LoadHeader(replaystream, error_output))
		return false;
	
	//load all of the input/state frame chunks from the file until we hit the EOF
	while (Load(replaystream))
	{
	}
	
	return true;
}

void REPLAY::StopRecording(const std::string & replayfilename)
{
	replaymode = IDLE;
	
	if (!replayfilename.empty())
	{
		ofstream f(replayfilename.c_str());
		if (f)
		{
			SaveHeader(f);
			Save(f);
		}
	}
}

void REPLAY::StopPlaying()
{
	replaymode = IDLE;
}

void REPLAY::RecordFrame(const std::vector <float> & inputs, CAR & car)
{
	if (!GetRecording())
		return;
	
	if (frame > 2000000000) //enforce a maximum recording time of about 92 days
	{
		StopRecording("");
	}
	
	assert(inputbuffer.size() == CARINPUT::GAME_ONLY_INPUTS_START_HERE);
	assert((unsigned int) version_info.inputs_supported == CARINPUT::GAME_ONLY_INPUTS_START_HERE);
	
	//record inputs
	INPUTFRAME newinputframe(frame);
	
	for (unsigned int i = 0; i < CARINPUT::GAME_ONLY_INPUTS_START_HERE; i++)
	{
		if (inputs[i] != inputbuffer[i])
		{
			inputbuffer[i] = inputs[i];
			newinputframe.AddInput(i, inputs[i]);
		}
	}
	
	if (newinputframe.GetNumInputs() > 0)
		inputframes.push_back(newinputframe);
	
	
	//record state
	int framespersecond = 1.0/version_info.framerate;
	if (frame % framespersecond == 0) //once per second
	{
		stringstream statestream;
		joeserialize::BinaryOutputSerializer serialize_output(statestream);
		car.Serialize(serialize_output);
		stateframes.push_back(STATEFRAME(frame));
		//stateframes.push_back(STATEFRAME(11189196)); //for debugging; in hex, 11189196 is AABBCC
		//cout << "Recording state size: " << statestream.str().length() << endl; //should be ~984
		stateframes.back().SetBinaryStateData(statestream.str());
		stateframes.back().SetInputSnapshot(inputs);
	}
	
	frame++;
}

const std::vector <float> & REPLAY::PlayFrame(CAR & car)
{
	if (!GetPlaying())
	{
		return inputbuffer;
	}
	
	frame++;
	
	assert(inputbuffer.size() == CARINPUT::GAME_ONLY_INPUTS_START_HERE);
	assert((unsigned int) version_info.inputs_supported == CARINPUT::GAME_ONLY_INPUTS_START_HERE);
	
	//fast forward through the inputframes until we're up to date
	while (cur_inputframe < inputframes.size() && inputframes[cur_inputframe].GetFrame() <= frame)
	{
		ProcessPlayInputFrame(inputframes[cur_inputframe]);
		cur_inputframe++;
	}
	
	//fast forward through the stateframes until we're up to date
	while (cur_stateframe < stateframes.size() && stateframes[cur_stateframe].GetFrame() <= frame)
	{
		if (stateframes[cur_stateframe].GetFrame() == frame) ProcessPlayStateFrame(stateframes[cur_stateframe], car);
		cur_stateframe++;
	}
	
	//detect end of input
	if (cur_stateframe == stateframes.size() && cur_inputframe == inputframes.size())
	{
		StopPlaying();
	}
	
	return inputbuffer;
}

void REPLAY::ProcessPlayInputFrame(const INPUTFRAME & frame)
{
	for (unsigned int i = 0; i < frame.GetNumInputs(); i++)
	{
		std::pair <int, float> input_pair = frame.GetInput(i);
		inputbuffer[input_pair.first] = input_pair.second;
	}
}

void REPLAY::ProcessPlayStateFrame(const STATEFRAME & frame, CAR & car)
{
	//process input snapshot
	for (unsigned int i = 0; i < inputbuffer.size() && i < frame.GetInputSnapshot().size(); i++)
	{
		inputbuffer[i] = frame.GetInputSnapshot()[i];
	}
	
	//process binary car state
	//cout << "Playing state frame..." << endl;
	stringstream statestream(frame.GetBinaryStateData());
	//cout << "Frame size: " << frame.GetBinaryStateData().length() << endl;
	joeserialize::BinaryInputSerializer serialize_input(statestream);
	car.Serialize(serialize_input);
	//cout << "Played state frame" << endl;
}

QT_TEST(replay_test)
{
	/*//basic version validity check
	{
		REPLAY replay(0.004);
		stringstream teststream;
		replay.Save(teststream);
		QT_CHECK(replay.Load(teststream, std::cerr));
	}*/
}
