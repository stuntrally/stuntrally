#pragma once
#include <map>

template <typename T>
class PAIRSORTER_FIRST
{
	public:
	bool operator() (const std::pair <T, T> & p1, const std::pair <T, T> & p2) const
	{
		return p1.first < p2.first;
	}
};

template <typename T>
class PAIRSORTER_SECOND
{
	public:
	bool operator() (const std::pair <T, T> & p1, const std::pair <T, T> & p2) const
	{
		return p1.second < p2.second;
	}
};
