#include "zmainviewcontroller.h"
#include "utils/zsettingsstore.h"
#include "ui/zlinechart.h"
#include <chrono>
#include <ctime>
#include <thread>
#include <iomanip>
#include <curl/curl.h>
#include <json/json.hpp>
#include <sys/statvfs.h>
#include <csignal>

MainViewController::MainViewController(char **argv) : ZViewController(argv), m_running(true) {}

MainViewController::~MainViewController() {

    m_running = false;
    if (mUpdaterThread.joinable()) {
        mUpdaterThread.join();
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

double MainViewController::getOutdoorTemp() {
    const std::string API_KEY = "16ab18f280bb9f7b99c11ad6f28e6cdc";
    const std::string CITY_ID = "5037649";

    cout << "Hitting weather API, minimize this" << endl;
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        std::string url = "http://api.openweathermap.org/data/2.5/weather?id=" + CITY_ID + "&appid=" + API_KEY;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            nlohmann::json jsonData = nlohmann::json::parse(readBuffer);

            double tempK = jsonData["main"]["temp"];
            double tempC = tempK - 273.15;
            double tempF = 9.0 / 5.0 * (tempK - 273.15) + 32.0;
            return tempF;
            //} else {
              //  std::cout << "Failed to parse JSON" << std::endl;
            //}
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0.0;
}

void MainViewController::onCreate() {
    ZViewController::onCreate();
    auto* root = new ZView(ZView::fillParent, ZView::fillParent, getRootView());
    root->setBackgroundColor(ZSettings::getInstance().getBackgroundColor());
    root->setMargin(vec4(50));

    auto* ll = new ZLinearLayout(ZView::fillParent, ZView::fillParent, root);
    auto* timeWidget = new ZView(ZView::fillParent, 120, ll);

    mTimeLabel = new ZLabel("", timeWidget);
    mTimeLabel->setText(getCurrentTime());
    mTimeLabel->setTextSize(120);

    mUpdaterThread = std::thread(&MainViewController::updateTime, this);

    mDateLabel = new ZLabel(getCurrentDate(), timeWidget);
    mDateLabel->setTextSize(30);
    mDateLabel->setMaxHeight(70);
    mDateLabel->setYOffset(30);
    mDateLabel->setXOffset(mTimeLabel->getEndPoint().first);
    mDateLabel->setTextColor(gold);

    mOutdoorTemp = new ZLabel("t70°", ll);
    mOutdoorTemp->setTextSize(60);
    mOutdoorTemp->setMaxWidth(200);
    mOutdoorTemp->setTextColor(vec4(0.180972, 0.587873, 1.000000, 1.000000));
    mOutdoorTemp->setOnClick([this](ZView* sender) {
        double temp = getOutdoorTemp();
        mOutdoorTemp->setText(to_string((int) temp));
    });

    mGPUUsage = new ZLabel("VRAM", ll);
    mGPUUsage->setTextSize(60);
    mGPUUsage->setMaxWidth(300);
    mGPUUsage->setMaxHeight(170);
    mGPUUsage->setTextColor(vec4(0.800657, 0.148515, 0.040476, 1.000000));

    mDiskSpaceLabel = new ZLabel("", ll);
    mDiskSpaceLabel->setTextSize(30);
    mDiskSpaceLabel->setMaxHeight(70);
    mDiskSpaceLabel->setTextColor(gold);
    mDiskSpaceLabel->setYOffset(420);

    mIPLabel = new ZLabel("Pi IPs Here", ll);
    mIPLabel->setTextSize(30);
    mIPLabel->setMaxHeight(70);
    mIPLabel->setTextColor(white);
    mIPLabel->setYOffset(420);
    mIPLabel->setOnClick([this](ZView* sender) {
        string ip = getPIIpds();
        mIPLabel->setText(ip);
    });

    mVramChart = new ZLineChart(ZView::fillParent, 300, ll);
    mVramChart->setMargin(vec4(0, 0, 0, 50));
    mVramChart->setBounds(vec4(0,5,5,0));
    mVramChart->setLineCount(1);
    mVramChart->setResolution(1000);
    mVramChart->setChartListener([this](vector<int> x, int lineIndex) {
        cout << x.at(0) << endl;
        vector<float> point;
        point.push_back(sin((float) x.at(0) * 0.05f));

        return point;
    });
    mVramChart->setInvalidateListener([this]() {
        mVramChart->resetTmpTransform();
        mVramChart->invalidateData();

    });
    mVramChart->resetZoom();
    mVramChart->resetTmpTransform();
    mVramChart->invalidateData();

    ll->refreshMargins();
    updateDiskSpace();

//    auto subViews = ll->getSubViews();
//    for (auto view : subViews) {
//        view->setDraggable(true);
//    }
    ll->setConsumeClicks(false);
    ll->setDraggable(false);
}

void MainViewController::updateDiskSpace() {// Update disk space label
    float currentDiskSpace = getUsedDiskSpace("/");
    float totalDiskSpace = getTotalDiskSpace("/");

    // Calculate available disk space
    float availableDiskSpace = totalDiskSpace - currentDiskSpace;

    // Convert the disk space float to string and keep 2 decimal points
    stringstream streamCurrent, streamTotal, streamAvailable;

    streamCurrent << fixed << setprecision(2) << currentDiskSpace;
    string currentDiskSpaceStr = streamCurrent.str() + " GB";

    streamTotal << fixed << setprecision(2) << totalDiskSpace;
    string totalDiskSpaceStr = streamTotal.str() + " GB";

    streamAvailable << fixed << setprecision(2) << availableDiskSpace;
    string availableDiskSpaceStr = streamAvailable.str() + " GB";

    // Concatenate the used, total and available disk space strings
    string diskSpaceStr = "Available: " + availableDiskSpaceStr + " / Total: " + totalDiskSpaceStr;

    mDiskSpaceLabel->setText(diskSpaceStr);
}

std::vector<int> MainViewController::getVRAMUsage() {
    std::string cmd = "nvidia-smi --query-gpu=memory.used --format=csv,noheader,nounits";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return std::vector<int>{};
    }

    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);

    std::stringstream ss(result);
    std::vector<int> usages;
    std::string temp;
    while (std::getline(ss, temp, '\n')) {
        usages.push_back(std::stoi(temp));
    }

    return usages;
}

float MainViewController::getUsedDiskSpace(const char* path) {
    struct statvfs stat;

    if (statvfs(path, &stat) != 0) {
        // error happens, just quits here
        std::cerr << "Failed to get disk usage: " << strerror(errno) << std::endl;
        return -1.0;
    }

    // the total file system size is f_bsize * f_blocks
    uint64_t total = stat.f_blocks * stat.f_frsize;
    // the free size including reserved space is f_bsize * f_bfree
    uint64_t free = stat.f_bfree * stat.f_frsize;
    // used space is total space - free space
    uint64_t used = total - free;

    // convert it into decimal gigabytes (GB)
    return (float) used / (1000 * 1000 * 1000);
}

float MainViewController::getTotalDiskSpace(const char* path) {
    struct statvfs stat;

    if (statvfs(path, &stat) != 0) {
        // error happens, just quits here
        std::cerr << "Failed to get disk usage: " << strerror(errno) << std::endl;
        return -1.0;
    }

    // the total file system size is f_bsize * f_blocks
    uint64_t total = stat.f_blocks * stat.f_frsize;

    // convert it into decimal gigabytes (GB)
    return (float) total / (1000 * 1000 * 1000);
}

std::string MainViewController::getCurrentTime() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%I:%M %p");

    std::string timeStr = ss.str();

    // Remove leading zero if present
    if (timeStr[0] == '0') {
        timeStr.erase(0, 1);
    }

    return timeStr;
}

std::string MainViewController::getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentDate = std::chrono::system_clock::to_time_t(now);

    struct tm *dateinfo;
    char buffer[80];

    dateinfo = localtime(&currentDate);
    strftime(buffer, 80, "%A\n%B %d, %Y", dateinfo);

    return std::string(buffer);
}

std::vector<double> MainViewController::getGPUTemperatures() {
    std::string cmd = "nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return std::vector<double>{};
    }

    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);

    std::stringstream ss(result);
    std::vector<double> temperatures;
    std::string temp;
    while (std::getline(ss, temp, '\n')) {
        temperatures.push_back(std::stod(temp));
    }

    return temperatures;
}

std::string MainViewController::getPIIpds() {
    std::string result;
    char buffer[128];
    const char* cmd = "/home/lukas/bin/pilan";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }

    return result;
}

std::string MainViewController::formatBytesToGB(int megabytes) {
    double gigabytes = megabytes / 1024.0;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << gigabytes;
    return ss.str() + " GB";
}

void MainViewController::updateTime() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while (m_running) {
        mTimeLabel->setText(getCurrentTime());

        mDateLabel->setXOffset(mTimeLabel->getEndPoint().first + MARGIN);
       // getRootView()->onWindowChange(getWindowWidth(), getWindowHeight());
        mDateLabel->setText(getCurrentDate());

        mTimeLabel->invalidate();

        int min15 = 15 * 60;
        if (mElapsedTime % (min15) == min15 - 1) {
            double temp = getOutdoorTemp();
            mOutdoorTemp->setText(to_string((int) temp) + "°");

            string piIpds = getPIIpds();
            mIPLabel->setText(piIpds);
        }
        getRootView()->invalidate();
        mElapsedTime++;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Update gpu usage
        std::vector<int> usages = getVRAMUsage();

        std::string usageStr = "";
        for (int usage : usages) {
            usageStr += formatBytesToGB(usage) + "\n";
        }

        mGPUUsage->setText(usageStr);
        updateDiskSpace();
    }
}
