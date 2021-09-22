#define TINYOBJLOADER_IMPLEMENTATION
#include "Timer.h"
#include "Application.h"


int main() {
	Application app;

	Instrumenter::Get().BeginSession("runtime");
	app.startApplication();
	Instrumenter::Get().EndSession();

	return 0;
}

