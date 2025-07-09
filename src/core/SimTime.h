//
// Created by zhou on 25-6-12.
//

#ifndef SIMTIME_H
#define SIMTIME_H

#include <tuple>
#include <cmath>
#include <iomanip>


namespace core {
class SimTime {
public:
    double currentTime = 0; //当前时间sec
    double startTime   = 0; //开始时间sec
    double endTime     = 0; //结束时间sec

    double timeDelta   = 1;   //时间间隔 sec

    int startYear = 2025;
    int startMonth = 1;
    int startDay   = 1;

    int endYear = 2025;
    int endMonth = 1;
    int endDay   = 1;
public:
    SimTime(double startTime, double endTime, double timeDelta) {
        this->startTime = startTime;
        this->endTime = endTime;
        this->timeDelta = timeDelta;
    }
    ~SimTime()=default;

    void calcEndTime() {
        double endTimeDays = daysBetweenDates(startYear, startMonth, startDay, endYear, endMonth, endDay);
        if (endTimeDays>0) {
            //从0小时开始计算，最后一天是23小时结束，因此少1和小时
            endTime = (endTimeDays*24.0-1.0)*3600.0;
        }
    }

    /**
     * 判断指定年份是否为闰年
     * @param year 待判断的年份
     * @return 如果是闰年返回true，否则返回false
     */
    static bool isLeapYear(int year) {
        return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    }
    /**
     * 获取指定年份和月份的天数
     * @param year 年份
     * @param month 月份 (1-12)
     * @return 该月的天数，如果月份无效则返回0
     */
    static int daysInMonth(int year, int month) {
        if (month == 1 || month == 3 || month == 5 || month == 7 ||
        month == 8 || month == 10 || month == 12) {
            return 31;
        }
        if (month == 4 || month == 6 || month == 9 || month == 11) {
            return 30;
        }
        if (month == 2) {
            return isLeapYear(year) ? 29 : 28;
        }
        return 0;
    }

    /**
     * 计算指定日期是该年的第几天
     * @param year 年份
     * @param month 月份 (1-12)
     * @param day 日期
     * @return 该年的第几天，如果日期无效则返回0
     */
    static int dayOfYear(int year, int month, int day) {
        int totalDays = 0;
        for (int m = 1; m < month; m++) {
            totalDays += daysInMonth(year, m);
        }
        totalDays += day;
        return totalDays;
    }

    static std::tuple<int, int, int, int, int, int> getCurrentDateTime(
    int startYear, int startMonth, int startDay, double timeDiffSeconds) {

        // 计算总天数和剩余秒数
        double totalDays = std::floor(timeDiffSeconds / (24 * 60 * 60));
        double remainingSeconds = std::fmod(timeDiffSeconds, 24 * 60 * 60);

        int currentYear = startYear;
        int currentMonth = startMonth;
        int currentDay = startDay;
        int hours, minutes, seconds;

        // 处理天数增量
        while (totalDays > 0) {
            int daysInCurrentMonth = daysInMonth(currentYear, currentMonth);

            // 如果当前月份剩余天数足够，直接增加日期
            if (totalDays <= daysInCurrentMonth - currentDay) {
                currentDay += static_cast<int>(totalDays);
                totalDays = 0;
            } else {
                // 否则，推进到下一个月
                totalDays -= (daysInCurrentMonth - currentDay + 1);
                currentDay = 1;  // 新月份从1日开始
                currentMonth++;

                // 处理跨年
                if (currentMonth > 12) {
                    currentMonth = 1;
                    currentYear++;
                }
            }
        }

        // 计算时分秒
        hours = static_cast<int>(remainingSeconds / (60 * 60));
        remainingSeconds = std::fmod(remainingSeconds, 60 * 60);
        minutes = static_cast<int>(remainingSeconds / 60);
        seconds = static_cast<int>(std::fmod(remainingSeconds, 60));

        return std::make_tuple(currentYear, currentMonth, currentDay, hours, minutes, seconds);
    }

    void advanceTime() {
        this->currentTime += this->timeDelta;

        //std::string now = std::to_string(currentTime);
        //LOG_DEBUG(now);
    }

    [[nodiscard]] std::tuple<int, int, int, int, int, int> getCurrentDateTime() const {
        return getCurrentDateTime(this->startYear, this->startMonth, this->startDay, currentTime);
    }

    [[nodiscard]] std::string get_current_datetime_str() const {
        // 计算当前的年、月、日、时、分、秒
        // 这里需要根据实际的时间计算逻辑实现，以下是简化示例
        auto dt = SimTime::getCurrentDateTime(startYear, startMonth, startDay, currentTime);
        int current_year = std::get<0>(dt);
        int current_month = std::get<1>(dt);
        int current_day = std::get<2>(dt);

        // 从 current_time（秒）计算时、分、秒
        int hours = std::get<3>(dt);
        int minutes = std::get<4>(dt);
        int seconds = std::get<5>(dt);


        // 实际应用中，需要更复杂的逻辑来计算年、月、日
        // 这通常涉及到处理闰年、每月天数等问题

        // 格式化为字符串
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(4) << current_year << "-"
            << std::setw(2) << current_month << "-"
            << std::setw(2) << current_day << " "
            << std::setw(2) << hours << ":"
            << std::setw(2) << minutes << ":"
            << std::setw(2) << seconds;

        return oss.str();
    }

    /**
     * 计算两个日期之间的天数间隔
     * @param startYear 开始年份
     * @param startMonth 开始月份 (1-12)
     * @param startDay 开始日期
     * @param endYear 结束年份
     * @param endMonth 结束月份 (1-12)
     * @param endDay 结束日期
     * @return 两个日期之间的天数间隔，如果开始日期晚于结束日期返回负数
     */
    static int daysBetweenDates(int startYear, int startMonth, int startDay,
                                int endYear, int endMonth, int endDay) {
        // 计算从公元元年1月1日到开始日期的总天数
        auto daysToDate = [](int year, int month, int day) {
            int totalDays = 0;

            // 计算之前所有整年的天数
            for (int y = 1; y < year; y++) {
                totalDays += isLeapYear(y) ? 366 : 365;
            }

            // 计算当前年中到指定月份的天数
            for (int m = 1; m < month; m++) {
                totalDays += daysInMonth(year, m);
            }

            // 加上当月的天数
            totalDays += day;

            return totalDays;
        };

        // 计算两个日期到公元元年的天数差
        int daysStart = daysToDate(startYear, startMonth, startDay);
        int daysEnd = daysToDate(endYear, endMonth, endDay);

        return daysEnd - daysStart+1;
    }


};


}






#endif //SIMTIME_H
