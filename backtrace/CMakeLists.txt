add_link_options(-rdynamic)
add_executable(trace trace.cc)
set_property(TARGET trace PROPERTY CXX_STANDARD 17)
target_link_libraries(trace PUBLIC Boost::headers backtrace dl)
