#pragma once
#include <list>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>

#ifdef USE_TR1
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif

namespace joeserialize
{

///abstract base class for serializers.
class Serializer
{
	protected:
		///optional hints to higher level classes about where we are in the serialization process
		virtual void ComplexTypeStart(const std::string & name) {}
		
		///optional hints to higher level classes about where we are in the serialization process
		virtual void ComplexTypeEnd(const std::string & name) {}
	
	public:
		///generic serialization function that will be called for complex types; this is a branch. returns true on success
		template <typename T>
		bool Serialize(const std::string & name, T & t)
		{
			ComplexTypeStart(name);
			bool success = t.Serialize(*this);
			ComplexTypeEnd(name);
			return success;
		}
		
		///shortcut function for anonymously serializing the top-most object. returns true on success
		template <typename T>
		bool Serialize(T & t)
		{
			return t.Serialize(*this);
		}
		
		///serialization overload for a simple type; this is a leaf.
		///a serializer must implement this function. note that in some cases (if the format has IDs implied by the order of data) the name might be ignored. returns true on success
		virtual bool Serialize(const std::string & name, int & t) = 0;
		virtual bool Serialize(const std::string & name, unsigned int & t) = 0;
		virtual bool Serialize(const std::string & name, float & t) = 0;
		virtual bool Serialize(const std::string & name, double & t) = 0;
		virtual bool Serialize(const std::string & name, std::string & t) = 0;
		
		///treat bools like ints
		virtual bool Serialize(const std::string & name, bool & t)
		{
			int boolint = t;
			if (!Serialize(name, boolint)) return false;
			t = boolint;
			return true;
		}
		
		///serialization overload for a complex type that we don't have the ability to change,
		/// so we explicitly define the serialization process here. returns true on success
		template <typename U, typename T>
		bool Serialize(const std::string & name, std::pair <U, T> & t)
		{
			ComplexTypeStart(name);
			if (!Serialize("first", t.first)) return false;
			if (!Serialize("second", t.second)) return false;
			ComplexTypeEnd(name);
			return true;
		}
		
		///serialization overload for a complex type that we don't have the ability to change,
		/// so we explicitly define the serialization process here. returns true on success
		template <typename T>
		bool Serialize(const std::string & name, std::list <T> & t)
		{
			ComplexTypeStart(name);
			if (this->GetIODirection() == DIRECTION_OUTPUT)
			{
				int listsize = t.size();
				if (!this->Serialize("size", listsize)) return false;
				
				int count = 1;
				for (typename std::list <T>::iterator i = t.begin(); i != t.end(); ++i, ++count)
				{
					std::stringstream itemname;
					itemname << "item" << count;
					if (!this->Serialize(itemname.str(), *i)) return false;
				}
			}
			else //input
			{
				t.clear();

				int listsize(0);
				if (!this->Serialize("size", listsize)) return false;

				for (int i = 0; i < listsize; ++i)
				{
					std::stringstream itemname;
					itemname << "item" << i+1;
					t.push_back(T());
					if (!this->Serialize(itemname.str(), t.back())) return false;
				}
			}
			ComplexTypeEnd(name);
			return true;
		}
		
		///serialization overload for a complex type that we don't have the ability to change,
		/// so we explicitly define the serialization process here. returns true on success
		template <typename T>
		bool Serialize(const std::string & name, std::set <T> & t)
		{
			ComplexTypeStart(name);
			if (this->GetIODirection() == DIRECTION_OUTPUT)
			{
				int listsize = t.size();
				if (!this->Serialize("size", listsize)) return false;
				
				int count = 1;
				for (typename std::set <T>::iterator i = t.begin(); i != t.end(); ++i, ++count)
				{
					std::stringstream itemname;
					itemname << "item" << count;
					if (!this->Serialize(itemname.str(), const_cast<T&>(*i))) return false;
				}
			}
			else //input
			{
				t.clear();

				int listsize(0);
				if (!this->Serialize("size", listsize)) return false;

				for (int i = 0; i < listsize; ++i)
				{
					std::stringstream itemname;
					itemname << "item" << i+1;
					//t.push_back(T());
					//if (!this->Serialize(itemname.str(), t.back())) return false;
					T prototype;
					if (!this->Serialize(itemname.str(), prototype)) return false;
					t.insert(prototype);
				}
			}
			ComplexTypeEnd(name);
			return true;
		}
		
		///serialization overload for a complex type that we don't have the ability to change,
		/// so we explicitly define the serialization process here. returns true on success
		template <typename T>
		bool Serialize(const std::string & name, std::vector <T> & t)
		{
			ComplexTypeStart(name);
			if (this->GetIODirection() == DIRECTION_OUTPUT)
			{
				int listsize = t.size();
				if (!this->Serialize("size", listsize)) return false;
				
				int count = 1;
				for (typename std::vector <T>::iterator i = t.begin(); i != t.end(); ++i, ++count)
				{
					std::stringstream itemname;
					itemname << "item" << count;
					if (!this->Serialize(itemname.str(), *i)) return false;
				}
			}
			else //input
			{
				//t.clear();
				int listsize;
				if (!this->Serialize("size", listsize)) return false;
				t.resize(listsize); //only resize, don't clear; we don't want to throw away information

				for (int i = 0; i < listsize; ++i)
				{
					std::stringstream itemname;
					itemname << "item" << i+1;
					//t.push_back(T());
					//if (!this->Serialize(itemname.str(), t.back())) return false;
					if (!this->Serialize(itemname.str(), t[i])) return false;
				}
			}
			ComplexTypeEnd(name);
			return true;
		}
		
		/// vector <bool> is special
		bool Serialize(const std::string & name, std::vector <bool> & t)
		{
			ComplexTypeStart(name);
			if (this->GetIODirection() == DIRECTION_OUTPUT)
			{
				int listsize = t.size();
				if (!this->Serialize("size", listsize)) return false;
				
				int count = 1;
				for (std::vector <bool>::iterator i = t.begin(); i != t.end(); ++i, ++count)
				{
					std::stringstream itemname;
					itemname << "item" << count;
					bool booli = *i;
					if (!this->Serialize(itemname.str(), booli)) return false;
				}
			}
			else //input
			{
				//t.clear();
				int listsize;
				if (!this->Serialize("size", listsize)) return false;
				t.resize(listsize); //only resize, don't clear; we don't want to throw away information

				for (int i = 0; i < listsize; ++i)
				{
					std::stringstream itemname;
					itemname << "item" << i+1;
					//t.push_back(T());
					//if (!this->Serialize(itemname.str(), t.back())) return false;
					bool booli;
					if (!this->Serialize(itemname.str(), booli)) return false;
					t[i] = booli;
				}
			}
			ComplexTypeEnd(name);
			return true;
		}
		
		///serialization overload for a complex type that we don't have the ability to change,
		/// so we explicitly define the serialization process here. returns true on success
		template <typename T>
		bool Serialize(const std::string & name, std::deque <T> & t)
		{
			ComplexTypeStart(name);
			if (this->GetIODirection() == DIRECTION_OUTPUT)
			{
				int listsize = t.size();
				if (!this->Serialize("size", listsize)) return false;
				
				int count = 1;
				for (typename std::deque <T>::iterator i = t.begin(); i != t.end(); ++i, ++count)
				{
					std::stringstream itemname;
					itemname << "item" << count;
					if (!this->Serialize(itemname.str(), *i)) return false;
				}
			}
			else //input
			{
				//t.clear();
				int listsize;
				if (!this->Serialize("size", listsize)) return false;
				t.resize(listsize); //only resize, don't clear; we don't want to throw away information

				for (int i = 0; i < listsize; ++i)
				{
					std::stringstream itemname;
					itemname << "item" << i+1;
					//t.push_back(T());
					//if (!this->Serialize(itemname.str(), t.back())) return false;
					if (!this->Serialize(itemname.str(), t[i])) return false;
				}
			}
			ComplexTypeEnd(name);
			return true;
		}
		
		///serialization overload for a complex type that we don't have the ability to change,
		/// so we explicitly define the serialization process here. returns true on success
		template <typename U, typename T>
		bool Serialize(const std::string & name, std::map <U, T> & t)
		{
			ComplexTypeStart(name);
			if (this->GetIODirection() == DIRECTION_OUTPUT)
			{
				int listsize = t.size();
				if (!this->Serialize("size", listsize)) return false;
				
				int count = 1;
				for (typename std::map <U,T>::iterator i = t.begin(); i != t.end(); ++i, ++count)
				{
					std::stringstream countstr;
					countstr << count;
					
					std::string keystr = "Key" + countstr.str();
					std::string valstr = "Value" + countstr.str();

					U tempkey = i->first;
					if (!this->Serialize(keystr, tempkey)) return false;
					if (!this->Serialize(valstr, i->second)) return false;
				}
			}
			else //input
			{
				t.clear();

				int listsize;
				if (!this->Serialize("size", listsize)) return false;

				for (int i = 0; i < listsize; ++i)
				{
					std::stringstream countstr;
					countstr << i+1;

					std::string keystr = "Key" + countstr.str();
					std::string valstr = "Value" + countstr.str();

					U tempkey;
					if (!this->Serialize(keystr, tempkey)) return false;
					if (!this->Serialize(valstr, t[tempkey])) return false;
				}
			}
			ComplexTypeEnd(name);
			return true;
		}
		
#ifdef USE_TR1
		///serialization overload for a complex type that we don't have the ability to change,
		/// so we explicitly define the serialization process here. returns true on success
		template <typename U, typename T, typename H>
		bool Serialize(const std::string & name, std::tr1::unordered_map <U, T, H> & t)
		{
			ComplexTypeStart(name);
			if (this->GetIODirection() == DIRECTION_OUTPUT)
			{
				int listsize = t.size();
				if (!this->Serialize("size", listsize)) return false;
				
				int count = 1;
				for (typename std::tr1::unordered_map <U,T>::iterator i = t.begin(); i != t.end(); ++i, ++count)
				{
					std::stringstream countstr;
					countstr << count;
					
					std::string keystr = "Key" + countstr.str();
					std::string valstr = "Value" + countstr.str();

					U tempkey = i->first;
					if (!this->Serialize(keystr, tempkey)) return false;
					if (!this->Serialize(valstr, i->second)) return false;
				}
			}
			else //input
			{
				t.clear();

				int listsize;
				if (!this->Serialize("size", listsize)) return false;

				for (int i = 0; i < listsize; ++i)
				{
					std::stringstream countstr;
					countstr << i+1;

					std::string keystr = "Key" + countstr.str();
					std::string valstr = "Value" + countstr.str();

					U tempkey;
					if (!this->Serialize(keystr, tempkey)) return false;
					if (!this->Serialize(valstr, t[tempkey])) return false;
				}
			}
			ComplexTypeEnd(name);
			return true;
		}
		
		///serialization overload for a complex type that we don't have the ability to change,
		/// so we explicitly define the serialization process here. returns true on success
		template <typename T, typename H>
		bool Serialize(const std::string & name, std::tr1::unordered_set <T, H> & t)
		{
			ComplexTypeStart(name);
			if (this->GetIODirection() == DIRECTION_OUTPUT)
			{
				int listsize = t.size();
				if (!this->Serialize("size", listsize)) return false;
				
				int count = 1;
				for (typename std::tr1::unordered_set <T>::iterator i = t.begin(); i != t.end(); ++i, ++count)
				{
					std::stringstream itemname;
					itemname << "item" << count;
					if (!this->Serialize(itemname.str(), const_cast<T&>(*i))) return false;
				}
			}
			else //input
			{
				t.clear();

				int listsize(0);
				if (!this->Serialize("size", listsize)) return false;

				for (int i = 0; i < listsize; ++i)
				{
					std::stringstream itemname;
					itemname << "item" << i+1;
					//t.push_back(T());
					//if (!this->Serialize(itemname.str(), t.back())) return false;
					T prototype;
					if (!this->Serialize(itemname.str(), prototype)) return false;
					t.insert(prototype);
				}
			}
			ComplexTypeEnd(name);
			return true;
		}
#endif
		
		enum Direction
		{
			DIRECTION_INPUT,
   			DIRECTION_OUTPUT
		};
		virtual Direction GetIODirection() = 0;
};

class SerializerOutput : public Serializer
{
	public:
		virtual Direction GetIODirection() {return DIRECTION_OUTPUT;}
};

class SerializerInput : public Serializer
{
	public:
		virtual Direction GetIODirection() {return DIRECTION_INPUT;}
};

///handy utility class for (key,value) style parsed serializers that parse an entire input file before doing any serialization. provides functionality for keeping track of where we are in the serialization process.
template <typename T>
class TreeMap
{
	private:
		T leaf_;
		std::map <std::string, TreeMap> branches_;
		
		void Print(std::ostream & out, std::deque <std::string> & location) const
		{
			if (branches_.empty()) //leaf
			{
				out << Implode(location) << " = " << leaf_ << std::endl;
			}
			else
			{
				for (typename std::map <std::string, TreeMap>::const_iterator i = branches_.begin(); i != branches_.end(); ++i)
				{
					location.push_back(i->first);
					i->second.Print(out, location);
					location.pop_back();
				}
			}
		}

	public:
		std::vector <std::string> GetBranches() const
		{
			std::vector <std::string> leavesout;
			for (typename std::map <std::string, TreeMap>::const_iterator i = branches_.begin(); i != branches_.end(); ++i)
			{
				leavesout.push_back(i->first);
			}
			return leavesout;
		}
		
		void clear()
		{
			branches_.clear();
			leaf_ = T();
		}
		
		const T & leaf() const
		{
			return leaf_;
		}

		void set_leaf ( const T& theValue )
		{
			leaf_ = theValue;
		}

		unsigned int NumberOfBranches() const {return branches_.size();}

		///returns NULL if the branch does not exist
		TreeMap * branch ( const std::string & name )
		{
			typename std::map <std::string, TreeMap>::iterator i = branches_.find ( name );
			if ( i == branches_.end() )
				return NULL;
			else
				return & ( i->second );
		}
		///returns NULL if the branch does not exist
		const TreeMap * branch ( const std::string & name ) const
		{
			typename std::map <std::string, TreeMap>::const_iterator i = branches_.find ( name );
			if ( i == branches_.end() )
				return NULL;
			else
				return & ( i->second );
		}
		
		///gets the branch or NULL if an error occurred.
		const TreeMap * GetBranch ( const std::deque <std::string> & location ) const
		{
			const TreeMap * curmap = this;
			
			for ( std::deque <std::string>::const_iterator i = location.begin(); i != location.end(); ++i )
			{
				curmap = curmap->branch ( *i );

				if ( !curmap ) //branch not found
				{
					return NULL;
				}
			}
			
			return curmap;
		}
		
		TreeMap * GetBranch ( const std::deque <std::string> & location )
		{
			TreeMap * curmap = this;
			
			for ( std::deque <std::string>::const_iterator i = location.begin(); i != location.end(); ++i )
			{
				curmap = curmap->branch ( *i );

				if ( !curmap ) //branch not found
				{
					return NULL;
				}
			}
			
			return curmap;
		}

		///sets the output argument, or if an error occurred, does not modify it. returns true on success.
		bool GetLeaf ( const std::deque <std::string> & location, T & output ) const
		{
			const TreeMap * curmap = this;

			for ( std::deque <std::string>::const_iterator i = location.begin(); i != location.end(); ++i )
			{
				curmap = curmap->branch ( *i );

				if ( !curmap ) //branch not found
				{
					return false;
				}
			}

			if ( curmap->NumberOfBranches() > 0 ) //not really a leaf
			{
				return false;
			}

			output = curmap->leaf();
			return true;
		}

		///set the leaf that resides at the list of branches passed in as a location.  automatically creates branches as necessary.
		void SetLeaf ( const std::deque <std::string> & location, const T & value )
		{
			TreeMap * curmap = this;

			for ( std::deque <std::string>::const_iterator i = location.begin(); i != location.end(); ++i )
			{
				TreeMap * nextmap = curmap->branch ( *i );

				if ( !nextmap ) //branch not found
					nextmap = &curmap->AddBranch ( *i );

				curmap = nextmap;
			}

			curmap->set_leaf ( value );
		}

		///add the branch and return a reference to it.  if the branch already exists then it is simply returned unmodified.
		TreeMap <T> & AddBranch ( const std::string & name )
		{
			return branches_[name];
		}
		
		void Print(std::ostream & out) const
		{
			std::deque <std::string> location;
			Print(out, location);
		}
		
		///construct a string from a deque of strings, adding the delimiter between each string
		std::string Implode(const std::deque <std::string> & container, char implode_delimiter='.') const
		{
			std::string imploded;
			
			for (std::deque <std::string>::const_iterator i = container.begin(); i != container.end(); ++i)
			{
				if (i != container.begin())
					imploded.push_back(implode_delimiter);
				imploded = imploded + *i;
			}
			
			return imploded;
		}
		
		///add the parameters from the other tree to this tree, over-writing duplicates with the othertree values
		void Merge(const TreeMap <T> & othertree)
		{
			leaf_ = othertree.leaf_;
			
			for (typename std::map <std::string, TreeMap>::const_iterator i = othertree.branches_.begin();
				i != othertree.branches_.end(); ++i)
			{
				TreeMap * curmap = branch(i->first);
				if (!curmap)
					branches_[i->first] = i->second;
				else
					curmap->Merge(i->second);
			}
		}
};

///input serializer for a simple text format.
class TextOutputSerializer : public SerializerOutput
{
	private:
		std::ostream & out_;
		int indent_;
		
		void PrintIndent()
		{
			for (int i = 0; i < indent_; ++i)
				out_ << "  ";
		}
		
		///replaces <CR> with '\n'
		std::string Escape(const std::string & input) const
		{
			std::string outputstr;
			for (unsigned int i = 0; i < input.length(); ++i)
			{
				if (input[i] == '\n')
					outputstr.append("\\n");
				else
					outputstr.append(1, input[i]);
			}
			return outputstr;
		}
		
		template <typename T>
		bool WriteData(const std::string & name, const T & i, bool precise)
		{
			PrintIndent();
			out_ << name << " = ";
			if (precise)
				out_ << std::scientific << std::setprecision (20) << i << std::endl;
			else
				out_ << i << std::endl;
			return true;
		}
		
	protected:
		virtual void ComplexTypeStart(const std::string & name)
		{
			PrintIndent();
			out_ << name << std::endl;
			PrintIndent();
			out_ << "{" << std::endl;
			indent_++;
		}
		
		virtual void ComplexTypeEnd(const std::string & name)
		{
			indent_--;
			PrintIndent();
			out_ << "}" << std::endl;
		}
		
	public:
		TextOutputSerializer(std::ostream & newout) : out_(newout),indent_(0) {}
		
		virtual bool Serialize(const std::string & name, int & i)
		{
			return WriteData(name, i, false);
		}
		
		virtual bool Serialize(const std::string & name, unsigned int & i)
		{
			return WriteData(name, i, false);
		}
		
		virtual bool Serialize(const std::string & name, float & i)
		{
			return WriteData(name, i, true);
		}
		
		virtual bool Serialize(const std::string & name, double & i)
		{
			return WriteData(name, i, true);
		}
		
		virtual bool Serialize(const std::string & name, std::string & i)
		{
			return WriteData(name, Escape(i), false);
		}
};

///input serializer for our simple text format. this makes use of the TreeMap class to parse in the entire file before serialization, so it is tolerant of out-of-order fields.
class TextInputSerializer : public SerializerInput
{
	private:
		TreeMap <std::string> parsed_data_tree_;
		std::deque <std::string> serialization_location_;
		std::ostream * error_output_;
		
		void ConsumeWhitespace(std::istream & in) const
		{
			while (in && (in.peek() == ' ' || in.peek() == '\n' || in.peek() == '\t'))
				in.get();
		}
		
		///seeks to the delimiter return the text minus the delimiter
		std::string SeekTo(std::istream & in, char delim) const
		{
			std::string text;
			std::getline(in,text,delim);
			return text;
		}
		
		std::string strLTrim(std::string instr) const
		{
			return instr.erase(0, instr.find_first_not_of(" \t"));
		}

		std::string strRTrim(std::string instr) const
		{
			if (instr.find_last_not_of(" \t") != std::string::npos)
				return instr.erase(instr.find_last_not_of(" \t") + 1);
			else
				return instr;
		}

		std::string strTrim(std::string instr) const
		{
			return strLTrim(strRTrim(instr));
		}

    	///replace "\n" with <CR>
		std::string UnEscape(const std::string & input) const
		{
			std::string outputstr;
			for (unsigned int i = 0; i < input.length(); i ++)
			{
				if ((unsigned int)(i + 1) == input.length())
					outputstr.append(1,input[i]);
				else
				{
					if (input[i] == '\\' && input[i+1] == 'n')
					{
						outputstr.append(1,'\n');
						++i;
					}
					else
					{
						outputstr.append(1,input[i]);
					}
				}
			}
			return outputstr;
		}
		
		template <typename T>
		bool ReadData(const std::string & name, T & i)
		{
			serialization_location_.push_back(name);
			std::string string_representation;
			if (!parsed_data_tree_.GetLeaf(serialization_location_, string_representation))
			{
				if (error_output_)
				{
					*error_output_ << "Error serializing " << parsed_data_tree_.Implode(serialization_location_) << " as it does not exist in the parsed data" << std::endl;
					*error_output_ << "Full dump of parsed data follows:" << std::endl;
					parsed_data_tree_.Print(*error_output_);
					*error_output_ << "Full dump of parsed data ends." << std::endl;
				}
				return false;
			}
			
			std::stringstream converter(string_representation);
			converter >> i;
			
			serialization_location_.pop_back();
			
			return true;
		}
		
		bool ReadStringData(const std::string & name, std::string & i)
		{
			serialization_location_.push_back(name);
			if (!parsed_data_tree_.GetLeaf(serialization_location_, i))
			{
				if (error_output_)
				{
					*error_output_ << "Error serializing " << parsed_data_tree_.Implode(serialization_location_) << " as it does not exist in the parsed data" << std::endl;
					*error_output_ << "Full dump of parsed data follows:" << std::endl;
					parsed_data_tree_.Print(*error_output_);
					*error_output_ << "Full dump of parsed data ends." << std::endl;
				}
				return false;
			}
			
			serialization_location_.pop_back();
			
			return true;
		}
		
	protected:
		virtual void ComplexTypeStart(const std::string & name)
		{
			serialization_location_.push_back(name);
		}
		
		virtual void ComplexTypeEnd(const std::string & name)
		{
			serialization_location_.pop_back();
		}
		
	public:
		TextInputSerializer() : error_output_(NULL) {}
		TextInputSerializer(std::istream & in) : error_output_(NULL) {Parse(in);}
		
		void set_error_output(std::ostream & value)
		{
			error_output_ = &value;
		}
		
		///this must be done before an object is serialized. returns true on success
		bool Parse(std::istream & in)
		{
			std::deque <std::string> tree_location;
			
			//the parse loop
			while (in)
			{
				ConsumeWhitespace(in); //get rid of cruft
				std::string line = SeekTo(in, '\n'); //read in this line
				std::stringstream linestream(line);
				std::string name = SeekTo(linestream, '=');
				if (name.length() == line.length()) //no assignment, so this must be the start or end of a complex type
				{
					name = strTrim(name);
					if (name == "{")
					{
						//ignore redundant information
					}
					else if (name == "}")
					{
						if (tree_location.empty()) //error: too many } in stream
						{
							if (error_output_)
							{
								*error_output_ << "Error parsing; encountered too many levels of }" << std::endl;
							}
							return false;
						}
						else
							tree_location.pop_back();
					}
					else
					{
						tree_location.push_back(name);
					}
				}
				else //assignment of non-complex type
				{
					name = strTrim(name);
					tree_location.push_back(name);
					std::string loc = parsed_data_tree_.Implode(tree_location); //useful for debugging only
					std::string value = SeekTo(linestream, '\n');
					value = strTrim(value);
					parsed_data_tree_.SetLeaf(tree_location, value);
					tree_location.pop_back();
				}
			}
			
			serialization_location_.clear();
			
			return true;
		}
		
		virtual bool Serialize(const std::string & name, int & i)
		{
			return ReadData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, unsigned int & i)
		{
			return ReadData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, float & i)
		{
			return ReadData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, double & i)
		{
			return ReadData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, std::string & i)
		{
			if (!ReadStringData(name, i)) return false;
			i = UnEscape(i);
			return true;
		}

		const TreeMap< std :: string > & parsed_data_tree() const
		{
			return parsed_data_tree_;
		}
};

///space efficient, very simple binary format with no provisions for validity checks. data is always written in big-endian format.
class BinaryOutputSerializer : public SerializerOutput
{
	private:
		std::ostream & out_;
		const bool bigendian_;
		
		bool IsBigEndian() const
		{
			short word = 0x4321;
			if((*(char *)& word) != 0x21 )
				return true;
			else
				return false;
		}
		
		void ByteSwap(unsigned char * b, int n) const
		{
			register int i = 0;
			register int j = n-1;
			while (i<j)
			{
				std::swap(b[i], b[j]);
				++i;  --j;
			}
		}
		
		template <typename T>
		bool WriteData(const std::string & name, const T & i)
		{
			if (!bigendian_)
			{
				T temp(i);
				ByteSwap(reinterpret_cast<unsigned char *>(&temp),sizeof(T));
				out_.write(reinterpret_cast<const char *>(&temp),sizeof(T));
			}
			else
				out_.write(reinterpret_cast<const char *>(&i),sizeof(T));
			
			return !out_.bad();
		}
		
		bool WriteStringData(const std::string & name, const std::string & i)
		{
			unsigned int strlen = i.length();
			if (!WriteData("length", strlen)) return false;
			out_.write(i.c_str(),strlen*sizeof(char));
			return !out_.bad();
		}
		
	public:
		BinaryOutputSerializer(std::ostream & newout) : out_(newout),bigendian_(IsBigEndian()) {}
		
		virtual bool Serialize(const std::string & name, int & i)
		{
			return WriteData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, unsigned int & i)
		{
			return WriteData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, float & i)
		{
			return WriteData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, double & i)
		{
			return WriteData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, std::string & i)
		{
			return WriteStringData(name, i);
		}
};

///space efficient, very simple binary format with no provisions for validity checks. data is always in big-endian format. fields are implied by their ordering and therefore this format is intolerant of data that doesn't exactly match the expected. due to this it's dangerous to read corrupted data.
class BinaryInputSerializer : public SerializerInput
{
	private:
		std::istream & in_;
		const bool bigendian_;
		
		bool IsBigEndian() const
		{
			short word = 0x4321;
			if((*(char *)& word) != 0x21 )
				return true;
			else
				return false;
		}
		
		void ByteSwap(unsigned char * b, int n) const
		{
			register int i = 0;
			register int j = n-1;
			while (i<j)
			{
				std::swap(b[i], b[j]);
				++i;  --j;
			}
		}
		
		template <typename T>
		bool ReadData(const std::string & name, T & i)
		{
			if (in_.eof()) return false;
			in_.read(reinterpret_cast<char *>(&i),sizeof(T));
			if (in_.fail() || in_.gcount() != sizeof(T)) return false;

			if (!bigendian_)
			{
				ByteSwap(reinterpret_cast<unsigned char *>(&i),sizeof(T));
			}
			
			return true;
		}
		
		bool ReadStringData(const std::string & name, std::string & i)
		{
			int strlen = 0;
			if (!ReadData("length", strlen)) return false;
			char * inbuffer = new char[strlen+1];
			inbuffer[strlen] = '\0';
			in_.read(inbuffer,strlen*sizeof(char));
			if (in_.fail() || in_.gcount() != (int)(strlen*sizeof(char)))
			{
				delete [] inbuffer;
				return false;
			}
			i.clear();
			for (int n = 0; n < strlen; ++n)
			{
				i.push_back(inbuffer[n]);
			}
			delete [] inbuffer;
			return true;
		}
		
	public:
		BinaryInputSerializer(std::istream & newin) : in_(newin),bigendian_(IsBigEndian()) {}
		
		virtual bool Serialize(const std::string & name, int & i)
		{
			return ReadData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, unsigned int & i)
		{
			return ReadData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, float & i)
		{
			return ReadData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, double & i)
		{
			return ReadData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, std::string & i)
		{
			return ReadStringData(name, i);
		}
};

///serializer that provides a reflection interface so the application can use dynamic programming techniques.  the treemap class is used to store data.  this can be either an output or input serializer depending on its mode.  note that internally all data is stored as strings.  this class should not be used via the normal serialization method, but instead use the ReadFromObject and WriteToObject functions.
class ReflectionSerializer : public Serializer
{
	private:
		TreeMap <std::string> reflection_data_tree_;
		Direction direction_;
		std::deque <std::string> serialization_location_;
		std::ostream * error_output_;
		
		template <typename T>
		void ConvertStringToOther(const std::string & string_representation, T & i) const
		{
			std::stringstream converter(string_representation);
			converter >> i;
		}
		
		void ConvertStringToOther(const std::string & string_representation, std::string & i) const
		{
			i = string_representation;
		}
		
		template <typename T>
		std::string ConvertOtherToString(T & i) const
		{
			std::stringstream converter;
			converter << i;
			return converter.str();
		}
		
		std::string ConvertOtherToString(float & i) const
		{
			std::stringstream converter;
			converter << std::scientific << std::setprecision (20) << i;
			return converter.str();
		}
		
		std::string ConvertOtherToString(double & i) const
		{
			std::stringstream converter;
			converter << std::scientific << std::setprecision (20) << i;
			return converter.str();
		}
		
		std::string ConvertOtherToString(std::string & i) const
		{
			return i;
		}
		
		template <typename T>
		bool ReadWriteData(const std::string & name, T & i)
		{
			serialization_location_.push_back(name);
			
			if (direction_ == DIRECTION_INPUT)
			{
				std::string string_representation;
				if (!reflection_data_tree_.GetLeaf(serialization_location_, string_representation))
				{
					if (error_output_)
					{
						*error_output_ << "Error serializing " << reflection_data_tree_.Implode(serialization_location_) << " as it does not exist in the reflection tree" << std::endl;
						*error_output_ << "Full dump of reflection data follows:" << std::endl;
						reflection_data_tree_.Print(*error_output_);
						*error_output_ << "Full dump of reflection data ends." << std::endl;
					}
					return false;
				}
				
				ConvertStringToOther(string_representation, i);
			}
			else if (direction_ == DIRECTION_OUTPUT)
			{
				std::string string_representation = ConvertOtherToString(i);
				reflection_data_tree_.SetLeaf(serialization_location_, string_representation);
			}
			
			serialization_location_.pop_back();
			
			return true;
		}
		
	protected:
		virtual Direction GetIODirection() {return direction_;}
		
		virtual void ComplexTypeStart(const std::string & name)
		{
			serialization_location_.push_back(name);
		}
		
		virtual void ComplexTypeEnd(const std::string & name)
		{
			serialization_location_.pop_back();
		}
		
	public:
		ReflectionSerializer() : direction_(DIRECTION_INPUT), error_output_(NULL) {}
		
		void set_error_output(std::ostream & value)
		{
			error_output_ = &value;
		}
		
		void TurnOffErrorOutput()
		{
			error_output_ = NULL;
		}
		
		///get a list of branches at the given location.  returns true on success.
		bool GetBranches(const std::deque <std::string> & location, std::vector <std::string> & output) const
		{
			const TreeMap <std::string> * branch = reflection_data_tree_.GetBranch(location);
			if (!branch)
			{
				if (error_output_)
				{
					*error_output_ << "Error getting branch " << reflection_data_tree_.Implode(location) << " as it does not exist in the reflection tree" << std::endl;
					*error_output_ << "Full dump of reflection data follows:" << std::endl;
					reflection_data_tree_.Print(*error_output_);
					*error_output_ << "Full dump of reflection data ends." << std::endl;
				}
				return false;
			}
			else
			{
				output = branch->GetBranches();
				return true;
			}
		}
		
		///read from the object into our reflection storage.  return true on success.
		template <typename T>
		bool ReadFromObject(T & object) {direction_ = DIRECTION_OUTPUT; reflection_data_tree_.clear(); serialization_location_.clear(); return Serializer::Serialize(object);}
		
		///write from our reflection storage to the object.  return true on success.
		template <typename T>
		bool WriteToObject(T & object) {direction_ = DIRECTION_INPUT; serialization_location_.clear(); return Serializer::Serialize(object);}
		
		virtual bool Serialize(const std::string & name, int & i)
		{
			return ReadWriteData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, unsigned int & i)
		{
			return ReadWriteData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, float & i)
		{
			return ReadWriteData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, double & i)
		{
			return ReadWriteData(name, i);
		}
		
		virtual bool Serialize(const std::string & name, std::string & i)
		{
			return ReadWriteData(name, i);
		}
		
		///gets the requested variable and sets the output argument to it, or if an error occurred, does not modify the output argument. returns true on success.
		template <typename T>
		bool Get ( const std::deque <std::string> & location, T & output ) const
		{
			std::string string_representation;
			if (!GetString(location, string_representation))
				return false;
			ConvertStringToOther(string_representation, output);
			
			return true;
		}
		
		///gets the requested variable in a string representation and sets the output argument.  returns true on success.
		bool GetString ( const std::deque <std::string> & location, std::string & output ) const
		{
			std::string string_representation;
			if (!reflection_data_tree_.GetLeaf(location, string_representation))
			{
				if (error_output_)
				{
					*error_output_ << "Error getting " << reflection_data_tree_.Implode(location) << " as it does not exist in the reflection tree" << std::endl;
					*error_output_ << "Full dump of reflection data follows:" << std::endl;
					reflection_data_tree_.Print(*error_output_);
					*error_output_ << "Full dump of reflection data ends." << std::endl;
				}
				return false;
			}
			
			output = string_representation;
			
			return true;
		}
		
		///sets the requested variable to the given value, or if an error occurred, does not modify it. this Set function only allows setting of existing variables; see Add for an alternative that allows setting of existing or new variables. returns true on success.
		template <typename T>
		bool Set ( const std::deque <std::string> & location, const T & value )
		{
			std::string string_representation = ConvertOtherToString(value);
			return SetString(location, string_representation);
		}
		
		///alternative to Set that takes a string representation of the type.  returns true on success.
		bool SetString(const std::deque <std::string> & location, const std::string & valuestr)
		{
			TreeMap <std::string> * branch = reflection_data_tree_.GetBranch(location);
			
			if (!branch)
			{
				if (error_output_)
				{
					*error_output_ << "Error setting " << reflection_data_tree_.Implode(location) << " as it does not exist in the reflection tree" << std::endl;
					*error_output_ << "Full dump of reflection data follows:" << std::endl;
					reflection_data_tree_.Print(*error_output_);
					*error_output_ << "Full dump of reflection data ends." << std::endl;
				}
				return false;
			}
			
			branch->set_leaf(valuestr);
			
			return true;
		}
		
		///sets the requested variable to the given value, and if it does not exist, creates the variable; note that this may result in errors when serializing back to the object unless you know what you're doing. see the Set function for a version that won't automatically create the variable if it doesn't exist. returns true on success.
		template <typename T>
		bool Add ( const std::deque <std::string> & location, const T & value )
		{
			return AddString(location, ConvertOtherToString(value));
		}
		
		///alternative to Add that takes a string representation.  returns true on success.
		bool AddString ( const std::deque <std::string> & location, const std::string & value )
		{
			reflection_data_tree_.SetLeaf(location, value);
			
			return true;
		}
		
		///generate a deque of strings from a delimited string
		std::deque <std::string> Explode(const std::string & delimited_string, char delimiter='.') const
		{
			std::deque <std::string> container;
			std::stringstream instr(delimited_string);
			
			while (instr)
			{
				std::string text;
				std::getline(instr,text,delimiter);
				if (!text.empty())
					container.push_back(text);
			}
			
			return container;
		}
		
		///debug print of the reflection data map
		void Print(std::ostream & output)
		{
			reflection_data_tree_.Print(output);
		}
		
		///merge the other tree into our reflection tree
		void Merge(const TreeMap <std::string> & othertree)
		{
			reflection_data_tree_.Merge(othertree);
		}
};

//utility functions

///returns true on success
template <typename T>
bool WriteObjectToFile(const std::string & path, T & object, std::ostream & info_output, bool binary=false)
{
	std::ofstream outfile(path.c_str(), binary ? std::ios_base::binary : std::ios_base::out);
	bool error = false;
	if (outfile)
	{
		if (binary)
		{
			joeserialize::BinaryOutputSerializer out(outfile);
			error = !object.Serialize(out);
		}
		else
		{
		joeserialize::TextOutputSerializer out(outfile);
		error = !object.Serialize(out);
	}
	}
	else
		error = true;
	
	if (error)
		info_output << "Could not write to file " << path << std::endl;
	else
		info_output << "Wrote " << path << std::endl;
	
	return !error;
}

///returns true on success
template <typename T>
bool LoadObjectFromFile(const std::string & path, T & object, std::ostream & info_output, bool binary=false)
{
	info_output << "Loading " << path << "..." << std::endl;

	std::ifstream infile(path.c_str(), binary ? std::ios_base::binary : std::ios_base::in);
	bool error = false;
	if (infile)
	{
		if (binary)
		{
			joeserialize::BinaryInputSerializer in(infile);
			if (object.Serialize(in))
			{
				info_output << "Loaded " << path << std::endl;
			}
			else
			{
				info_output << "File " << path << " had serialization errors" << std::endl;
				error = true;
			}
		}
		else
		{
			joeserialize::TextInputSerializer in;
			in.set_error_output(info_output);
			std::istream & i = infile;
			if (!in.Parse(i))
			{
				info_output << "File " << path << " had parsing errors" << std::endl;
				error = true;
			}
			else
			{
				if (object.Serialize(in))
				{
					info_output << "Loaded " << path << std::endl;
				}
				else
				{
					info_output << "File " << path << " had serialization errors; trying to upgrade it" << std::endl;
					error = true;
				}
			}
		}
	}
	else
	{
		error = true;
		info_output << "File " << path << " not found" << std::endl;
	}
	
	return !error;
}

template <typename T>
void LoadObjectFromFileOrCreateDefault(const std::string & path, T & object, std::ostream & info_output)
{
	info_output << "Loading " << path << "..." << std::endl;

	std::ifstream infile(path.c_str());
	bool error = false;
	if (infile)
	{
		joeserialize::TextInputSerializer in;
		in.set_error_output(info_output);
		std::istream & i = infile;
		if (!in.Parse(i))
		{
			info_output << "File " << path << " had parsing errors; creating default" << std::endl;
			error = true;
		}
		else
		{
			if (object.Serialize(in))
			{
				info_output << "Loaded " << path << std::endl;
			}
			else
			{
				info_output << "File " << path << " had serialization errors; upgrading it" << std::endl;
				object = T(); //create default object
				joeserialize::ReflectionSerializer reflect;
				if (!reflect.ReadFromObject(object))
				{
					error = true;
					info_output << "File " << path << " couldn't be upgraded during reflection read; creating default" << std::endl;
				}
				else
				{
					reflect.Merge(in.parsed_data_tree());
					if (!reflect.WriteToObject(object))
					{
						error = true;
						info_output << "File " << path << " couldn't be upgraded during reflection write; creating default" << std::endl;
					}
					else
						WriteObjectToFile(path, object, info_output);
				}
			}
		}
	}
	else
	{
		error = true;
		info_output << "File " << path << " not found; creating default" << std::endl;
	}
	
	if (error)
	{
		object = T();
		WriteObjectToFile(path, object, info_output);
	}
}

}
