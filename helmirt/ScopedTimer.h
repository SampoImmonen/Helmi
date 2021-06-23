#pragma once

#include <chrono>
#include <iostream>
#include <string>

namespace helmirt {

#ifdef PROFILE
#define PROFILE_SCOPE(name) Timer t##__LINE__(name)
#define PROFILE_FUNC() PROFILE_SCOPE(__FUNCSIG__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNC()
#endif



class ScopedTimer
{
public:
	ScopedTimer(const std::string& scope, bool toFile = false);
	~ScopedTimer();

private:
	void stop();
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
	bool writetoFile;
	std::string m_scope;
};
}

