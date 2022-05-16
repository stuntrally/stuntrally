#pragma once
#include <vector>
#include <map>
#include <cassert>
#include <string>

namespace HASH_NAMESPACE
{

template <class T>
class hasher
{
public:
	unsigned int GetHash(T & val) const {assert(0);return 0;} //no hash function specified for this type
};

//DJB hash from https://www.partow.net/programming/hashfunctions/
//very simple, excellent performance
//the one at https://burtleburtle.net/bob/c/lookup3.c sounds good, but seems much slower
template <>
class hasher <std::string>
{
	private:
	
	public:
		unsigned int GetHash(const std::string & str) const
		{
			unsigned int hash = 5381;
			unsigned int strlen = str.length();
			for(std::size_t i = 0; i < strlen; ++i)
			{
				hash = ((hash << 5) + hash) + str[i];
			}

			return (hash & 0x7FFFFFFF);
		}
};

template <>
class hasher <int>
{
	private:
	
	public:
		unsigned int GetHash(int & key) const
		{
			unsigned int retval = *((unsigned int*) (&key));
			return retval;
		}
};

template <>
class hasher <float>
{
	private:
	
	public:
		unsigned int GetHash(float & key) const
		{
			unsigned int retval = *((unsigned int *) (&key));
			return retval;
		}
};

template <>
class hasher <double>
{
	private:
	
	public:
		unsigned int GetHash(double & key) const
		{
			//unsigned int retval = *((unsigned int *) (&key) + (sizeof(double)/2));
			unsigned int retval = *((unsigned int *) (&key));
			return retval;
		}
};

template <>
class hasher <unsigned int>
{
	private:
	
	public:
		unsigned int GetHash(unsigned int & key) const
		{
			return key;
		}
};

template <class KEYCLASS, class DATACLASS, class VI_ITERATOR, class MI_ITERATOR>
class hash_iterator
{
	public:
		typedef std::forward_iterator_tag iterator_category;
		typedef DATACLASS value_type;
		typedef value_type & reference;
		typedef value_type * pointer;
		typedef int difference_type;
	
	private:
		VI_ITERATOR vi;
		VI_ITERATOR data_end;
		MI_ITERATOR mi;
		
	public:
		hash_iterator(VI_ITERATOR vi_cur, VI_ITERATOR vi_end) : vi(vi_cur), data_end(vi_end)
		{
			while (vi != data_end && vi->empty()) //move vi to first non-empty bucket
			{
				++vi;
				if (vi != data_end)
					mi = vi->begin();
			}
			if (vi != data_end)
				mi = vi->begin();
		}
		
		pointer operator->() const
		{
			//is it OK to assert inside an iterator?  well, here goes nothin'.
			assert (vi != data_end);
			assert (mi != vi->end());
			
			return &(operator*());
		}

		reference operator*() const
		{
			//is it OK to assert inside an iterator?  well, here goes nothin'.
			assert (vi != data_end);
			assert (mi != vi->end());
			
			return mi->second; //only valid if mi and vi are valid
		}

		hash_iterator& operator++()
		{
			if (vi == data_end) //vi is at the end of buckets already
				return (*this);
			
			if (mi != vi->end()) //mi is valid
			{
				++mi;
			
				if (mi == vi->end()) //we moved to the end of our bucket
				{
					++vi;
					while (vi != data_end && vi->empty()) //move vi to next non-empty bucket
					{
						++vi;
						if (vi != data_end) //found a bucket
							mi = vi->begin();
					}
					if (vi != data_end)
						mi = vi->begin();
				}
			}
			
			return (*this);
		}

		hash_iterator operator++(int)
		{
			// postincrement
			hash_iterator _Tmp = *this;
			++*this;
			return (_Tmp);
		}

		bool operator==(const hash_iterator & other) const
		{
			// test for iterator equality
			if (vi == data_end && other.vi == other.data_end)
				return true;
			
			return (vi == other.vi && mi == other.mi);
		}

		bool operator!=(const hash_iterator & other) const
		{
			// test for iterator inequality
			return (!(*this == other));
		}
		
		const KEYCLASS & first() const
		{
			assert (vi != data_end);
			assert (mi != vi->end());
			
			return mi->first; //only valid if mi and vi are valid
		}
};

} //namespace HASH_NAMESPACE

template <class KEYCLASS, class DATACLASS>
class bucketed_hashmap
{
private:
	unsigned int nbuckets;
	std::vector < std::map <KEYCLASS, DATACLASS> > data;
	void set_buckets(unsigned int num_buckets) {data.resize(num_buckets);nbuckets = num_buckets;}
	HASH_NAMESPACE::hasher <KEYCLASS> hash;
	
public:
	typedef HASH_NAMESPACE::hash_iterator <KEYCLASS, DATACLASS,
		typename std::vector < std::map <KEYCLASS, DATACLASS> >::iterator,
						typename std::map <KEYCLASS, DATACLASS>::iterator> iterator;
	typedef HASH_NAMESPACE::hash_iterator <const KEYCLASS, const DATACLASS,
		typename std::vector < std::map <KEYCLASS, DATACLASS> >::const_iterator,
						typename std::map <KEYCLASS, DATACLASS>::const_iterator> const_iterator;
	
	bucketed_hashmap() {set_buckets(256);}
	bucketed_hashmap(unsigned int num_buckets) {set_buckets(num_buckets);}
	
	iterator begin()
	{
		return iterator(data.begin(), data.end());
	}
	
	const_iterator begin() const
	{
		return const_iterator(data.begin(), data.end());
	}
	
	iterator end()
	{
		return iterator(data.end(), data.end());
	}

	const_iterator end() const
	{
		return const_iterator(data.end(), data.end());
	}
	
	///returns true if the element was found and erased
	bool Erase(KEYCLASS & key)
	{
		unsigned int idx = hash.GetHash(key) % nbuckets;
		
		if (data[idx].find(key) == data[idx].end())
			return false;
		else
		{
			data[idx].erase(key);
			return true;
		}
	}
	
	///iterators will be invalidated
	void Clear()
	{
		for (auto i = data.begin(); i != data.end(); ++i)
		{
			i->clear();
		}
	}
	
	const DATACLASS * Get(KEYCLASS & key) const
	{
		unsigned int idx = hash.GetHash(key) % nbuckets;
		//typedef std::map <KEYCLASS, DATACLASS> mymap;
		//mymap::iterator i;// = data[idx].find(key);
		/*if (i == data[idx].end())
			return NULL;
		else
			return i;*/
		
		if (data[idx].find(key) == data[idx].end())
			return NULL;
		else
			return &(((data[idx]).find(key))->second);
	}
	
	DATACLASS * Get(KEYCLASS & key)
	{
		unsigned int idx = hash.GetHash(key) % nbuckets;
		//typedef std::map <KEYCLASS, DATACLASS> mymap;
		//mymap::iterator i;// = data[idx].find(key);
		/*if (i == data[idx].end())
		return NULL;
		else
		return i;*/
		
		if (data[idx].find(key) == data[idx].end())
			return NULL;
		else
			return &(((data[idx]).find(key))->second);
	}
	
	DATACLASS & Set(const KEYCLASS & key, const DATACLASS & ndata)
	{
		unsigned int idx = (hash.GetHash(key)) % nbuckets;
		(data[idx])[key] = ndata;
		return (data[idx])[key];
	}
	
	unsigned int GetNumCollisions()
	{
		unsigned int col = 0;
		unsigned int lim = data.size();
		
		for (unsigned int i = 0; i < lim; ++i)
		{
			unsigned int subsize = data[i].size();
			if (subsize > 1)
				col += subsize-1;
		}
		
		return col;
	}
	
	unsigned int GetTotalObjects() const
	{
		unsigned int obj = 0;
		//unsigned int lim = data.size();
		
		/*for (unsigned int i = 0; i < lim; ++i)
		{
			obj += data[i].size();
		}*/
		
		for (auto i = data.begin(); i != data.end(); ++i)
		{
			obj += i->size();
		}
		
		return obj;
	}
	
	int size() const {return GetTotalObjects();}
	
	float GetAvgBucketSize()
	{
		float avg = 0;
		unsigned int lim = data.size();
		
		for (unsigned int i = 0; i < lim; ++i)
		{
			avg += data[i].size();
		}
		
		return avg / nbuckets;
	}
	
	float GetBucketEvenness()
	{
		return (GetTotalObjects() - (float) GetNumCollisions()) / nbuckets;
	}
	
	float GetEmptyBucketPercent()
	{
		unsigned int lim = data.size();
		unsigned int empty = 0;
		
		for (unsigned int i = 0; i < lim; ++i)
		{
			if (data[i].empty())
				empty++;
		}
		
		return (float) empty / (float) nbuckets;
	}
	
	unsigned int GetLongestBucket()
	{
		unsigned int lim = data.size();
		unsigned int longest = 0;
		
		for (unsigned int i = 0; i < lim; ++i)
		{
			if (data[i].size() > longest)
				longest = data[i].size();
		}
		
		return longest;
	}
};
