# F Saturator 0.2.0 derleme tanımı. C++17 ve JUCE kullanır; Windows VST3/Standalone hedefleri üretir.
cmake_minimum_required(VERSION 3.24)
project(FSaturator VERSION 0.2.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
# MSVC çalışma zamanı statik bağlanır; /utf-8 Türkçe kaynak açıklamalarının doğru okunmasını sağlar.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options("$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>")
# Yerel geliştirme için mevcut JUCE kopyasını arar. GitHub’dan indiren kişi
# -DIRAK_JUCE_PATH=... ile Research/dependencies.json içindeki commit’e ait JUCE yolunu vermelidir.
set(f_saturator_default_juce "${CMAKE_CURRENT_SOURCE_DIR}/../upstream/gate12/libs/JUCE")
if(NOT EXISTS "${f_saturator_default_juce}/CMakeLists.txt")
    set(f_saturator_default_juce "${CMAKE_CURRENT_SOURCE_DIR}/../Shaper/upstream/gate12/libs/JUCE")
endif()
set(IRAK_JUCE_PATH "${f_saturator_default_juce}" CACHE PATH "JUCE checkout")
add_subdirectory("${IRAK_JUCE_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE")
# IRAK hedefi ve FIrk/Irak kimlikleri eski oturum uyumluluğu içindir; görünen ürün adı F Saturator’dır.
# Bu hedef bir ses efektidir; MIDI veya CLAP hedefi tanımlı değildir.
juce_add_plugin(IRAK COMPANY_NAME "FAZIL IRAK" BUNDLE_ID "com.fazilirak.irak" PLUGIN_MANUFACTURER_CODE FIrk PLUGIN_CODE Irak
    PRODUCT_NAME "F Saturator" FORMATS VST3 Standalone IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(IRAK)
# Windows kaynak dosyasının ürün bilgisi değiştiğinde yeniden üretilmesi için bağımlılık ekler.
if(WIN32)
    get_target_property(fs_generated IRAK JUCE_GENERATED_SOURCES_DIRECTORY)
    get_target_property(fs_info IRAK JUCE_INFO_FILE)
    add_custom_command(OUTPUT "${fs_generated}/IRAK_resources.rc" APPEND DEPENDS "${fs_info}")
endif()
# Book/Demi fontları ikiliye gömülür; Resources/fonts içindeki lisanslar dağıtımda korunur.
juce_add_binary_data(FSaturatorFonts NAMESPACE fsaturator_fonts HEADER_NAME FSaturatorFonts.h SOURCES Resources/fonts/Book.otf Resources/fonts/Demi.otf)
target_link_libraries(IRAK PRIVATE FSaturatorFonts)
# Web tarayıcısı ve curl devre dışıdır; VST3’ün VST2 yerine geçme davranışı kapatılır.
target_compile_definitions(IRAK PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
# Ses motoru, DAW köprüsü, düzenleyici ve JUCE giriş dosyaları bu hedefte derlenir.
target_sources(IRAK PRIVATE Source/SaturationDSP.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/Entry.cpp)
target_include_directories(IRAK PUBLIC Source)
target_link_libraries(IRAK PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags)
# IRAKTests uygulaması CTest’e irak_dsp adıyla eklenir; testler preset/rapor/görsel de üretir.
enable_testing()
juce_add_console_app(IRAKTests PRODUCT_NAME "IRAK DSP Tests")
target_sources(IRAKTests PRIVATE Tests/DSPTests.cpp)
target_include_directories(IRAKTests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/IRAK_artefacts/JuceLibraryCode" "${IRAK_JUCE_PATH}/modules")
target_compile_definitions(IRAKTests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
target_link_libraries(IRAKTests PRIVATE IRAK)
add_test(NAME irak_dsp COMMAND IRAKTests)
