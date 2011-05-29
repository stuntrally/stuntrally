#ifndef _CONTAINERALGORITHM_H
#define _CONTAINERALGORITHM_H

//wrap a bunch of the standard library algorithms to work on whole containers

#include <algorithm>
#include <functional>
#include <vector>
#include <map>
#include <cassert>

namespace calgo
{
	template<class Container, class T>
	typename Container::const_iterator find ( const Container & c, const T& value )
	{
		return std::find(c.begin(), c.end(), value);
	}
	
	template<class Container_in, class OutputIterator>
	OutputIterator copy ( const Container_in & c, OutputIterator result )
	{
		return std::copy(c.begin(), c.end(), result);
	}
	
	template<class Container_in>
	void sort ( Container_in & c )
	{
		std::sort(c.begin(), c.end());
	}
	
	template<class Container, class Function>
	Function for_each(Container & c, Function f)
	{
		return std::for_each(c.begin(), c.end(), f);
	}
	
	template <class Container1, class OutputIterator, class UnaryPredicate>
	OutputIterator copy_if(const Container1& container_in, OutputIterator result, UnaryPredicate pred)
	{
		typename Container1::const_iterator i = container_in.begin();
		typename Container1::const_iterator iend = container_in.end();
		for (; i != iend; ++i)
			if (pred( *i ))
				*result++ = *i;
		return result;
	}
	
	template < class Container, class OutputIterator, class UnaryOperator >
	OutputIterator transform ( const Container & c,
		OutputIterator result, UnaryOperator op )
	{
		return std::transform(c.begin(), c.end(), result, op);
	}
	
	template<class Container, class InputIterator, class T>
	InputIterator find ( const Container & c, const T& value )
	{
		return std::find(c.begin(), c.end(), value);
	}
	
	template<class T>
	void SwapAndPop(std::vector <T> & container, const std::vector <unsigned int> & todel)
	{
		if (todel.size() >= container.size())
		{
			container.clear();
		}
		else
		{
			std::map <unsigned int, unsigned int> remap;
			
			unsigned int orig_container_size = container.size();
			for (unsigned int i = 0; i < todel.size(); i++)
			{
				unsigned int endi = orig_container_size - i - 1;
				unsigned int todeli = todel[i];
				if (todeli > endi)
				{
					assert(remap.find(todeli) != remap.end());
					todeli = remap[todeli];
				}
				if (endi != todeli)
				{
					std::swap(container[endi], container[todeli]);
					remap[endi] = todeli;
				}
				container.pop_back();
			}
			
			/*std::vector <unsigned int> remap(container.size());
			for (unsigned int i = 0; i < remap.size(); i++)
				remap[i] = i;
			
			unsigned int orig_container_size = container.size();
			for (unsigned int i = 0; i < todel.size(); i++)
			{
				unsigned int endi = orig_container_size - i - 1;
				unsigned int todeli = remap[todel[i]];
				if (endi != todeli)
				{
					std::swap(container[endi], container[todeli]);
					remap[endi] = todeli;
				}
				container.pop_back();
			}*/
		}
	}
}

#endif
