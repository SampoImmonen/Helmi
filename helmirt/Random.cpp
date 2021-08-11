#include "Random.h"

Random::Random(): m_engine(std::mt19937(m_r()))
{
	
}

Random::Random(float seed): m_engine(std::mt19937(seed))
{

}
