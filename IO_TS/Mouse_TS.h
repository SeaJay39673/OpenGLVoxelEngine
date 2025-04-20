#ifndef MOUSE_TS_H
#define MOUSE_TS_H

#include <mutex>

using std::mutex, std::lock_guard;

class Mouse_TS
{
private:
    static mutex MouseMutex;
    static double pos[2];
    static double del[2];
    static bool firstMouse;

public:
    Mouse_TS()
    {
        firstMouse = true;
        double temp[2] = {0, 0};
        memcpy(pos, temp, sizeof(temp));
        memcpy(del, temp, sizeof(temp));
    }
    static const double *GetMouseDeltas()
    {
        lock_guard<mutex> lock(MouseMutex);
        return del;
    }

    static void ResetMouseDeltas()
    {
        lock_guard<mutex> lock(MouseMutex);
        double temp[2] = {0, 0};
        memcpy(del, temp, sizeof(temp));
    }

    static void MouseCallback(double xpos, double ypos)
    {
        lock_guard<mutex> lock(MouseMutex);
        if (firstMouse)
        {
            pos[0] = xpos;
            pos[1] = ypos;
            firstMouse = false;
            return;
        }
        del[0] = xpos - pos[0];
        del[1] = ypos - pos[1];
        pos[0] = xpos;
        pos[1] = ypos;
    }
};

mutex Mouse_TS::MouseMutex;
double Mouse_TS::pos[2];
double Mouse_TS::del[2];
bool Mouse_TS::firstMouse = true;

#endif