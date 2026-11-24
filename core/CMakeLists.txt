cmake_minimum_required(VERSION 3.16)
project(maksa_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAKSA_CORE_SOURCES
  exercise_catalog.cc
  routine_planner.cc
  exercise_session.cc
  settings.cc
  exercise_view_adapter.cc
  coordinator.cc
)

add_library(maksa_core STATIC ${MAKSA_CORE_SOURCES})
target_include_directories(maksa_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(maksa_core_tests tests/run_tests.cc)
add_executable(maksa_stage2_tests tests/run_stage2_tests.cc)
add_executable(maksa_plan_demo tools/plan_demo.cc)
add_executable(maksa_e2e_demo tools/e2e_demo.cc)

foreach(target maksa_core_tests maksa_stage2_tests maksa_plan_demo maksa_e2e_demo)
  target_link_libraries(${target} PRIVATE maksa_core)
endforeach()

enable_testing()
add_test(NAME maksa_stage1
         COMMAND maksa_core_tests ${CMAKE_CURRENT_SOURCE_DIR}/data/test_exercises.tsv)
add_test(NAME maksa_stage2
         COMMAND maksa_stage2_tests ${CMAKE_CURRENT_SOURCE_DIR}/data/test_exercises.tsv)
