#!/bin/bash

#******************************************************************************
#
# @file create_project.sh
# @brief 创建UnravelChat项目完整结构和基础代码
#
# @author unravel
# @date 2025/07/27
#******************************************************************************

set -e  # 遇到错误立即退出

# 颜色定义
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

log() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

info() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查当前目录并创建UnravelChat目录
setup_project_directory() {
    if [[ ! -d "UnravelChat" ]]; then
        log "创建UnravelChat项目目录..."
        mkdir UnravelChat
    fi
    
    cd UnravelChat
    log "进入项目目录: $(pwd)"
}

# 创建项目目录结构
create_directories() {
    info "创建项目目录结构..."
    
    # 创建主要目录结构
    mkdir -p client/{src/{ui,network,models,widgets,utils},resources/{images,icons,style,translations},config,build,bin}
    mkdir -p server/{src,config,build,bin}
    mkdir -p email-service/{src/{routes,models,utils},config,logs}
    mkdir -p database/{mysql,redis}
    mkdir -p deploy/{scripts,docker,nginx}
    mkdir -p docs
    mkdir -p tests
    mkdir -p tools
    
    log "项目目录结构创建完成！"
}

# 创建全局工具类
create_global_utils() {
    info "创建全局工具类..."
    
    # 创建Global.h
cat > client/src/utils/Global.h << 'GLOBAL_H_END'
/******************************************************************************
*
* @file Global.h
* @brief 全局定义和常量
*
* @author unravel
* @date 2025/07/27
*******************************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QDateTime>

// 应用程序信息
#define APP_NAME "UnravelChat"
#define APP_VERSION "1.0.0"
#define APP_ORGANIZATION "Unravel Studio"

// 服务器配置
#define DEFAULT_SERVER_HOST "127.0.0.1"
#define DEFAULT_SERVER_PORT 8888
#define DEFAULT_HTTP_PORT 3000

// 网络超时配置
#define CONNECTION_TIMEOUT 10000    // 10秒
#define HEARTBEAT_INTERVAL 30000    // 30秒
#define RECONNECT_INTERVAL 5000     // 5秒

// 全局工具函数
class GlobalUtils {
public:
    // 密码加密
    static QString encryptPassword(const QString& password, const QString& salt = "UnravelChat2025");
    
    // 生成UUID
    static QString generateUUID();
    
    // 日志输出
    static void debugLog(const QString& message);
};

#endif // GLOBAL_H
GLOBAL_H_END

    # 创建Global.cpp
cat > client/src/utils/Global.cpp << 'GLOBAL_CPP_END'
/******************************************************************************
*
* @file Global.cpp
* @brief 全局定义和常量实现
*
* @author unravel
* @date 2025/07/27
*******************************************************************************/

#include "Global.h"
#include <QUuid>
#include <QDateTime>

QString GlobalUtils::encryptPassword(const QString& password, const QString& salt) {
    QString saltedPassword = password + salt;
    QByteArray hash = QCryptographicHash::hash(saltedPassword.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

QString GlobalUtils::generateUUID() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void GlobalUtils::debugLog(const QString& message) {
    qDebug() << "[" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "]" << message;
}
GLOBAL_CPP_END

    log "全局工具类创建完成"
}

# 创建简化的主程序
create_simple_main() {
    info "创建简化的主程序..."
    
cat > client/src/main.cpp << 'MAIN_CPP_END'
/******************************************************************************
*
* @file main.cpp
* @brief 应用程序主入口
*
* @author unravel
* @date 2025/07/27
*******************************************************************************/

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

class SimpleLoginWidget : public QWidget {
    Q_OBJECT

public:
    SimpleLoginWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
    }

private slots:
    void onLoginClicked() {
        QString username = usernameEdit->text();
        QString password = passwordEdit->text();
        
        if (username == "admin" && password == "123456") {
            QMessageBox::information(this, "登录成功", 
                QString("欢迎使用UnravelChat!\n用户: %1").arg(username));
            titleLabel->setText("UnravelChat - 已登录: " + username);
        } else {
            QMessageBox::warning(this, "登录失败", "用户名或密码错误!");
        }
    }

private:
    void setupUI() {
        setWindowTitle("UnravelChat - 登录");
        setFixedSize(350, 250);
        
        auto *layout = new QVBoxLayout(this);
        
        titleLabel = new QLabel("UnravelChat");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2196F3; margin: 20px;");
        
        auto *subtitleLabel = new QLabel("连接世界，分享精彩");
        subtitleLabel->setAlignment(Qt::AlignCenter);
        subtitleLabel->setStyleSheet("color: #666; margin-bottom: 20px;");
        
        usernameEdit = new QLineEdit();
        usernameEdit->setPlaceholderText("请输入用户名");
        usernameEdit->setStyleSheet("padding: 8px; border: 2px solid #ddd; border-radius: 5px;");
        
        passwordEdit = new QLineEdit();
        passwordEdit->setPlaceholderText("请输入密码");
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setStyleSheet("padding: 8px; border: 2px solid #ddd; border-radius: 5px;");
        
        loginButton = new QPushButton("登录");
        loginButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #2196F3;"
            "  color: white;"
            "  border: none;"
            "  padding: 10px;"
            "  border-radius: 5px;"
            "  font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "  background-color: #1976D2;"
            "}"
        );
        
        auto *tipLabel = new QLabel("提示: 用户名 admin, 密码 123456");
        tipLabel->setAlignment(Qt::AlignCenter);
        tipLabel->setStyleSheet("color: #999; font-size: 12px; margin-top: 10px;");
        
        layout->addWidget(titleLabel);
        layout->addWidget(subtitleLabel);
        layout->addWidget(usernameEdit);
        layout->addWidget(passwordEdit);
        layout->addWidget(loginButton);
        layout->addWidget(tipLabel);
        layout->addStretch();
    }
    
    void connectSignals() {
        connect(loginButton, &QPushButton::clicked, this, &SimpleLoginWidget::onLoginClicked);
        connect(passwordEdit, &QLineEdit::returnPressed, this, &SimpleLoginWidget::onLoginClicked);
    }

private:
    QLabel *titleLabel;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "UnravelChat 启动中...";
    
    SimpleLoginWidget window;
    window.show();
    
    qDebug() << "UnravelChat 界面显示完成";
    
    return app.exec();
}

#include "main.moc"
MAIN_CPP_END

    log "简化主程序创建完成"
}

# 创建简化的项目文件
create_simple_project_files() {
    info "创建项目文件..."
    
cat > client/CMakeLists.txt << 'CMAKE_END'
cmake_minimum_required(VERSION 3.16)
project(UnravelChat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 尝试查找Qt6，如果找不到则查找Qt5
find_package(Qt6 QUIET COMPONENTS Core Widgets)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

set(CMAKE_AUTOMOC ON)

# 源文件
set(SOURCES
    src/main.cpp
    src/utils/Global.cpp
)

set(HEADERS
    src/utils/Global.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接Qt库
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)
else()
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets)
endif()

# 编译器定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
    APP_VERSION="${PROJECT_VERSION}"
    APP_NAME="UnravelChat"
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

message(STATUS "使用Qt${QT_VERSION_MAJOR}构建UnravelChat")
CMAKE_END

cat > client/UnravelChat.pro << 'PRO_END'
QT += core widgets

TARGET = UnravelChat
TEMPLATE = app
VERSION = 1.0.0

CONFIG += c++11
CONFIG += warn_on

# 输出目录
DESTDIR = $$PWD/bin

# 头文件路径
INCLUDEPATH += src src/utils

# 源文件
SOURCES += \
    src/main.cpp \
    src/utils/Global.cpp

HEADERS += \
    src/utils/Global.h

# 编译器定义
DEFINES += \
    QT_DEPRECATED_WARNINGS \
    APP_VERSION=\\\"$$VERSION\\\" \
    APP_NAME=\\\"UnravelChat\\\"

message("构建UnravelChat v$$VERSION")
PRO_END

    log "项目文件创建完成"
}

# 创建构建脚本
create_build_script() {
    info "创建构建脚本..."
    
cat > client/build.sh << 'BUILD_END'
#!/bin/bash

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

log() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

info() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    info "检查构建依赖..."
    
    if ! command -v cmake &> /dev/null; then
        error "CMake未安装，请运行: sudo apt install cmake"
        exit 1
    fi
    
    if ! command -v g++ &> /dev/null; then
        error "g++编译器未安装，请运行: sudo apt install build-essential"
        exit 1
    fi
    
    # 检查Qt
    if ! pkg-config --exists Qt6Core 2>/dev/null && ! pkg-config --exists Qt5Core 2>/dev/null; then
        error "Qt未安装，请运行: sudo apt install qt6-base-dev 或 sudo apt install qt5-default"
        exit 1
    fi
    
    log "依赖检查通过"
}

build_with_cmake() {
    info "使用CMake构建项目..."
    
    mkdir -p build
    cd build
    
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    
    cd ..
    log "CMake构建完成"
}

build_with_qmake() {
    info "使用qmake构建项目..."
    
    if command -v qmake6 &> /dev/null; then
        qmake6 UnravelChat.pro CONFIG+=release
    elif command -v qmake &> /dev/null; then
        qmake UnravelChat.pro CONFIG+=release
    else
        error "找不到qmake命令"
        exit 1
    fi
    
    make -j$(nproc)
    log "qmake构建完成"
}

run_program() {
    info "运行程序..."
    
    if [ -f "build/UnravelChat" ]; then
        ./build/UnravelChat
    elif [ -f "bin/UnravelChat" ]; then
        ./bin/UnravelChat
    elif [ -f "UnravelChat" ]; then
        ./UnravelChat
    else
        error "找不到可执行文件"
        exit 1
    fi
}

clean() {
    info "清理构建文件..."
    rm -rf build/ bin/ Makefile *.o moc_* ui_* qrc_*
    log "清理完成"
}

show_help() {
    echo "UnravelChat 构建脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  cmake     使用CMake构建"
    echo "  qmake     使用qmake构建"  
    echo "  run       构建并运行程序"
    echo "  clean     清理构建文件"
    echo "  help      显示帮助信息"
    echo ""
    echo "默认登录信息:"
    echo "  用户名: admin"
    echo "  密码: 123456"
}

case "$1" in
    "cmake")
        check_dependencies
        build_with_cmake
        ;;
    "qmake")
        check_dependencies
        build_with_qmake
        ;;
    "run")
        check_dependencies
        build_with_cmake
        run_program
        ;;
    "clean")
        clean
        ;;
    "help"|"-h"|"--help")
        show_help
        ;;
    "")
        check_dependencies
        build_with_cmake
        ;;
    *)
        error "未知选项: $1"
        show_help
        exit 1
        ;;
esac
BUILD_END

    chmod +x client/build.sh
    log "构建脚本创建完成"
}

# 创建README
create_readme() {
    info "创建README文件..."
    
cat > README.md << 'README_END'
# UnravelChat - C++/Qt聊天系统

一个基于C++11和Qt的聊天系统项目，用于学习网络编程和Qt开发。

## 快速开始

### 环境要求

```bash
# Ubuntu 22.04
sudo apt update
sudo apt install -y qt6-base-dev cmake build-essential
# 或者使用Qt5
# sudo apt install -y qt5-default qttools5-dev-tools

README_END
}
