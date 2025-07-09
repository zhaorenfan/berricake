#include <SimManager.h>
#include <ComponentRegistry.h>
#include <iostream>
#include <string>
#include <getopt.h>

// 程序参数结构体
struct ProgramArgs {
    std::string inputFile = "in.idf"; // 默认输入文件
    bool help = false;                // 是否显示帮助信息
    bool verbose = false;             // 是否启用详细输出
    std::string logLevel = "INFO";    // 日志级别
};

// 显示帮助信息
void showHelp(const std::string& programName) {
    std::cout << "使用方法: " << programName << " [选项] [输入文件]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -h, --help          显示此帮助信息" << std::endl;
    std::cout << "  -v, --verbose       启用详细输出" << std::endl;
    std::cout << "  -l, --loglevel      设置日志级别 (DEBUG, INFO, WARN, ERROR, FATAL)" << std::endl;
    std::cout << "  -i, --input         指定输入文件 (默认: in.idf)" << std::endl;
}

// 解析命令行参数
ProgramArgs parseArguments(int argc, char* argv[]) {
    ProgramArgs args;
    
    // 定义长选项
    static struct option longOptions[] = {
        {"help",    no_argument,       0, 'h'},
        {"verbose", no_argument,       0, 'v'},
        {"loglevel", required_argument, 0, 'l'},
        {"input",   required_argument, 0, 'i'},
        {nullptr, 0, nullptr, 0}
    };
    
    int opt;
    int optionIndex = 0;
    
    // 解析选项
    while ((opt = getopt_long(argc, argv, "hvl:i:", longOptions, &optionIndex)) != -1) {
        switch (opt) {
            case 'h':
                args.help = true;
                break;
            case 'v':
                args.verbose = true;
                break;
            case 'l':
                args.logLevel = optarg;
                break;
            case 'i':
                args.inputFile = optarg;
                break;
            case '?':
                // getopt_long 已经输出了错误信息
                break;
            default:
                abort();
        }
    }
    
    // 处理非选项参数（最后一个参数视为输入文件）
    if (optind < argc) {
        args.inputFile = argv[optind];
    }
    
    return args;
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    ProgramArgs args = parseArguments(argc, argv);
    
    // 显示帮助信息并退出
    if (args.help) {
        showHelp(argv[0]);
        return 0;
    }
    
    // 输出程序启动信息
    std::cout << "仿真程序启动..." << std::endl;
    if (args.verbose) {
        std::cout << "详细模式: 开启" << std::endl;
        std::cout << "日志级别: " << args.logLevel << std::endl;
        std::cout << "输入文件: " << args.inputFile << std::endl;
    }
    
    try {
        // 注册组件
        ComponentRegistry::registerAllComponents();
        
        // 初始化仿真管理器
        core::SimManager manager;
        
        // 设置日志级别（假设SimManager有相应接口）
        //manager.setLogLevel(args.logLevel);
        
        // 解析输入文件
        std::cout << "正在解析输入文件: " << args.inputFile << std::endl;
        manager.parse_file(args.inputFile);
        
        // 运行仿真
        std::cout << "开始运行仿真..." << std::endl;
        manager.run();
        
        std::cout << "仿真完成!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "仿真错误: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "未知错误发生!" << std::endl;
        return 1;
    }
}