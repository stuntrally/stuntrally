#pragma once
//#include <iostream>
//#include <string>

namespace logging
{
	class logstreambuf : public std::streambuf
	{
		public:
			typedef int int_type;
			typedef std::char_traits<char> traits_type;

		protected:
			std::string buffer;
			const std::string prefix;
			std::ostream & forwardstream;

		public:
			logstreambuf(const std::string & newprefix, std::ostream & forwardee) : 
				prefix(newprefix),forwardstream(forwardee) {}
			
			std::string str() const
			{
				std::string ret;
				if (this->pptr())
				{
					ret = std::string(this->pbase(), this->pptr());
				}
				else
					ret = buffer;
				return ret;
			}

		protected:
			virtual int_type overflow(int_type c = traits_type::eof())
			{
			
				const std::string::size_type capacity = buffer.capacity();
				const std::string::size_type max_size = buffer.max_size();
			
				/*std::cout << "Overflow: \"" << (char)c << "\" (" << (int)c << ")" << std::endl;
				std::cout << "Capacity: " << capacity << std::endl;
				std::cout << "pptr: " << this->pptr() - this->pbase() << std::endl;
				std::cout << "epptr: " << this->epptr() - this->pbase() << std::endl;
				std::cout << "alignment check: " << this->pbase() - buffer.data() << std::endl;*/
			
				// Try to append __c into output sequence in one of two ways.
				// Order these tests done in is unspecified by the standard.
				const char conv = traits_type::to_char_type(c);
				if (this->pptr() >= this->epptr())
				{
					// NB: Start ostringstream buffers at 512 chars.  This is an
					// experimental value (pronounced "arbitrary" in some of the
					// hipper english-speaking countries), and can be changed to
					// suit particular needs.
						//
					// _GLIBCXX_RESOLVE_LIB_DEFECTS
					// 169. Bad efficiency of overflow() mandated
					// 432. stringbuf::overflow() makes only one write position
					//      available
					std::string::size_type opt_len = std::max(std::string::size_type(2 * capacity),
							std::string::size_type(512));
					if (this->epptr() - this->pbase() < (int) capacity)
						opt_len = capacity; 
					const std::string::size_type len = std::min(opt_len, max_size);
					std::string tmp;
					tmp.reserve(len);
					if (this->pbase())
						tmp.assign(this->pbase(), this->epptr() - this->pbase());
					tmp.push_back(conv);
					buffer.swap(tmp);
					std::string::size_type o = this->pptr() - this->pbase();
					char * newbasep = const_cast<char_type*>(buffer.data());
					this->setp(newbasep, newbasep + buffer.capacity());
					this->pbump(o);
				}
				else
					*this->pptr() = conv;
				this->pbump(1);
				return c;
			}
		
			virtual int_type sync()
			{
				std::string myoutput = prefix + str();
				for (int i = 0; i < (int)myoutput.length()-1; i++)
				{
					if (myoutput[i] == '\n')
						myoutput.insert(i+1,prefix.length(),' ');
				}
			
				forwardstream << myoutput << std::flush;
				char * newbasep = const_cast<char_type*>(buffer.data());
				this->setp(newbasep, newbasep+buffer.capacity());
				pbump(newbasep-this->pptr());
				return 0;
			}
	};

	class splitterstreambuf : public std::streambuf
	{
		public:
			typedef int int_type;
			typedef std::char_traits<char> traits_type;

		protected:
			std::string buffer;
			std::ostream & forwardstream1;
			std::ostream & forwardstream2;

		public:
			splitterstreambuf (std::ostream & forwardee1, std::ostream & forwardee2) : 
				forwardstream1(forwardee1),forwardstream2(forwardee2) {}
			
			std::string str() const
			{
				std::string ret;
				if (this->pptr())
				{
					ret = std::string(this->pbase(), this->pptr());
				}
				else
					ret = buffer;
				return ret;
			}

		protected:
			virtual int_type overflow(int_type c = traits_type::eof())
			{
			
				const std::string::size_type capacity = buffer.capacity();
				const std::string::size_type max_size = buffer.max_size();
			
				/*std::cout << "Overflow: \"" << (char)c << "\" (" << (int)c << ")" << std::endl;
				std::cout << "Capacity: " << capacity << std::endl;
				std::cout << "pptr: " << this->pptr() - this->pbase() << std::endl;
				std::cout << "epptr: " << this->epptr() - this->pbase() << std::endl;
				std::cout << "alignment check: " << this->pbase() - buffer.data() << std::endl;*/
			
				// Try to append __c into output sequence in one of two ways.
				// Order these tests done in is unspecified by the standard.
				const char conv = traits_type::to_char_type(c);
				if (this->pptr() >= this->epptr())
				{
					// NB: Start ostringstream buffers at 512 chars.  This is an
					// experimental value (pronounced "arbitrary" in some of the
					// hipper english-speaking countries), and can be changed to
					// suit particular needs.
						//
					// _GLIBCXX_RESOLVE_LIB_DEFECTS
					// 169. Bad efficiency of overflow() mandated
					// 432. stringbuf::overflow() makes only one write position
					//      available
					std::string::size_type opt_len = std::max(std::string::size_type(2 * capacity),
							std::string::size_type(512));
					if (this->epptr() - this->pbase() < (int)capacity)
						opt_len = capacity; 
					const std::string::size_type len = std::min(opt_len, max_size);
					std::string tmp;
					tmp.reserve(len);
					if (this->pbase())
						tmp.assign(this->pbase(), this->epptr() - this->pbase());
					tmp.push_back(conv);
					buffer.swap(tmp);
					std::string::size_type o = this->pptr() - this->pbase();
					char * newbasep = const_cast<char_type*>(buffer.data());
					this->setp(newbasep, newbasep + buffer.capacity());
					this->pbump(o);
				}
				else
					*this->pptr() = conv;
				this->pbump(1);
				return c;
			}
		
			virtual int_type sync()
			{
				forwardstream1 << str() << std::flush;
				forwardstream2 << str() << std::flush;
				char * newbasep = const_cast<char_type*>(buffer.data());
				this->setp(newbasep, newbasep+buffer.capacity());
				pbump(newbasep-this->pptr());
				return 0;
			}
	};
}
