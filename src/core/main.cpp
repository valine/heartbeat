#include <ui/zapplication.h>
#include <zmainviewcontroller.h>

int main(int argc, char* argv[]) {
    /**
     * Main application loop
     */
    ZApplication(new MainViewController(argv), "Heartbeat", true,
                 1920, 1080,1);
}
