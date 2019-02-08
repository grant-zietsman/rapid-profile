#include "../rapid-profile.hpp"

int main() 
{
    RAPID_PROFILE_INIT();

    INTERVAL(RUNTIME);

    INTERVAL(EMPTY);
    INTERVAL_END(EMPTY);

    INTERVAL(LOOP);
    for (int i = 0; i < 5; i++)
    {
        INTERVAL(REPEAT_OUTER);
        INTERVAL(REPEAT_INNER);
        INTERVAL_END(REPEAT_INNER);
        INTERVAL_END(REPEAT_OUTER);
    }
    INTERVAL_END(LOOP);

    INTERVAL(INCREMENT);
    int i = 2;
    i++;
    INTERVAL_END(INCREMENT);

    INTERVAL_END(RUNTIME);

    return i;
}
