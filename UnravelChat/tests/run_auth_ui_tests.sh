#!/bin/bash

# @file run_auth_ui_tests.sh
# @author unravel
# @date 2025-01-29
# @framework Bash Script for Qt Test Framework
# @technology QTest, Bash, CI/CD
# @task AuthenticationUI模块完整测试运行脚本

# 脚本配置
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
TEST_DIR="$PROJECT_ROOT/tests"
BUILD_DIR="$PROJECT_ROOT/build/test_authenticationui_complete"
BIN_DIR="$PROJECT_ROOT/bin/tests"
REPORTS_DIR="$PROJECT_ROOT/test-reports"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 测试配置
TEST_EXECUTABLE="test_authenticationui_complete"
TEST_TIMEOUT=300  # 5分钟超时
VERBOSE=false
COVERAGE=false
PERFORMANCE=false
QUICK_MODE=false
CI_MODE=false

# 打印带颜色的消息
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    echo ""
    print_message $CYAN "╔══════════════════════════════════════════════════════════════╗"
    print_message $CYAN "║                    UnravelChat 测试套件                      ║"
    print_message $CYAN "║                AuthenticationUI 模块测试                    ║"
    print_message $CYAN "╚══════════════════════════════════════════════════════════════╝"
    echo ""
}

print_usage() {
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help          显示帮助信息"
    echo "  -v, --verbose       详细输出模式"
    echo "  -q, --quick         快速测试模式（仅运行基本测试）"
    echo "  -c, --coverage      启用代码覆盖率分析"
    echo "  -p, --performance   运行性能基准测试"
    echo "  -ci, --ci-mode      CI/CD模式（机器友好输出）"
    echo "  --clean             清理构建文件后退出"
    echo "  --build-only        仅编译，不运行测试"
    echo "  --test-only         仅运行测试，不编译"
    echo ""
    echo "示例:"
    echo "  $0                  # 运行标准测试"
    echo "  $0 -v -c            # 详细模式 + 代码覆盖率"
    echo "  $0 -q               # 快速测试"
    echo "  $0 -p               # 性能测试"
    echo "  $0 --clean          # 清理构建文件"
}

# 解析命令行参数
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_usage
                exit 0
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -q|--quick)
                QUICK_MODE=true
                shift
                ;;
            -c|--coverage)
                COVERAGE=true
                shift
                ;;
            -p|--performance)
                PERFORMANCE=true
                shift
                ;;
            -ci|--ci-mode)
                CI_MODE=true
                shift
                ;;
            --clean)
                clean_build
                exit 0
                ;;
            --build-only)
                BUILD_ONLY=true
                shift
                ;;
            --test-only)
                TEST_ONLY=true
                shift
                ;;
            *)
                print_message $RED "错误: 未知选项 '$1'"
                print_usage
                exit 1
                ;;
        esac
    done
}

# 检查系统依赖
check_dependencies() {
    local missing_deps=()
    
    # 检查必需的工具
    command -v qmake >/dev/null 2>&1 || missing_deps+=("qmake")
    command -v make >/dev/null 2>&1 || missing_deps+=("make")
    command -v g++ >/dev/null 2>&1 || missing_deps+=("g++")
    
    # 检查可选工具
    if [[ $COVERAGE == true ]]; then
        command -v gcov >/dev/null 2>&1 || missing_deps+=("gcov")
        command -v lcov >/dev/null 2>&1 || missing_deps+=("lcov")
        command -v genhtml >/dev/null 2>&1 || missing_deps+=("genhtml")
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        print_message $RED "错误: 缺少必需的依赖:"
        for dep in "${missing_deps[@]}"; do
            print_message $RED "  - $dep"
        done
        exit 1
    fi
    
    # 检查Qt版本
    local qt_version=$(qmake -query QT_VERSION 2>/dev/null)
    if [[ -z "$qt_version" ]]; then
        print_message $RED "错误: 无法获取Qt版本信息"
        exit 1
    fi
    
    print_message $GREEN "✅ 依赖检查通过 (Qt $qt_version)"
}

# 创建目录
create_directories() {
    mkdir -p "$BUILD_DIR"
    mkdir -p "$BIN_DIR"
    mkdir -p "$REPORTS_DIR"
    
    if [[ $VERBOSE == true ]]; then
        print_message $BLUE "📁 创建目录结构:"
        print_message $BLUE "   构建目录: $BUILD_DIR"
        print_message $BLUE "   可执行文件目录: $BIN_DIR"
        print_message $BLUE "   报告目录: $REPORTS_DIR"
    fi
}

# 清理构建文件
clean_build() {
    print_message $YELLOW "🧹 清理构建文件..."
    
    rm -rf "$BUILD_DIR"
    rm -rf "$BIN_DIR"
    rm -f "$PROJECT_ROOT"/*.gcov
    rm -f "$PROJECT_ROOT"/*.gcda
    rm -f "$PROJECT_ROOT"/*.gcno
    rm -f "$PROJECT_ROOT"/coverage.info
    rm -rf "$PROJECT_ROOT"/coverage-html
    
    print_message $GREEN "✅ 清理完成"
}

# 编译测试程序
build_tests() {
    print_message $BLUE "🔨 编译AuthenticationUI测试程序..."
    
    cd "$TEST_DIR" || exit 1
    
    # 生成Makefile
    local qmake_args=""
    if [[ $COVERAGE == true ]]; then
        qmake_args="CONFIG+=coverage"
    fi
    
    if [[ $VERBOSE == true ]]; then
        print_message $BLUE "   运行: qmake test_authenticationui_complete.pro $qmake_args"
    fi
    
    if ! qmake test_authenticationui_complete.pro $qmake_args; then
        print_message $RED "❌ qmake失败"
        exit 1
    fi
    
    # 编译
    local make_jobs=$(nproc 2>/dev/null || echo "4")
    if [[ $VERBOSE == true ]]; then
        print_message $BLUE "   运行: make -j$make_jobs"
        make -j"$make_jobs"
    else
        make -j"$make_jobs" >/dev/null 2>&1
    fi
    
    if [[ $? -ne 0 ]]; then
        print_message $RED "❌ 编译失败"
        exit 1
    fi
    
    # 检查可执行文件
    if [[ ! -f "$BIN_DIR/$TEST_EXECUTABLE" ]]; then
        print_message $RED "❌ 测试可执行文件不存在: $BIN_DIR/$TEST_EXECUTABLE"
        exit 1
    fi
    
    print_message $GREEN "✅ 编译成功"
}

# 运行测试
run_tests() {
    print_message $BLUE "🧪 运行AuthenticationUI测试..."
    
    cd "$BIN_DIR" || exit 1
    
    local test_args=""
    local output_file="$REPORTS_DIR/test_results.xml"
    
    # 配置测试参数
    if [[ $VERBOSE == true ]]; then
        test_args="-v2"
    else
        test_args="-silent"
    fi
    
    if [[ $QUICK_MODE == true ]]; then
        test_args="$test_args -functions testUIInitialization,testPageNavigation,testLoginValidation"
    fi
    
    if [[ $CI_MODE == true ]]; then
        test_args="$test_args -o $output_file,xml -o -,txt"
    fi
    
    # 设置环境变量
    export QT_LOGGING_RULES="qt.qpa.xcb.xcb_error.debug=false"
    export DISPLAY=${DISPLAY:-:0}
    
    # 运行测试
    local start_time=$(date +%s)
    
    if [[ $VERBOSE == true ]]; then
        print_message $BLUE "   运行: ./$TEST_EXECUTABLE $test_args"
    fi
    
    timeout "$TEST_TIMEOUT" "./$TEST_EXECUTABLE" $test_args
    local test_result=$?
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    # 处理测试结果
    case $test_result in
        0)
            print_message $GREEN "✅ 所有测试通过 (${duration}秒)"
            ;;
        124)
            print_message $RED "❌ 测试超时 (${TEST_TIMEOUT}秒)"
            return 1
            ;;
        *)
            print_message $RED "❌ 测试失败 (退出码: $test_result)"
            return 1
            ;;
    esac
    
    return 0
}

# 生成代码覆盖率报告
generate_coverage_report() {
    if [[ $COVERAGE != true ]]; then
        return 0
    fi
    
    print_message $BLUE "📊 生成代码覆盖率报告..."
    
    cd "$PROJECT_ROOT" || exit 1
    
    # 收集覆盖率数据
    if ! lcov --capture --directory . --output-file coverage.info >/dev/null 2>&1; then
        print_message $YELLOW "⚠️  无法生成覆盖率数据"
        return 1
    fi
    
    # 过滤系统文件
    lcov --remove coverage.info '/usr/*' --output-file coverage_filtered.info >/dev/null 2>&1
    lcov --remove coverage_filtered.info '*/tests/*' --output-file coverage_final.info >/dev/null 2>&1
    
    # 生成HTML报告
    if genhtml coverage_final.info --output-directory "$REPORTS_DIR/coverage-html" >/dev/null 2>&1; then
        print_message $GREEN "✅ 覆盖率报告生成完成: $REPORTS_DIR/coverage-html/index.html"
    else
        print_message $YELLOW "⚠️  无法生成HTML覆盖率报告"
    fi
    
    # 显示覆盖率摘要
    if command -v lcov >/dev/null 2>&1; then
        local coverage_summary=$(lcov --summary coverage_final.info 2>/dev/null | grep "lines")
        if [[ -n "$coverage_summary" ]]; then
            print_message $CYAN "📈 覆盖率摘要: $coverage_summary"
        fi
    fi
}

# 运行性能测试
run_performance_tests() {
    if [[ $PERFORMANCE != true ]]; then
        return 0
    fi
    
    print_message $BLUE "⚡ 运行性能基准测试..."
    
    cd "$BIN_DIR" || exit 1
    
    # 运行性能相关的测试
    local perf_tests="testMemoryUsage,testResponseTimes,testAnimationPerformance,testConcurrentOperations"
    
    if "./$TEST_EXECUTABLE" -functions "$perf_tests" -silent; then
        print_message $GREEN "✅ 性能测试完成"
    else
        print_message $YELLOW "⚠️  性能测试执行异常"
    fi
}

# 生成测试报告摘要
generate_test_summary() {
    local summary_file="$REPORTS_DIR/test_summary.txt"
    
    {
        echo "UnravelChat AuthenticationUI 测试报告"
        echo "========================================"
        echo "测试时间: $(date)"
        echo "测试模式: $([ $QUICK_MODE == true ] && echo "快速模式" || echo "完整模式")"
        echo "覆盖率分析: $([ $COVERAGE == true ] && echo "已启用" || echo "未启用")"
        echo "性能测试: $([ $PERFORMANCE == true ] && echo "已启用" || echo "未启用")"
        echo ""
        
        if [[ -f "$REPORTS_DIR/test_results.xml" ]]; then
            echo "详细测试结果: test_results.xml"
        fi
        
        if [[ -d "$REPORTS_DIR/coverage-html" ]]; then
            echo "覆盖率报告: coverage-html/index.html"
        fi
        
        echo ""
        echo "测试环境信息:"
        echo "  操作系统: $(uname -s) $(uname -r)"
        echo "  Qt版本: $(qmake -query QT_VERSION 2>/dev/null || echo "未知")"
        echo "  编译器: $(g++ --version 2>/dev/null | head -n1 || echo "未知")"
        echo "  CPU: $(nproc 2>/dev/null || echo "未知") 核心"
        
    } > "$summary_file"
    
    if [[ $CI_MODE == true ]]; then
        print_message $CYAN "📋 测试摘要已保存: $summary_file"
    else
        print_message $CYAN "📋 测试摘要:"
        cat "$summary_file"
    fi
}

# 主函数
main() {
    print_header
    
    parse_arguments "$@"
    
    # 检查系统环境
    check_dependencies
    create_directories
    
    # 执行构建和测试
    local overall_result=0
    
    if [[ $TEST_ONLY != true ]]; then
        build_tests || overall_result=1
    fi
    
    if [[ $BUILD_ONLY != true && $overall_result -eq 0 ]]; then
        run_tests || overall_result=1
        generate_coverage_report
        run_performance_tests
    fi
    
    # 生成报告
    generate_test_summary
    
    # 最终结果
    echo ""
    if [[ $overall_result -eq 0 ]]; then
        print_message $GREEN "🎉 AuthenticationUI测试全部完成！"
        if [[ $CI_MODE == true ]]; then
            echo "TEST_RESULT=PASS" >> "$GITHUB_ENV" 2>/dev/null || true
        fi
    else
        print_message $RED "💥 测试失败！"
        if [[ $CI_MODE == true ]]; then
            echo "TEST_RESULT=FAIL" >> "$GITHUB_ENV" 2>/dev/null || true
        fi
    fi
    
    exit $overall_result
}

# 信号处理
trap 'print_message $RED "🛑 测试被中断"; exit 130' INT TERM

# 运行主函数
main "$@"