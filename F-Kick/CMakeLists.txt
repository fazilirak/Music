# Fazıl Irak · 27.10.2026 · F-Kick derleme ve doğrulama hedefleri.
cmake_minimum_required(VERSION 3.24)
project(FKick VERSION 0.8.2 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options("$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/JUCE/CMakeLists.txt")
    set(default_juce "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/JUCE")
else()
    set(default_juce "${CMAKE_CURRENT_SOURCE_DIR}/../_Dependencies/JUCE")
endif()
set(JUCE_PATH "${default_juce}" CACHE PATH "JUCE source directory")
add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")
juce_add_binary_data(FKickResources NAMESPACE fkick_data SOURCES
    Resources/Ubuntu-Medium.ttf LICENSE.txt THIRD-PARTY-NOTICES.txt
    licenses/AGPL-3.0.txt licenses/GPL-3.0.txt licenses/JUCE-LICENSE.md licenses/Ubuntu-Font-Licence-1.0.txt licenses/VST3-LICENSE.txt)
set(formats VST3 Standalone)
if(APPLE)
    list(APPEND formats AU)
endif()
juce_add_plugin(FKick COMPANY_NAME "FAZIL IRAK" BUNDLE_ID "com.fazilirak.fkick"
    PLUGIN_MANUFACTURER_CODE FzIr PLUGIN_CODE FkK1 PRODUCT_NAME "F-Kick"
    FORMATS ${formats} IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Instrument Synth Sampler COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(FKick)
target_sources(FKick PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/Engine.cpp Source/TechnoFX.cpp Source/NativeFileDrop.cpp)
target_include_directories(FKick PUBLIC Source)
target_compile_definitions(FKick PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    FKICK_VERSION="${PROJECT_VERSION}")
target_link_libraries(FKick PRIVATE FKickResources PUBLIC juce::juce_audio_utils juce::juce_dsp
    juce::juce_recommended_config_flags)
enable_testing()
juce_add_console_app(FKickTests PRODUCT_NAME "F-Kick Tests")
target_sources(FKickTests PRIVATE Tests/AudioTests.cpp Tests/TechnoTests.cpp Tests/BankTests.cpp Tests/ImpactTests.cpp Tests/FXRackTests.cpp Tests/CleanTests.cpp)
target_compile_definitions(FKickTests PRIVATE JUCE_PLUGINHOST_VST3=1 JUCE_MODAL_LOOPS_PERMITTED=1)
target_compile_definitions(FKickTests PRIVATE FKICK_TEST_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/Tests/Fixtures")
target_link_libraries(FKickTests PRIVATE FKick FKickResources)
add_dependencies(FKickTests FKick_VST3)
add_test(NAME fkick_audio_state_gui COMMAND FKickTests "${CMAKE_CURRENT_SOURCE_DIR}/validation" "${CMAKE_BINARY_DIR}/FKick_artefacts/$<CONFIG>/VST3/F-Kick.vst3")
if(WIN32)
    # JUCE info.txt değişince RC de yeniden üretilmeli; Windows sürüm alanları eski kalmasın.
    foreach(resource_target FKick FKickTests)
        get_target_property(resource_generated ${resource_target} JUCE_GENERATED_SOURCES_DIRECTORY)
        get_target_property(resource_info ${resource_target} JUCE_INFO_FILE)
        add_custom_command(OUTPUT "${resource_generated}/${resource_target}_resources.rc" APPEND DEPENDS "${resource_info}")
    endforeach()
endif()
