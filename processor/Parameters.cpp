#include "Parameters.h"
#include "Functions.hpp"

template<typename T>
static auto castParameter(const AudioProcessorValueTreeState& tree, 
    const ParameterID* id, T*& dest) -> void {
    dest = dynamic_cast<T*>(tree.getParameter(id->getParamID()));
    jassert(dest != nullptr);
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterFloat* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = paramObj->getDefaultValue();
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterBool* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = static_cast<bool>(paramObj->getDefaultValue());
}

template <typename T>
static auto resetParameter(const AudioProcessorValueTreeState& tree, 
    const AudioParameterChoice* param, T*& dest) -> void {
    auto* paramObj = tree.getParameter(param->getParameterID());
    if (paramObj) *dest = static_cast<T>(paramObj->getDefaultValue());
}

ParameterIDs Parameters::paramIDs = ParameterIDs::loadFromJSON();

Parameters::Parameters(AudioProcessorValueTreeState& tree) : tree(tree) {
    using FloatPair = std::pair<AudioParameterFloat*&, const ParameterID*>;

    auto floatParameters = std::vector<FloatPair>{
        {mixParam, &paramIDs.mix},
        {decayParam, &paramIDs.decay},
        {predelayParam, &paramIDs.predelay},
        {sizeParam, &paramIDs.size},
        {widthParam, &paramIDs.width},
        {dampingParam, &paramIDs.damping},
        {diffusionParam, &paramIDs.diffusion},
        {lowcutParam, &paramIDs.lowcut},
        {highcutParam, &paramIDs.highcut}
    };

    for (const auto& [param, paramID] : floatParameters) {
        castParameter(tree, paramID, param);
    }
}

auto Parameters::createParameterLayout() -> AudioProcessorValueTreeState::ParameterLayout {
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.mix, "Mix", NormalisableRange<float>{0.0f, 1.0f, 0.01f}, 0.5f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayPercent)
        .withValueFromStringFunction(Functions::parsePercent)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.decay, "Decay", NormalisableRange<float>{0.1f, 10.0f, 0.01f}, 5.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displaySeconds)
        .withValueFromStringFunction(Functions::parseSeconds)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.predelay, "Pre-Delay", NormalisableRange<float>{0.0f, 500.0f, 1.0f}, 20.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayMilliseconds)
        .withValueFromStringFunction(Functions::parseMilliseconds)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.size, "Size", NormalisableRange<float>{0.0f, 1.0f, 0.01f}, 0.5f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayPercent)
        .withValueFromStringFunction(Functions::parsePercent)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.width, "Width", NormalisableRange<float>{0.0f, 2.0f, 0.01f}, 1.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayPercent)
        .withValueFromStringFunction(Functions::parsePercent)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.damping, "Damping", NormalisableRange<float>{0.0f, 1.0f, 0.01f}, 0.3f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayPercent)
        .withValueFromStringFunction(Functions::parsePercent)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.diffusion, "Diffusion", NormalisableRange<float>{0.0f, 1.0f, 0.01f}, 0.7f,
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayPercent)
        .withValueFromStringFunction(Functions::parsePercent)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.lowcut, "Lowcut Freq", NormalisableRange<float>{200.0f, 20000.0f, 1.0f, 0.3f}, 200.0f, 
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayHz)
        .withValueFromStringFunction(Functions::parseHz)
    ));

    layout.add(std::make_unique<AudioParameterFloat>(
        paramIDs.highcut, "Highcut Freq", NormalisableRange<float>{200.0f, 20000.0f, 1.0f, 0.3f}, 20000.0f, 
        AudioParameterFloatAttributes().withStringFromValueFunction(Functions::displayHz)
        .withValueFromStringFunction(Functions::parseHz)
    ));

    return layout;
}

auto Parameters::getDefaultParameter(const Array<var>& args,
    WebBrowserComponent::NativeFunctionCompletion completion) -> void {

    auto paramID = args[0].toString();
    auto* param = this->tree.getParameter(paramID);
    float defaultValue = param->convertFrom0to1(param->getDefaultValue());

    completion(defaultValue);
}

auto Parameters::prepareToPlay(double sampleRate, int blockSize) noexcept -> void {
    this->sampleRate = sampleRate;
    this->blockSize = blockSize;

    double duration = 0.001;

    auto smoothers = std::vector{
        &mixSmoother,
        &decaySmoother,
        &predelaySmoother,
        &sizeSmoother,
        &widthSmoother,
        &dampingSmoother,
        &diffusionSmoother,
        &lowcutSmoother,
        &highcutSmoother
    };

    for (const auto& smoother : smoothers) {
        smoother->reset(this->sampleRate, duration);
    }
}

auto Parameters::reset() noexcept -> void {
    auto paramFloats = std::vector{
        std::pair{mixParam, &mix},
        std::pair{decayParam, &decay},
        std::pair{predelayParam, &predelay},
        std::pair{sizeParam, &size},
        std::pair{widthParam, &width},
        std::pair{dampingParam, &damping},
        std::pair{diffusionParam, &diffusion},
        std::pair{lowcutParam, &lowcut},
        std::pair{highcutParam, &highcut}
    };

    for (auto& [param, value] : paramFloats) {
        resetParameter(tree, param, value);
    }
    
    auto smoothers = std::vector{
        std::pair{mixParam, &mixSmoother},
        std::pair{decayParam, &decaySmoother},
        std::pair{predelayParam, &predelaySmoother},
        std::pair{sizeParam, &sizeSmoother},
        std::pair{widthParam, &widthSmoother},
        std::pair{dampingParam, &dampingSmoother},
        std::pair{diffusionParam, &diffusionSmoother},
        std::pair{lowcutParam, &lowcutSmoother},
        std::pair{highcutParam, &highcutSmoother}
    };

    for (const auto& [param, smoother] : smoothers) {
        smoother->setCurrentAndTargetValue(param->get());
    }
}

auto Parameters::setHostInfo(double bpm, double ppq, const AudioPlayHead::TimeSignature& timeSignature) noexcept -> void {
    this->bpm = bpm;
    this->ppq = ppq;
    this->timeSignature = timeSignature;

    if (ppq > 0.0) {
        this->ppq = ppq;
        this->internalPPQ = ppq;
    } else {
        double ppqPerSample = (this->bpm / 60.0) / this->sampleRate;
        this->internalPPQ += ppqPerSample * this->blockSize; 
        this->ppq = this->internalPPQ;
    }
}

auto Parameters::blockUpdate() noexcept -> void {
    auto smoothers = std::vector{
        std::pair{mixParam, &mixSmoother},
        std::pair{decayParam, &decaySmoother},
        std::pair{predelayParam, &predelaySmoother},
        std::pair{sizeParam, &sizeSmoother},
        std::pair{widthParam, &widthSmoother},
        std::pair{dampingParam, &dampingSmoother},
        std::pair{diffusionParam, &diffusionSmoother},
        std::pair{lowcutParam, &lowcutSmoother},
        std::pair{highcutParam, &highcutSmoother}
    };

    for (const auto& [param, smoother] : smoothers) {
        smoother->setTargetValue(param->get());
    }
}

auto Parameters::update() noexcept -> void {
    this->mix = this->mixSmoother.getNextValue();

    this->decay = this->decaySmoother.getNextValue();
    this->predelay = this->predelaySmoother.getNextValue();

    this->size = this->sizeSmoother.getNextValue();
    this->width = this->widthSmoother.getNextValue();

    this->damping = this->dampingSmoother.getNextValue();
    this->diffusion = this->diffusionSmoother.getNextValue();

    this->lowcut = this->lowcutSmoother.getNextValue();
    this->highcut = this->highcutSmoother.getNextValue();
}