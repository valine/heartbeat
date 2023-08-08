#include "ui/zviewcontroller.h"

#include "mesh/zscene.h"
#include "mesh/zpointlight.h"
#include "ui/ztexture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <atomic>
#include "mesh/zobject.h"
#include "ui/zchart.h"
#include "ui/zscrollview.h"
#include "ui/zlinechart.h"

static const int MARGIN = 10;
using namespace glm;
using namespace std;

class MainViewController : public ZViewController {

public:
    explicit MainViewController(char* argv[]);
 	void onCreate() override;
    virtual ~MainViewController();
private:

    int mElapsedTime = 0;
    string getCurrentTime();

    void updateTime();
    void updateWeather();

    ZLabel* mTimeLabel;
    std::thread mUpdaterThread;

    std::atomic<bool> m_running;

    string getCurrentDate();

    ZLabel *mDateLabel;

    double getCPUTemperature();

    ZLabel *mGPUTemp;

    ZLabel *mOutdoorTemp;


    vector<double> getGPUTemperatures();

    double getOutdoorTemp();

    vector<int> getVRAMUsage();

    ZLabel *mGPUUsage;

    string formatBytesToGB(int bytes);

    float getUsedDiskSpace(const char *path);

    float getTotalDiskSpace(const char *path);

    ZLabel *mDiskSpaceLabel;

    void updateDiskSpace();

    ZLineChart* mVramChart;
    ZLabel *mIPLabel;

    string getPIIpds();
};

