#!/bin/bash

# run_simple_test.sh - 基于您成功经验的简单测试脚本
# 使用方法: ./run_simple_test.sh

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_message() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    echo ""
    print_message $BLUE "╔══════════════════════════════════════════════════════════════╗"
    print_message $BLUE "║            UnravelChat AuthenticationUI 简化测试             ║"
    print_message $BLUE "║              基于成功的test_authmanager模式                   ║"
    print_message $BLUE "╚══════════════════════════════════════════════════════════════╝"
    echo ""
}

# 检查基本环境
check_environment() {
    print_message $BLUE "🔍 检查环境..."
    
    # 检查是否在正确目录
    if [ ! -f "test_authmanager.pro" ]; then
        print_message $RED "❌ 请在tests目录下运行此脚本"
        print_message $YELLOW "当前目录: $(pwd)"
        print_message $YELLOW "应该在: ~/Projects/UnravelChat/UnravelChat/tests"
        exit 1
    fi
    
    # 检查qmake（使用与您成功测试相同的方式）
    if ! command -v qmake >/dev/null 2>&1; then
        print_message $RED "❌ qmake未找到"
        exit 1
    fi
    
    # 获取Qt版本（与您的环境一致）
    local qt_version=$(qmake -query QT_VERSION 2>/dev/null)
    print_message $GREEN "✅ Qt版本: $qt_version"
    
    # 检查之前的成功测试
    if [ -f "../bin/test_authmanager" ]; then
        print_message $GREEN "✅ 发现之前成功的测试: test_authmanager"
    fi
    
    print_message $GREEN "✅ 环境检查通过"
}

# 编译测试（使用与您成功测试相同的模式）
build_test() {
    print_message $BLUE "🏗️ 编译AuthenticationUI测试..."
    
    # 使用与test_authmanager.pro相同的模式
    print_message $YELLOW "📝 使用成功的编译模式..."
    
    # 编译简化版测试
    qmake test_authUI.pro
    if [ $? -ne 0 ]; then
        print_message $RED "❌ qmake失败"
        return 1
    fi
    
    make
    if [ $? -ne 0 ]; then
        print_message $RED "❌ 编译失败"
        print_message $YELLOW "💡 可能原因："
        print_message $YELLOW "  1. AuthenticationUI.cpp文件不完整"
        print_message $YELLOW "  2. 缺少必要的源文件"
        print_message $YELLOW "  3. 头文件路径问题"
        return 1
    fi
    
    # 检查输出文件
    if [ ! -f "../bin/test_authUI" ]; then
        print_message $RED "❌ 测试可执行文件未生成"
        return 1
    fi
    
    print_message $GREEN "✅ 编译成功: ../bin/test_authUI"
    return 0
}

# 运行测试
run_test() {
    print_message $BLUE "🧪 运行测试..."
    
    cd ../bin || exit 1
    
    # 设置环境变量（避免Qt警告）
    export QT_LOGGING_RULES="qt.qpa.xcb.xcb_error.debug=false"
    
    # 运行测试
    ./test_authUI
    local result=$?
    
    cd - > /dev/null
    
    return $result
}

# 清理函数
clean_build() {
    print_message $YELLOW "🧹 清理构建文件..."
    
    rm -f Makefile
    rm -f *.o
    rm -rf ../build/tests/*
    rm -f ../bin/test_authUI
    
    print_message $GREEN "✅ 清理完成"
}

# 显示帮助
show_help() {
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  --clean    清理构建文件"
    echo "  --help     显示此帮助"
    echo ""
    echo "示例:"
    echo "  $0         # 运行测试"
    echo "  $0 --clean # 清理构建文件"
}

# 主函数
main() {
    # 处理参数
    case "${1:-}" in
        --clean)
            print_header
            clean_build
            exit 0
            ;;
        --help)
            show_help
            exit 0
            ;;
        "")
            # 继续执行测试
            ;;
        *)
            print_message $RED "错误: 未知选项 '$1'"
            show_help
            exit 1
            ;;
    esac
    
    print_header
    
    # 检查环境
    check_environment
    
    # 编译测试
    build_test
    if [ $? -ne 0 ]; then
        print_message $RED "💥 编译失败"
        print_message $YELLOW ""
        print_message $YELLOW "🔧 故障排除："
        print_message $YELLOW "  1. 检查AuthenticationUI源文件是否存在："
        print_message $YELLOW "     ls -la ../src/auth/AuthenticationUI.*"
        print_message $YELLOW "     ls -la ../include/auth/AuthenticationUI.*"
        print_message $YELLOW ""
        print_message $YELLOW "  2. 如果文件不存在，请先创建基础文件"
        print_message $YELLOW "  3. 参考成功的test_authmanager结构"
        exit 1
    fi
    
    # 运行测试
    run_test
    local test_result=$?
    
    echo ""
    if [ $test_result -eq 0 ]; then
        print_message $GREEN "🎉 AuthenticationUI基础测试成功！"
        print_message $BLUE "📋 测试报告："
        print_message $BLUE "  - Qt环境正常"
        print_message $BLUE "  - 基础Widget功能正常"
        print_message $BLUE "  - 编译系统工作正常"
        print_message $YELLOW ""
        print_message $YELLOW "🚀 下一步建议："
        print_message $YELLOW "  1. 完善AuthenticationUI类实现"
        print_message $YELLOW "  2. 添加更多UI功能测试"
        print_message $YELLOW "  3. 集成到主项目中"
    else
        print_message $RED "💥 测试失败"
        print_message $YELLOW "请检查上述错误信息"
    fi
    
    exit $test_result
}

# 运行主函数
main "$@"