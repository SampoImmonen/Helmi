

#include "App.h"

int main()
{
	helmirt::App app(600, 800);
	app.renderRT();
	app.saveRTImage("image.ppm");
}