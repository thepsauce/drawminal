#include "screen.h"

int main(void)
{
    InitScreen();

    Panic("Actually I'm fine.");

    EndScreen();
    return 0;
}
