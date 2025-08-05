# CMakeLists.txt for AuthenticationUI Test
# Author: unravel
# Date: 2025-01-29
# Framework: CMake 3.16+ with Qt6
# Task: AuthenticationUI模块测试的CMake构建配置

cmake_minimum_required(VERSION 3.16)

# 项目信息
project(UnravelChatAuthTest 
    VERSION 1.0.0
    DESCRIPTION "UnravelChat AuthenticationUI Module Test Suite"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif()

# 编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Test
    Network
    Sql
)

# 启用Qt MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 启用测试
enable_testing()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/../../include
    ${CMAKE_SOURCE_DIR}/../../include/auth
    ${CMAKE_SOURCE_DIR}/../../include/core
    ${CMAKE_SOURCE_DIR}
)

# 预处理器定义
add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DTESTING_MODE
    -DQT_USE_QSTRINGBUILDER
    -DUNRAVEL_CHAT_VERSION=\"1.0.0\"
)

# 调试模式定义
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
else()
    add_definitions(-DRELEASE_MODE)
endif()

# 平台特定设置
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(PLATFORM_LIBS ws2_32 user32 advapi32 shell32 ole32 uuid)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LIBS dl pthread X11)
elseif(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
    set(PLATFORM_LIBS ${COREFOUNDATION_LIBRARY} ${SECURITY_LIBRARY})
endif()

# 查找OpenSSL
find_package(OpenSSL REQUIRED)

# 源文件
set(AUTH_SOURCES
    ../../src/auth/AuthenticationUI.cpp
    ../../src/auth/AuthenticationManager.cpp
    ../../src/auth/SessionManager.cpp
    ../../src/auth/PasswordManager.cpp
    ../../src/auth/UserDatabase.cpp
)

set(AUTH_HEADERS
    ../../include/auth/AuthenticationUI.h
    ../../include/auth/AuthenticationManager.h
    ../../include/auth/SessionManager.h
    ../../include/auth/PasswordManager.h
    ../../include/auth/UserDatabase.h
)

set(TEST_SOURCES
    test_authenticationui_complete.cpp
)

# 创建测试可执行文件
add_executable(test_authenticationui_complete
    ${TEST_SOURCES}
    ${AUTH_SOURCES}
    ${AUTH_HEADERS}
)

# 链接库
target_link_libraries(test_authenticationui_complete
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    Qt6::Network
    Qt6::Sql
    OpenSSL::Crypto
    ${PLATFORM_LIBS}
)

# 设置目标属性
set_target_properties(test_authenticationui_complete PROPERTIES
    OUTPUT_NAME "test_authenticationui_complete"
    DEBUG_POSTFIX "_debug"
)

# 添加测试
add_test(NAME AuthenticationUI_Tests
    COMMAND test_authenticationui_complete
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 测试环境变量
set_tests_properties(AuthenticationUI_Tests PROPERTIES
    ENVIRONMENT "QT_LOGGING_RULES=qt.qpa.xcb.xcb_error.debug=false"
    TIMEOUT 300
)

# 代码覆盖率配置
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(test_authenticationui_complete PRIVATE --coverage)
        target_link_libraries(test_authenticationui_complete gcov)
        
        # 添加覆盖率目标
        find_program(GCOV_PATH gcov)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)
        
        if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
            add_custom_target(coverage
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage_filtered.info
                COMMAND ${LCOV_PATH} --remove coverage_filtered.info '*/tests/*' --output-file coverage_final.info
                COMMAND ${GENHTML_PATH} coverage_final.info --output-directory coverage-html
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report"
            )
            
            add_dependencies(coverage test_authenticationui_complete)
        endif()
    endif()
endif()

# 内存检查配置
option(ENABLE_MEMCHECK "Enable memory checking with Valgrind" OFF)
if(ENABLE_MEMCHECK)
    find_program(VALGRIND_PATH valgrind)
    if(VALGRIND_PATH)
        add_custom_target(memcheck
            COMMAND ${VALGRIND_PATH} --tool=memcheck --leak-check=full 
                    --show-leak-kinds=all --track-origins=yes --verbose 
                    --log-file=valgrind.log $<TARGET_FILE:test_authenticationui_complete>
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running memory check with Valgrind"
        )
        
        add_dependencies(memcheck test_authenticationui_complete)
    endif()
endif()

# 静态分析配置
option(ENABLE_STATIC_ANALYSIS "Enable static analysis with cppcheck" OFF)
if(ENABLE_STATIC_ANALYSIS)
    find_program(CPPCHECK_PATH cppcheck)
    if(CPPCHECK_PATH)
        add_custom_target(static_analysis
            COMMAND ${CPPCHECK_PATH} --enable=all --xml --xml-version=2
                    --output-file=cppcheck.xml
                    ${CMAKE_SOURCE_DIR}/../../src/auth/
                    ${CMAKE_SOURCE_DIR}/../../include/auth/
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running static analysis with cppcheck"
        )
    endif()
endif()

# 性能分析配置
option(ENABLE_PROFILING "Enable profiling with gprof" OFF)
if(ENABLE_PROFILING)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(test_authenticationui_complete PRIVATE -pg)
        target_link_options(test_authenticationui_complete PRIVATE -pg)
        
        add_custom_target(profile
            COMMAND $<TARGET_FILE:test_authenticationui_complete>
            COMMAND gprof $<TARGET_FILE:test_authenticationui_complete> gmon.out > profile.txt
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running performance profiling"
        )
        
        add_dependencies(profile test_authenticationui_complete)
    endif()
endif()

# 文档生成配置
option(ENABLE_DOCS "Enable documentation generation" OFF)
if(ENABLE_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_INPUT ${CMAKE_SOURCE_DIR}/../../src/auth/ ${CMAKE_SOURCE_DIR}/../../include/auth/)
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
        
        configure_file(${CMAKE_SOURCE_DIR}/../../docs/Doxyfile.in 
                       ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
        
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
        )
    endif()
endif()

# 安装配置
install(TARGETS test_authenticationui_complete
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 测试数据文件安装
if(EXISTS ${CMAKE_SOURCE_DIR}/test_data)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/test_data
        DESTINATION share/unravelchat/tests
    )
endif()

# 自定义目标：快速测试
add_custom_target(quick_test
    COMMAND $<TARGET_FILE:test_authenticationui_complete> 
            -functions testUIInitialization,testPageNavigation,testLoginValidation
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running quick tests"
)
add_dependencies(quick_test test_authenticationui_complete)

# 自定义目标：性能测试
add_custom_target(perf_test
    COMMAND $<TARGET_FILE:test_authenticationui_complete>
            -functions testMemoryUsage,testResponseTimes,testAnimationPerformance
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running performance tests"
)
add_dependencies(perf_test test_authenticationui_complete)

# 自定义目标：集成测试
add_custom_target(integration_test
    COMMAND $<TARGET_FILE:test_authenticationui_complete>
            -functions testEmailServiceIntegration,testNetworkErrorHandling
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running integration tests"
)
add_dependencies(integration_test test_authenticationui_complete)

# 自定义目标：全部测试
add_custom_target(run_all_tests
    COMMAND $<TARGET_FILE:test_authenticationui_complete> -v2
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running all tests with verbose output"
)
add_dependencies(run_all_tests test_authenticationui_complete)

# 自定义目标：清理测试数据
add_custom_target(clean_test_data
    COMMAND ${CMAKE_COMMAND} -E remove_directory test_temp
    COMMAND ${CMAKE_COMMAND} -E remove *.gcov *.gcda *.gcno
    COMMAND ${CMAKE_COMMAND} -E remove coverage.info valgrind.log profile.txt
    COMMAND ${CMAKE_COMMAND} -E remove_directory coverage-html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning test data and temporary files"
)

# CTest配置
include(CTest)
if(BUILD_TESTING)
    # 添加不同类型的测试
    add_test(NAME UI_Initialization 
             COMMAND test_authenticationui_complete -functions testUIInitialization)
    
    add_test(NAME Page_Navigation 
             COMMAND test_authenticationui_complete -functions testPageNavigation)
    
    add_test(NAME Form_Validation 
             COMMAND test_authenticationui_complete -functions testLoginValidation,testRegistrationValidation)
    
    add_test(NAME Password_Strength 
             COMMAND test_authenticationui_complete -functions testPasswordStrengthIndicator)
    
    add_test(NAME Session_Management 
             COMMAND test_authenticationui_complete -functions testSessionDisplay,testSessionRefresh)
    
    # 设置测试标签
    set_tests_properties(UI_Initialization PROPERTIES LABELS "ui;basic")
    set_tests_properties(Page_Navigation PROPERTIES LABELS "ui;navigation")
    set_tests_properties(Form_Validation PROPERTIES LABELS "validation;forms")
    set_tests_properties(Password_Strength PROPERTIES LABELS "security;validation")
    set_tests_properties(Session_Management PROPERTIES LABELS "session;management")
    
    # 设置测试超时
    set_tests_properties(UI_Initialization Page_Navigation PROPERTIES TIMEOUT 30)
    set_tests_properties(Form_Validation Password_Strength PROPERTIES TIMEOUT 60)
    set_tests_properties(Session_Management PROPERTIES TIMEOUT 45)
endif()

# 打包配置
set(CPACK_PACKAGE_NAME "UnravelChat-AuthTest")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "UnravelChat Team")
set(CPACK_PACKAGE_CONTACT "unravel@example.com")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "ZIP;DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

include(CPack)

# 功能摘要
message(STATUS "=== UnravelChat AuthenticationUI Test Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt version: ${Qt6_VERSION}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")

message(STATUS "Optional features:")
message(STATUS "  Code coverage: ${ENABLE_COVERAGE}")
message(STATUS "  Memory checking: ${ENABLE_MEMCHECK}")
message(STATUS "  Static analysis: ${ENABLE_STATIC_ANALYSIS}")
message(STATUS "  Profiling: ${ENABLE_PROFILING}")
message(STATUS "  Documentation: ${ENABLE_DOCS}")
message(STATUS "")

message(STATUS "Available targets:")
message(STATUS "  test_authenticationui_complete - Main test executable")
message(STATUS "  quick_test                     - Run basic tests only")
message(STATUS "  perf_test                      - Run performance tests")
message(STATUS "  integration_test               - Run integration tests")
message(STATUS "  run_all_tests                  - Run all tests with verbose output")
if(ENABLE_COVERAGE)
    message(STATUS "  coverage                       - Generate coverage report")
endif()
if(ENABLE_MEMCHECK)
    message(STATUS "  memcheck                       - Run memory analysis")
endif()
if(ENABLE_STATIC_ANALYSIS)
    message(STATUS "  static_analysis                - Run static code analysis")
endif()
if(ENABLE_PROFILING)
    message(STATUS "  profile                        - Run performance profiling")
endif()
if(ENABLE_DOCS)
    message(STATUS "  docs                           - Generate documentation")
endif()
message(STATUS "  clean_test_data                - Clean temporary files")
message(STATUS "")

# 警告检查
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/../../include/auth/AuthenticationUI.h)
    message(WARNING "AuthenticationUI.h header file not found!")
endif()

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/../../src/auth/AuthenticationUI.cpp)
    message(WARNING "AuthenticationUI.cpp source file not found!")
endif()

# Qt版本检查
if(Qt6_VERSION VERSION_LESS "6.0.0")
    message(FATAL_ERROR "Qt 6.0.0 or higher is required!")
endif()

# 编译器检查
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
    message(WARNING "GCC 7.0 or higher is recommended for best C++11 support")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0")
    message(WARNING "Clang 5.0 or higher is recommended for best C++11 support")
endif()

message(STATUS "=== Configuration Complete ===")
message(STATUS "Run 'make' or 'cmake --build .' to build the test suite")
message(STATUS "Run 'ctest' or 'make test' to execute tests")
message(STATUS "===============================================")