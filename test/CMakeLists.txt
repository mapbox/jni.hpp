add_executable(low_level low_level.cpp)
target_link_libraries(low_level Mapbox::JNI)
target_include_directories(low_level PRIVATE android)

add_executable(high_level high_level.cpp)
target_link_libraries(high_level Mapbox::JNI)
target_include_directories(high_level PRIVATE android)
