#include <App.h>

int main()
{
	App app;
	if (app.init() == true)
	{
		app.main_loop();
	}

	return 0;
}