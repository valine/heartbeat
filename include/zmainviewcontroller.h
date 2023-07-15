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

using namespace glm;
using namespace std;

class MainViewController : public ZViewController {

public:
    explicit MainViewController(char* argv[]);
 	void onCreate() override;
    virtual ~MainViewController();
private:

    string getCurrentTime();

    void updateTime();

    ZLabel* mTimeLabel;
    std::thread mUpdaterThread;
    std::atomic<bool> m_running;

    string getCurrentDate();

    ZLabel *m_dateLabel;
};

