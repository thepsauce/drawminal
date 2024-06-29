#include "ui.h"

#include "screen.h"

int main(void) {
	InitScreen();

	RunUI();

	EndScreen();
	return 0;
}
