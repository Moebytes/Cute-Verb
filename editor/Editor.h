#pragma once
#pragma clang diagnostic ignored "-Wshadow-field"
#include <JuceHeader.h>
#include "Processor.h"
#include "EventEmitter.hpp"

class Editor : public AudioProcessorEditor, public EventEmitter::Listener {
public:
    Editor(Processor& p);
    ~Editor() override;
    
    auto resized() -> void override;

    auto getResource(const String& url) -> std::optional<WebBrowserComponent::Resource>;
    auto webviewOptions() -> WebBrowserComponent::Options;
    auto getWebviewFileBytes(const String& resourceStr) -> std::vector<std::byte>;

    auto handleEvent(const String& name, const var& payload) -> void override;
    auto handleThemeChange(const String& theme) -> void;
        
private:
    Processor& processor;
    ComponentBoundsConstrainer constrainer;

    WebSliderRelay mixRelay {Parameters::paramIDs.mix.getParamID()};
    WebSliderParameterAttachment mixAttachment {*this->processor.parameters.mixParam, mixRelay, nullptr};

    WebSliderRelay decayRelay {Parameters::paramIDs.decay.getParamID()};
    WebSliderParameterAttachment decayAttachment {*this->processor.parameters.decayParam, decayRelay, nullptr};

    WebSliderRelay predelayRelay {Parameters::paramIDs.predelay.getParamID()};
    WebSliderParameterAttachment predelayAttachment {*this->processor.parameters.predelayParam, predelayRelay, nullptr};

    WebSliderRelay sizeRelay {Parameters::paramIDs.size.getParamID()};
    WebSliderParameterAttachment sizeAttachment {*this->processor.parameters.sizeParam, sizeRelay, nullptr};
    
    WebSliderRelay widthRelay {Parameters::paramIDs.width.getParamID()};
    WebSliderParameterAttachment widthAttachment {*this->processor.parameters.widthParam, widthRelay, nullptr};

    WebSliderRelay dampingRelay {Parameters::paramIDs.damping.getParamID()};
    WebSliderParameterAttachment dampingAttachment {*this->processor.parameters.dampingParam, dampingRelay, nullptr};

    WebSliderRelay diffusionRelay {Parameters::paramIDs.diffusion.getParamID()};
    WebSliderParameterAttachment diffusionAttachment {*this->processor.parameters.diffusionParam, diffusionRelay, nullptr};

    WebSliderRelay lowcutRelay {Parameters::paramIDs.lowcut.getParamID()};
    WebSliderParameterAttachment lowcutAttachment {*this->processor.parameters.lowcutParam, lowcutRelay, nullptr};

    WebSliderRelay highcutRelay {Parameters::paramIDs.highcut.getParamID()};
    WebSliderParameterAttachment highcutAttachment {*this->processor.parameters.highcutParam, highcutRelay, nullptr};

    WebBrowserComponent webview;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};