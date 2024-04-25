#include <ui/zapplication.h>
#include <zmainviewcontroller.h>

int main(int argc, char* argv[]) {
    /**
     * Main application loop
     */
    ZApplication(new MainViewController(argv), "Heartbeat", false,
                 960, 1080);
}
