#include "../rapid-profile.hpp"

#include <iostream>
#include <cstdlib>

int main() 
{
    RAPID_PROFILE_INIT();
    INTERVAL(RT);
    INTERVAL(TEST1);
    INTERVAL_END(TEST1);
    for (int i = 0; i < 5; i++)
    {
        INTERVAL(OUT);
        INTERVAL(IN);
        INTERVAL_END(IN);
        INTERVAL_END(OUT);
    }
    // while(true);
    INTERVAL(TEST);
    INTERVAL_END(TEST);
    INTERVAL_END(RT);
    return 0;
}
