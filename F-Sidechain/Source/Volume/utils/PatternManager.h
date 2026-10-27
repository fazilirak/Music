// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Kullanıcı eğrilerinin düzenlenmesi, sıralanması ve durum dönüşümü.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include <memory>
#include <functional>

namespace volumef {

// Forward declarations
class Pattern;
class Sequencer;
struct TensionParameters;

/**
 * PatternManager handles import/export operations for patterns.
 * This class provides functionality to save and load pattern data
 * in the TIME12 plugin's custom format.
 */
class PatternManager
{
public:
    PatternManager() = default;
    ~PatternManager() = default;    
    /**
     * Import patterns from a .12pat file
     * @param patterns Array of 12 Pattern pointers to import into
     * @param sequencer Pointer to the sequencer for UI mode handling
     * @param tensionParameters Struct holding the tension parameters
     */
    void importPatterns(Pattern* patterns[12],const TensionParameters& tensionParameters);

    /**
     * Export patterns to a .12pat file
     * @param patterns Array of 12 Pattern pointers to export from
     * @param sequencer Pointer to the sequencer to handle backup data
     */
    void exportPatterns(Pattern* patterns[12]);

private:
    static constexpr const char* patternExtension= "*.12pat";
    static constexpr const char* exportWindowTitle= "Export Patterns to a file";
    static constexpr const char* importWindowTitle = "Import Patterns from a file";
    std::unique_ptr<juce::FileChooser> mFileChooser;
    juce::ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternManager)
};

} // namespace volumef
