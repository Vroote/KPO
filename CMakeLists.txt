cmake_minimum_required(VERSION 3.20)
project(KPO_v2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(BOOST_DI_OPT_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(boost_di
        GIT_REPOSITORY https://github.com/boost-ext/di.git
        GIT_TAG v1.3.2
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(boost_di)

add_library(vyshkat_core
        src/equipment.cpp
        src/Equipment_factory.cpp
        src/Fleet.cpp
        src/ServiceCenter_service.cpp
        src/Intake_service.cpp
        src/Beginner_policy.cpp
        src/Fleet_reports.cpp
        src/app.cpp
)
target_include_directories(vyshkat_core PUBLIC include)
target_link_libraries(vyshkat_core PUBLIC Boost.DI)

add_executable(KPO main.cpp)
target_link_libraries(KPO PRIVATE vyshkat_core Boost.DI)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(vyshkat_tests
        tests/test_equipment.cpp
        tests/test_equipment_factory.cpp
        tests/test_fleet.cpp
        tests/test_service_center.cpp
        tests/test_intake_service.cpp
        tests/test_beginner_policy.cpp
        tests/test_fleet_reports.cpp
        tests/test_app.cpp
        tests/test_di_container.cpp
)
target_include_directories(vyshkat_tests PRIVATE tests)
target_link_libraries(vyshkat_tests PRIVATE vyshkat_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(vyshkat_tests)
