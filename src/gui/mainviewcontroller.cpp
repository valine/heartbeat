#include "zmainviewcontroller.h"
#include "utils/zsettingsstore.h"
#include <chrono>
#include <ctime>
#include <thread>

MainViewController::MainViewController(char **argv) : ZViewController(argv), m_running(true) {}

MainViewController::~MainViewController() {

    m_running = false;
    if (mUpdaterThread.joinable()) {
        mUpdaterThread.join();
    }
}

void MainViewController::onCreate() {
    ZViewController::onCreate();
    auto* root = new ZView(ZView::fillParent, ZView::fillParent, getRootView());
    root->setBackgroundColor(ZSettings::getInstance().getBackgroundColor());
    root->setMargin(vec4(20));

    mTimeLabel = new ZLabel("", root);
    mTimeLabel->setText(getCurrentTime());
    mTimeLabel->setTextSize(30);
    mUpdaterThread = std::thread(&MainViewController::updateTime, this);

    m_dateLabel = new ZLabel(getCurrentDate(), root);
    m_dateLabel->setTextSize(30);
    m_dateLabel->setXOffset(120);
}

std::string MainViewController::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    struct tm *timeinfo;
    char buffer[80];

    timeinfo = localtime(&currentTime);
    strftime(buffer, 80, "%I:%M:%S %p", timeinfo);
    return string(buffer);
}

std::string MainViewController::getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentDate = std::chrono::system_clock::to_time_t(now);

    struct tm *dateinfo;
    char buffer[80];

    dateinfo = localtime(&currentDate);
    strftime(buffer, 80, "%A %B %d, %Y", dateinfo);

    return std::string(buffer);
}

void MainViewController::updateTime() {
    while (m_running) {
        mTimeLabel->setText(getCurrentTime());
        std::this_thread::sleep_for(std::chrono::seconds(1));

        m_dateLabel->setText(getCurrentDate());
        mTimeLabel->invalidate();
        getRootView()->invalidate();
    }
}
