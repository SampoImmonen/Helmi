#include "ScopedTimer.h"

namespace helmirt {

helmirt::ScopedTimer::ScopedTimer(const std::string& scope, bool toFile)
{
	writetoFile = toFile;
	m_StartTimepoint = std::chrono::high_resolution_clock::now();
	m_scope = scope;
}

helmirt::ScopedTimer::~ScopedTimer()
{
	stop();
}

void helmirt::ScopedTimer::stop()
{
	auto EndTimepoint = std::chrono::high_resolution_clock::now();

	auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
	auto end = std::chrono::time_point_cast<std::chrono::microseconds>(EndTimepoint).time_since_epoch().count();

	std::cout << m_scope << "\n";
	auto duration = end - start;
	double ms = duration * 0.001;
	std::cout << "Timer:\n";
	std::cout << " " << duration << "us\n";
	std::cout << " " << ms << "ms\n";
}
}
