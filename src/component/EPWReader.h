//
// Created by zhou on 25-6-13.
//

#ifndef FILEREADER_H
#define FILEREADER_H
#include <BaseComponent.h>
#include <fstream>
#include <random>

namespace comp {
class EPWReader:public core::BaseComponent {
    private:

    bool flag=false;


    std::string filename;                 // 文件名
    std::string delimiter;                // 字段分隔符
    std::vector<unsigned int> targetCols;          // 目标列索引
    std::vector<std::vector<float>> data; // 存储数据的二维向量

    // 解析一行数据
    [[nodiscard]] std::vector<float> parseLine(const std::string& line) const {
        std::vector<float> values;
        std::stringstream ss(line);
        std::string part;

        // 分割行数据
        std::vector<std::string> parts;
        while (std::getline(ss, part, delimiter.empty() ? ',' : delimiter[0])) {
            parts.push_back(part);
        }

        // 提取目标列数据
        for (auto colIdx : targetCols) {
            if (colIdx >= parts.size()) {
                values.push_back(std::numeric_limits<float>::quiet_NaN());  // 列索引超出范围时填充NaN
            } else {
                std::string valStr = parts[colIdx];
                valStr.erase(std::ranges::remove_if(valStr, ::isspace).begin(), valStr.end());

                if (valStr.empty()) {
                    values.push_back(std::numeric_limits<float>::quiet_NaN());
                } else {
                    try {
                        float value = std::stof(valStr);
                        values.push_back(value);
                    } catch (const std::exception&) {
                        values.push_back(std::numeric_limits<float>::quiet_NaN());  // 转换失败则填充NaN
                    }
                }
            }
        }
        return values;
    }

    // 读取文件
    void readFile() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开文件: " + filename);
        }

        std::vector<std::string> validLines;
        std::string line;

        // 读取所有行
        while (std::getline(file, line)) {
            std::erase_if(line, ::isspace);
            if (line.empty() || line.substr(0, 1) == "#") {
                continue;  // 跳过空行和注释行
            }

            // 检查是否为有效数据行（第一列是4位数字年份）
            size_t delimiterPos = line.find(delimiter);
            if (delimiterPos != std::string::npos) {
                std::string firstPart = line.substr(0, delimiterPos);
                if (firstPart.size() == 4 && std::all_of(firstPart.begin(), firstPart.end(), ::isdigit)) {
                    validLines.push_back(line);
                }
            }
        }

        if (validLines.empty()) {
            throw std::runtime_error("文件中没有有效数据行");
        }

        // 逐行解析数据
        data.clear();
        for (const std::string& line_2 : validLines) {
            std::vector<float> values = parseLine(line_2);
            data.push_back(values);
        }
        //最后一个元素移动到前面

        if (data.size() >= 2) {  // 确保有至少两个元素
            // 将最后一个元素旋转到第一个位置
            std::ranges::rotate(data, data.end() - 1);
        }

    }

    // 获取指定小时的数据，支持线性插值
    std::vector<float> getDataAtHour(double hour) {
        const int TOTAL_HOURS = 8760;
        hour = fmod(hour, TOTAL_HOURS);

        if (hour < 0) {
            throw std::invalid_argument("指定的小时数不能为负数");
        }

        int hourInt = static_cast<int>(hour);
        if (hourInt == hour) {
            // 整数小时，直接获取
            if (hourInt >= 0 && hourInt < static_cast<int>(data.size())) {
                return data[hourInt];
            } else {
                throw std::out_of_range("索引超出数据范围");
            }
        }
        // 非整数小时，线性插值
        int lowerHour = static_cast<int>(hour);
        int upperHour = (lowerHour + 1) % TOTAL_HOURS;
        double fraction = hour - lowerHour;

        if (lowerHour >= 0 && lowerHour < static_cast<int>(data.size()) &&
            upperHour >= 0 && upperHour < static_cast<int>(data.size())) {
            const std::vector<float>& lowerData = data[lowerHour];
            const std::vector<float>& upperData = data[upperHour];
            std::vector<float> interpolatedData(lowerData.size());

            for (size_t i = 0; i < lowerData.size(); ++i) {
                interpolatedData[i] = lowerData[i] + fraction * (upperData[i] - lowerData[i]);
            }
            return interpolatedData;
        }
        throw std::out_of_range("插值索引超出数据范围");
    }

public:

    EPWReader()  {
        filename = "";
        delimiter = ",";
        targetCols = {6, 21, 13, 14, 15};  // [温度，风速，辐射1，辐射2，辐射3]
    }
    void awake() override;
    void before(const core::SimTime& time) override;
    void after(const core::SimTime& time) override;
    void sayHello() override;
    void parse(const json &in_params) override;


};


}


#endif //FILEREADER_H
