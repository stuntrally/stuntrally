#ifndef _RANDOM_H
#define _RANDOM_H

//#include <stdio.h>

//#include <stdlib.h>
//#include <time.h>

class DETERMINISTICRANDOM
{
private:
    unsigned int seed;

public:
	DETERMINISTICRANDOM() : seed(0) {}
	void ReSeed() {seed = time(NULL) % 2048;}
	void ReSeed(unsigned int newseed) {seed = newseed % 2048;}

	double Get();
	double Peek() const;
};

class RANDOM
{
private:
    unsigned int seed;

public:
	RANDOM() : seed(0) {}
	void ReSeed() {seed = time(NULL);srand(seed);}
	void ReSeed(unsigned int newseed) {seed = newseed;srand(newseed);}

	double Get() {return (double) rand()/RAND_MAX;}
};

#endif
