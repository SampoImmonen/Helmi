//from https://gist.github.com/TheCherno/31f135eea6ee729ab5f26a6908eb3a5e#file-instrumentor-h

/*
Usage

Make an Insturementer

Instrumenter::Get().Beginsession("name")
{
	PROFILE_FUNC()/PROFILE_SCOPE("Scope name")
}
Instrumenter::Get().Endsession()
*/


#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>

//Profiling Macros
#ifdef PROFILE
#define PROFILE_SCOPE(name) ScopedTimer t##__LINE__(name)
#define PROFILE_FUNC() PROFILE_SCOPE(__FUNCSIG__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNC()
#endif


struct ProfileResult {
	std::string name;
	long long start, end;
	uint32_t threadID;
};


struct InstrumentationSession {
	std::string name;
};

class Instrumenter {
public:
	Instrumenter() : m_currentSession(nullptr), m_profileCount(0)
	{
	}

	void BeginSession(const std::string& name, const std::string& filepath = "results.json")
	{
		m_outputStream.open(filepath);
		WriteHeader();
		m_currentSession = new InstrumentationSession{ name };

	}

	void EndSession()
	{
		WriteFooter();
		m_outputStream.close();
		delete m_currentSession;
		m_currentSession = nullptr;
		m_profileCount = 0;
	}

	void WriteProfile(const ProfileResult& result)
	{
		if (m_profileCount++ > 0)
			m_outputStream << ",";

		std::string name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		m_outputStream << "{";
		m_outputStream << "\"cat\":\"function\",";
		m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
		m_outputStream << "\"name\":\"" << name << "\",";
		m_outputStream << "\"ph\":\"X\",";
		m_outputStream << "\"pid\":0,";
		m_outputStream << "\"tid\":" << result.threadID << ",";
		m_outputStream << "\"ts\":" << result.start;
		m_outputStream << "}";

		m_outputStream.flush();
	}

	void WriteFooter()
	{
		m_outputStream << "]}";
		m_outputStream.flush();
	}

	void WriteHeader()
	{
		m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
		m_outputStream.flush();
	}

	static Instrumenter& Get()
	{
		static Instrumenter instance;
		return instance;
	}

private:
	InstrumentationSession* m_currentSession;
	std::ofstream m_outputStream;
	int m_profileCount;
};

class ScopedTimer
{
public:
	ScopedTimer(const char* name) : m_name(name), m_stopped(false)
	{
		m_startTimepoint = std::chrono::high_resolution_clock::now();
	}
	~ScopedTimer()
	{
		if (!m_stopped) {
			stop();
		}
	}

private:
	void stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		Instrumenter::Get().WriteProfile({ m_name, start, end, threadID });

		m_stopped = true;
	};

	const char* m_name;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
	bool m_stopped;
};



