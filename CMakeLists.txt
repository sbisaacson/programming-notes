cmake_minimum_required(VERSION 3.18)

project(programming-notes LANGUAGES C CXX)

enable_testing()

include(FetchContent)
find_package(TBB REQUIRED)
find_package(Boost REQUIRED)
find_package(Microsoft.GSL REQUIRED)

FetchContent_declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG main
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(cxx_notes general_notes.cc tbb_notes.cc ieee754_notes.cc avx2_notes.cc)
set_property(TARGET cxx_notes PROPERTY CXX_STANDARD 17)
target_link_libraries(cxx_notes gtest_main TBB::tbb TBB::tbbmalloc Boost::headers Microsoft.GSL::GSL gmp)
target_compile_options(cxx_notes PUBLIC -march=native)
gtest_discover_tests(cxx_notes)

add_subdirectory(backtrace)
