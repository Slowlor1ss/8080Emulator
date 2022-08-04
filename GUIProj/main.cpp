#include <chrono>
#include "i8080GUI.h"

using namespace std::chrono;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    i8080GUI i8080GUI;
    i8080GUI.show();

    uint64_t lastUiUpdate{};
    time_point<steady_clock> currTime{};
    time_point<steady_clock> prevTime{};

    while (!i8080GUI.GetIsClosed())
    {
        currTime = steady_clock::now();
        const auto deltaTime = std::chrono::duration_cast<microseconds>(currTime - prevTime).count();
        prevTime = currTime;

        lastUiUpdate += deltaTime;
        if(lastUiUpdate >= 500)
        {
	        QApplication::processEvents(QEventLoop::EventLoopExec);
            lastUiUpdate = 0;
        }

    	i8080GUI.Update8080();
    }
    app.exit();
    return 0;
}



