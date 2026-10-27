# Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
# Dosyanın görevi: Efekti, gömülü fontları ve doğrulama uygulamasını derlemek.
# SPDX-License-Identifier: AGPL-3.0-or-later
cmake_minimum_required(VERSION 3.24)
project(FTransForm VERSION 0.2.2 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options("$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>")
# Ortak JUCE kaynağını değiştirmeden kullanır. Başka bilgisayarda bu yolu override edin.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/JUCE/CMakeLists.txt")
  set(ft_default_juce "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/JUCE")
else()
  set(ft_default_juce "${CMAKE_CURRENT_SOURCE_DIR}/../_Dependencies/JUCE")
endif()
set(FT_JUCE_PATH "${ft_default_juce}" CACHE PATH "JUCE source")
add_subdirectory("${FT_JUCE_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE")
set(ft_formats VST3 Standalone)
if(APPLE)
  list(APPEND ft_formats AU)
endif()
juce_add_plugin(FTransForm COMPANY_NAME "FAZIL IRAK" BUNDLE_ID "com.fazilirak.ftransform"
  PLUGIN_MANUFACTURER_CODE FIrk PLUGIN_CODE FTrf PRODUCT_NAME "F-TransForm"
  FORMATS ${ft_formats} IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE MICROPHONE_PERMISSION_ENABLED TRUE
  MICROPHONE_PERMISSION_TEXT "F-TransForm processes your audio input.")
juce_generate_juce_header(FTransForm)
# Ürün sürümü değiştiğinde Windows dosya özelliklerindeki sürüm de yeniden derlenmelidir.
if(WIN32)
  get_target_property(ft_generated FTransForm JUCE_GENERATED_SOURCES_DIRECTORY)
  get_target_property(ft_info FTransForm JUCE_INFO_FILE)
  add_custom_command(OUTPUT "${ft_generated}/FTransForm_resources.rc" APPEND DEPENDS "${ft_info}")
endif()
# Fontlar ikiliye gömülür; panel kullanıcının yüklü fontlarından etkilenmez.
juce_add_binary_data(FTFonts NAMESPACE ft_fonts HEADER_NAME FTFonts.h
  SOURCES Resources/fonts/Book.otf Resources/fonts/Demi.otf)
juce_add_binary_data(FTLicences NAMESPACE ft_licences HEADER_NAME FTLicences.h SOURCES
  licenses/Overview.txt licenses/AGPL.txt licenses/Notices.txt licenses/JUCE.txt
  licenses/Font.txt licenses/VST3.txt licenses/GPL.txt)
target_sources(FTransForm PRIVATE Source/TransformDSP.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/AboutPanel.cpp)
target_include_directories(FTransForm PUBLIC Source)
target_compile_definitions(FTransForm PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 FT_VERSION="${PROJECT_VERSION}")
target_link_libraries(FTransForm PRIVATE FTFonts FTLicences juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags)
enable_testing()
juce_add_console_app(FTTests PRODUCT_NAME "F-TransForm Checks")
target_sources(FTTests PRIVATE Tests/Checks.cpp)
target_link_libraries(FTTests PRIVATE FTransForm)
target_include_directories(FTTests PRIVATE "${FT_JUCE_PATH}/modules")
target_compile_definitions(FTTests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
add_test(NAME transform_checks COMMAND FTTests "${CMAKE_CURRENT_SOURCE_DIR}/dist")
