#ifndef _REPLAY_H
#define _REPLAY_H

#include "car.h"
#include "cardefs.h"
#include "joeserialize.h"
#include "macros.h"

//#include <iostream>
//#include <string>

class REPLAY
{
friend class joeserialize::Serializer;
private:
	//sub-classes
	class REPLAYVERSIONING
	{
		public:
			std::string format_version;
			int inputs_supported;
			float framerate;
			
			REPLAYVERSIONING() : format_version("VDRIFTREPLAYV??"), inputs_supported(0), framerate(0) {}
			REPLAYVERSIONING(const std::string & ver, unsigned int ins, float newfr) : format_version(ver),
				     inputs_supported(ins), framerate(newfr) {}
				     
			bool Serialize(joeserialize::Serializer & s)
			{
				_SERIALIZE_(s,inputs_supported);
				_SERIALIZE_(s,framerate);
				return true;
			}

			void Save(std::ostream & outstream)
			{
				//write the file format version data manually.  if the serialization functions were used, a variable length string would be written instead, which isns't exactly what we want
				outstream.write(format_version.data(), format_version.length());
				
				//write the rest of the versioning info
				joeserialize::BinaryOutputSerializer serialize_output(outstream);
				Serialize(serialize_output);
			}
			
			void Load(std::istream & instream)
			{
				//read the file format version data manually
				const unsigned int bufsize = format_version.length()+1;
				char * version_buf = new char[bufsize+1];
				instream.get(version_buf, bufsize);
				version_buf[bufsize] = '\0';
				format_version = version_buf;
				delete [] version_buf;
				
				//read the rest of the versioning info
				joeserialize::BinaryInputSerializer serialize_input(instream);
				Serialize(serialize_input);
			}
			
			bool operator==(const REPLAYVERSIONING & other) const
			{
				return (format_version == other.format_version && 
					inputs_supported == other.inputs_supported && 
				        framerate == other.framerate);
			}
	};
	
	class INPUTFRAME
	{
	private:
		friend class joeserialize::Serializer;
		class INPUTPAIR
		{
		public:
			INPUTPAIR() : index(0), value(0) {}
			INPUTPAIR(int newindex, float newvalue) : index(newindex), value(newvalue) {}
			
			int index;
			float value;
			
			bool Serialize(joeserialize::Serializer & s)
			{
				_SERIALIZE_(s,index);
				_SERIALIZE_(s,value);
				return true;
			}
		};
		
		int frame;
		std::vector <INPUTPAIR> inputs;
		
	public:
		INPUTFRAME() : frame(0) {}
		INPUTFRAME(int newframe) : frame(newframe) {}
		
		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s,frame);
			_SERIALIZE_(s,inputs);
			return true;
		}
		
		void AddInput(int index, float value)
		{
			inputs.push_back(INPUTPAIR(index,value));
		}
		
		unsigned int GetNumInputs() const {return inputs.size();}

		int GetFrame() const
		{
			return frame;
		}
		
		///returns a pair for the <control id, value> of the indexed input
		std::pair <int, float> GetInput(unsigned int index) const {assert(index < inputs.size());return std::pair <int, float> (inputs[index].index, inputs[index].value);}
	};
	
	class STATEFRAME
	{
	private:
		friend class joeserialize::Serializer;
		
		int frame;
		std::string binary_state_data;
		std::vector <float> input_snapshot;
		
	public:
		STATEFRAME() : frame(0) {}
		STATEFRAME(int newframe) : frame(newframe) {}
		
		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s,frame);
			_SERIALIZE_(s,binary_state_data);
			_SERIALIZE_(s,input_snapshot);
			//std::cout << binary_state_data.length() << std::endl;
			return true;
		}

		void SetBinaryStateData (const std::string & value)
		{
			binary_state_data = value;
			//std::cout << binary_state_data.length() << std::endl;
		}

		int GetFrame() const
		{
			return frame;
		}

		std::string GetBinaryStateData() const
		{
			//std::cout << binary_state_data.length() << std::endl;
			return binary_state_data;
		}

		const std::vector < float > & GetInputSnapshot() const
		{
			return input_snapshot;
		}

		void SetInputSnapshot ( const std::vector< float >& value )
		{
			input_snapshot = value;
		}
	};
	
	//version information
	REPLAYVERSIONING version_info;
	
	//serialized
	std::string cartype; //car type, used for loading graphics and sound
	std::string carpaint; //car paint id string
	std::string carfile; //entire contents of the car file (e.g. XS.car)
	std::string track;
	std::vector <INPUTFRAME> inputframes;
	std::vector <STATEFRAME> stateframes;
	
	//not stored in the replay file
	int frame;
	enum
	{
		IDLE,
		RECORDING,
		PLAYING
	} replaymode;
	std::vector <float> inputbuffer;
	unsigned int cur_inputframe;
	unsigned int cur_stateframe;
	
	//functions
	void ProcessPlayInputFrame(const INPUTFRAME & frame);
	void ProcessPlayStateFrame(const STATEFRAME & frame, CAR & car);
	bool Load(std::istream & instream); ///< load one input and state frame chunk from the stream. returns true on success, returns false for EOF
	bool LoadHeader(std::istream & instream, std::ostream & error_output); ///< returns true on success.
	void Save(std::ostream & outstream); ///< save all input and state frames to the stream and then clear them
	void SaveHeader(std::ostream & outstream); ///< write only the header information to the stream
	void GetReadyToPlay();
	void GetReadyToRecord();
	
public:
	REPLAY(float framerate) : version_info("VDRIFTREPLAYV11", CARINPUT::GAME_ONLY_INPUTS_START_HERE, framerate),frame(0),replaymode(IDLE) {inputbuffer.resize(CARINPUT::GAME_ONLY_INPUTS_START_HERE, 0);}
	
	//playing
	bool StartPlaying(const std::string & replayfilename, std::ostream & error_output); ///< returns true on success
	void StopPlaying();
	bool GetPlaying() const {return (replaymode == PLAYING);} ///< returns true if the replay system is currently playing
	const std::vector <float> & PlayFrame(CAR & car);
	
	//recording
	void StartRecording(const std::string & newcartype, const std::string & newcarpaint, const std::string & carfilename, const std::string & trackname, std::ostream & error_log);
	void StopRecording(const std::string & replayfilename); ///< if replayfilename is empty, do not save the data
	bool GetRecording() const {return (replaymode == RECORDING);} ///< returns true if the replay system is currently recording
	void RecordFrame(const std::vector <float> & inputs, CAR & car);
	
	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s, cartype);
		_SERIALIZE_(s, carpaint);
		_SERIALIZE_(s, carfile);
		_SERIALIZE_(s, track);
		//_SERIALIZE_(s, inputframes);
		//_SERIALIZE_(s, stateframes);
		return true;
	}

	std::string GetCarType() const
	{
		return cartype;
	}

	std::string GetCarFile() const
	{
		return carfile;
	}

	std::string GetTrack() const
	{
		return track;
	}

	std::string GetCarPaint() const
	{
		return carpaint;
	}
	
};

#endif
