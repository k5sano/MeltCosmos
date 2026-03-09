```cmake
Copycmake_minimum_required(VERSION 3.22)
project(EMSpace VERSION 1.0.0)

add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(EMSpace
    PLUGIN_MANUFACTURER_CODE K5sn
    PLUGIN_CODE EmSp
    COMPANY_NAME "K5SANO"
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "EMSpace"
)

juce_generate_juce_header(EMSpace)

target_sources(EMSpace PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/PresetManager.cpp
    Source/DSP/SpaceEngine.cpp
    Source/DSP/ReverbTank.cpp
)

target_include_directories(EMSpace PRIVATE
    Source
    Source/DSP
)

target_compile_features(EMSpace PRIVATE cxx_std_17)

target_compile_definitions(EMSpace PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
)

if(MSVC)
    target_compile_definitions(EMSpace PRIVATE _USE_MATH_DEFINES)
endif()

target_link_libraries(EMSpace
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_audio_plugin_client
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
)

# --- Tests ---

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()

    find_package(Catch2 3 QUIET)
    if(Catch2_FOUND)
        add_executable(EMSpaceTests
            Tests/SpaceEngineTests.cpp
            Source/DSP/SpaceEngine.cpp
            Source/DSP/ReverbTank.cpp
        )
        target_include_directories(EMSpaceTests PRIVATE
            Source
            Source/DSP
        )
        target_compile_features(EMSpaceTests PRIVATE cxx_std_17)
        target_link_libraries(EMSpaceTests PRIVATE
            Catch2::Catch2WithMain
        )
        if(MSVC)
            target_compile_definitions(EMSpaceTests PRIVATE _USE_MATH_DEFINES)
        endif()

        include(Catch2::Catch)
        catch_discover_tests(EMSpaceTests)
    endif()
endif()
Copy
```