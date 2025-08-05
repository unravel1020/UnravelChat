# test_authenticationui_complete.pro
# AuthenticationUI模块完整测试项目配置文件
# Author: unravel
# Date: 2025-01-29
# Framework: Qt6 Test Framework with C++11
# Task: 配置AuthenticationUI用户界面完整测试的编译环境

QT += core widgets testlib network sql

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++11

# 启用测试相关的特性
CONFIG += testcase
CONFIG += debug_and_release

# 模板类型
TEMPLATE = app

# 目标名称
TARGET = test_authenticationui_complete

# 版本信息
VERSION = 1.0.0

# 测试源文件
SOURCES += \
    test_authenticationui_complete.cpp \
    ../../src/auth/AuthenticationUI.cpp \
    ../../src/auth/AuthenticationManager.cpp \
    ../../src/auth/SessionManager.cpp \
    ../../src/auth/PasswordManager.cpp \
    ../../src/auth/UserDatabase.cpp

# 头文件包含路径
INCLUDEPATH += \
    ../../include \
    ../../include/auth \
    ../../include/core \
    .

# 头文件
HEADERS += \
    ../../include/auth/AuthenticationUI.h \
    ../../include/auth/AuthenticationManager.h \
    ../../include/auth/SessionManager.h \
    ../../include/auth/PasswordManager.h \
    ../../include/auth/UserDatabase.h

# 预处理器定义
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += TESTING_MODE
DEFINES += QT_USE_QSTRINGBUILDER
DEFINES += UNRAVEL_CHAT_VERSION=\\\"1.0.0\\\"

# 编译器标志
QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -Wpedantic
QMAKE_CXXFLAGS += -fno-omit-frame-pointer

# 调试标志
debug {
    DEFINES += DEBUG_MODE
    QMAKE_CXXFLAGS += -g -O0
    TARGET = $$join(TARGET,,,_debug)
}

# 发布标志
release {
    DEFINES += RELEASE_MODE
    QMAKE_CXXFLAGS += -O2 -DNDEBUG
    TARGET = $$join(TARGET,,,_release)
}

# 链接库
LIBS += -lcrypto

# 平台特定配置
win32 {
    LIBS += -lws2_32 -luser32 -ladvapi32
    CONFIG += windows
    DEFINES += WIN32_LEAN_AND_MEAN
    
    # Windows资源文件
    RC_FILE = ../../resources/app.rc
    
    # Windows特定的库
    LIBS += -lshell32 -lole32 -luuid
}

unix:!macx {
    LIBS += -ldl -lpthread
    CONFIG += linux
    
    # Linux特定的库
    LIBS += -lX11
    
    # 包含系统路径
    INCLUDEPATH += /usr/include/qt6
}

macx {
    CONFIG += mac
    LIBS += -framework CoreFoundation -framework Security
    
    # macOS特定配置
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
    QMAKE_INFO_PLIST = ../../resources/Info.plist
}

# 资源文件
RESOURCES += \
    ../../resources/icons.qrc \
    ../../resources/animations.qrc \
    ../../resources/styles.qrc

# 翻译文件
TRANSLATIONS += \
    ../../translations/auth_zh_CN.ts \
    ../../translations/auth_en_US.ts

# 目标目录
DESTDIR = ../../bin/tests

# 临时文件目录
OBJECTS_DIR = ../../build/test_authenticationui_complete/obj
MOC_DIR = ../../build/test_authenticationui_complete/moc
RCC_DIR = ../../build/test_authenticationui_complete/rcc
UI_DIR = ../../build/test_authenticationui_complete/ui

# 确保构建目录存在
!exists($$OBJECTS_DIR) {
    system(mkdir -p $$OBJECTS_DIR)
}

!exists($$MOC_DIR) {
    system(mkdir -p $$MOC_DIR)
}

!exists($$RCC_DIR) {
    system(mkdir -p $$RCC_DIR)
}

!exists($$UI_DIR) {
    system(mkdir -p $$UI_DIR)
}

!exists($$DESTDIR) {
    system(mkdir -p $$DESTDIR)
}

# 测试配置
test {
    CONFIG += console testcase
    QT += testlib
    
    # 测试输出格式
    TEST_ARGS = -o test_results.xml,xml -o -,txt
    
    # 测试环境变量
    TEST_ENV = QT_LOGGING_RULES=qt.qpa.xcb.xcb_error.debug=false
}

# 代码覆盖率（仅在debug模式下）
debug {
    coverage {
        QMAKE_CXXFLAGS += --coverage
        QMAKE_LFLAGS += --coverage
        LIBS += -lgcov
        
        # 覆盖率报告目标
        coverage_report.target = coverage-report
        coverage_report.commands = \
            gcov *.gcno && \
            lcov --capture --directory . --output-file coverage.info && \
            genhtml coverage.info --output-directory coverage-html
        QMAKE_EXTRA_TARGETS += coverage_report
    }
}

# 内存检查（Valgrind）
valgrind {
    valgrind_check.target = valgrind-check
    valgrind_check.commands = \
        valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
        --track-origins=yes --verbose --log-file=valgrind.log \
        ./$$TARGET
    QMAKE_EXTRA_TARGETS += valgrind_check
}

# 静态分析
static_analysis {
    cppcheck.target = cppcheck
    cppcheck.commands = \
        cppcheck --enable=all --xml --xml-version=2 \
        --output-file=cppcheck.xml \
        ../../src/auth/ ../../include/auth/
    QMAKE_EXTRA_TARGETS += cppcheck
}

# 性能分析
profiling {
    QMAKE_CXXFLAGS += -pg
    QMAKE_LFLAGS += -pg
    
    profile.target = profile
    profile.commands = \
        ./$$TARGET && \
        gprof $$TARGET gmon.out > profile.txt
    QMAKE_EXTRA_TARGETS += profile
}

# 文档生成
docs {
    doxygen.target = docs
    doxygen.commands = doxygen ../../docs/Doxyfile
    QMAKE_EXTRA_TARGETS += doxygen
}

# 清理额外文件
QMAKE_CLEAN += \
    *.gcov *.gcda *.gcno \
    coverage.info valgrind.log \
    profile.txt gmon.out \
    test_results.xml \
    cppcheck.xml

# 安装规则
target.path = $$DESTDIR
INSTALLS += target

# 自定义目标：运行所有测试
run_tests.target = run-tests
run_tests.commands = \
    @echo "🚀 运行AuthenticationUI完整测试套件..." && \
    ./$$TARGET -v2 -maxwarnings 0
run_tests.depends = $$TARGET

QMAKE_EXTRA_TARGETS += run_tests

# 自定义目标：快速测试
quick_test.target = quick-test
quick_test.commands = \
    @echo "⚡ 运行快速测试..." && \
    ./$$TARGET -functions testUIInitialization,testPageNavigation,testLoginValidation
quick_test.depends = $$TARGET

QMAKE_EXTRA_TARGETS += quick_test

# 自定义目标：性能测试
perf_test.target = perf-test
perf_test.commands = \
    @echo "📊 运行性能测试..." && \
    ./$$TARGET -functions testMemoryUsage,testResponseTimes,testAnimationPerformance
perf_test.depends = $$TARGET

QMAKE_EXTRA_TARGETS += perf_test

# 自定义目标：集成测试
integration_test.target = integration-test
integration_test.commands = \
    @echo "🔗 运行集成测试..." && \
    ./$$TARGET -functions testEmailServiceIntegration,testNetworkErrorHandling
integration_test.depends = $$TARGET

QMAKE_EXTRA_TARGETS += integration_test

# 自定义目标：帮助信息
help.target = help
help.commands = \
    @echo "📋 AuthenticationUI测试项目可用目标:" && \
    @echo "  make                  - 编译测试程序" && \
    @echo "  make run-tests        - 运行完整测试套件" && \
    @echo "  make quick-test       - 运行快速测试" && \
    @echo "  make perf-test        - 运行性能测试" && \
    @echo "  make integration-test - 运行集成测试" && \
    @echo "  make coverage-report  - 生成代码覆盖率报告" && \
    @echo "  make valgrind-check   - 运行内存检查" && \
    @echo "  make cppcheck         - 运行静态分析" && \
    @echo "  make profile          - 运行性能分析" && \
    @echo "  make docs             - 生成文档" && \
    @echo "  make clean            - 清理构建文件"

QMAKE_EXTRA_TARGETS += help

# 默认目标
DEFAULT_TARGETS += $$TARGET

# 构建信息
message("🏗️  配置AuthenticationUI测试项目:")
message("   目标平台: $$QMAKESPEC")
message("   Qt版本: $$QT_VERSION")
message("   编译器: $$QMAKE_CXX")
message("   目标目录: $$DESTDIR")
message("   构建模式: $$CONFIG")
message("   链接库: $$LIBS")

# 警告检查
!exists(../../include/auth/AuthenticationUI.h) {
    warning("找不到AuthenticationUI.h头文件，请确保路径正确")
}

!exists(../../src/auth/AuthenticationUI.cpp) {
    warning("找不到AuthenticationUI.cpp源文件，请确保路径正确")
}

# 版本检查
!versionAtLeast(QT_VERSION, 6.0.0) {
    error("需要Qt 6.0.0或更高版本")
}

# 编译器检查
!contains(QMAKE_CXX, g++):!contains(QMAKE_CXX, clang++) {
    warning("推荐使用GCC或Clang编译器")
}

# 构建完成信息
build_info.target = build-info
build_info.commands = \
    @echo "✅ AuthenticationUI测试程序构建完成!" && \
    @echo "📍 可执行文件: $$DESTDIR/$$TARGET" && \
    @echo "🎯 运行测试: make run-tests" && \
    @echo "📚 获取帮助: make help"

QMAKE_EXTRA_TARGETS += build_info