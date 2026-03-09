cmake\_minimum\_required(VERSION 3.22) project(EMverbAvalanche VERSION 1.0.0)

add\_subdirectory(/Users/sanokeigo/JUCE ${CMAKE\_BINARY\_DIR}/JUCE)

juce\_add\_plugin(EMverbAvalanche PLUGIN\_MANUFACTURER\_CODE K5sn PLUGIN\_CODE EmAv FORMATS AU VST3 Standalone PRODUCT\_NAME "EMverb Avalanche" )

juce\_generate\_juce\_header(EMverbAvalanche)

target\_sources(EMverbAvalanche PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/DSP/DattorroReverb.cpp Source/DSP/AvalancheEngine.cpp Source/SampleRateAdapter.cpp )

target\_include\_directories(EMverbAvalanche PRIVATE Source Source/DSP )

target\_compile\_features(EMverbAvalanche PRIVATE cxx\_std\_17)

target\_compile\_definitions(EMverbAvalanche PRIVATE JUCE\_VST3\_CAN\_REPLACE\_VST2=0 )

if(MSVC) target\_compile\_definitions(EMverbAvalanche PRIVATE \_USE\_MATH\_DEFINES) endif()

target\_link\_libraries(EMverbAvalanche PRIVATE juce::juce\_audio\_basics juce::juce\_audio\_processors juce::juce\_audio\_plugin\_client juce::juce\_audio\_devices juce::juce\_audio\_utils juce::juce\_dsp PUBLIC juce::juce\_recommended\_config\_flags )

# \--- Tests ---

option(BUILD\_TESTS "Build unit tests" ON) if(BUILD\_TESTS) enable\_testing()

```
find_package(Catch2 3 QUIET)
if(Catch2_FOUND)
    add_executable(EMverbAvalancheTests
        Tests/AvalancheTests.cpp
        Source/DSP/DattorroReverb.cpp
        Source/DSP/AvalancheEngine.cpp
        Source/SampleRateAdapter.cpp
    )
    target_include_directories(EMverbAvalancheTests PRIVATE
        Source
        Source/DSP
    )
    target_compile_features(EMverbAvalancheTests PRIVATE cxx_std_17)
    target_link_libraries(EMverbAvalancheTests PRIVATE
        Catch2::Catch2WithMain
    )
    if(MSVC)
        target_compile_definitions(EMverbAvalancheTests PRIVATE _USE_MATH_DEFINES)
    endif()

    include(Catch2::Catch)
    catch_discover_tests(EMverbAvalancheTests)
endif()
```

endif()