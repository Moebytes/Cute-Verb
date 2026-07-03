#pragma once
#include <JuceHeader.h>
#include "DelayLine.hpp"
#include "CombFilter.hpp"
#include "AllpassFilter.hpp"
#include "BiquadFilter.hpp"

class Reverb {
public:
    Reverb() = default;
    virtual ~Reverb() = default;

    auto prepareToPlay(double sampleRate) -> void {
        this->sampleRate = sampleRate;

        float sampleRateFactor = static_cast<float>(sampleRate) / 44100.0f;

        int maxCombDelay = static_cast<int>(std::ceil((1640 + 400) * sampleRateFactor));
        int maxAllpassDelay = static_cast<int>(std::ceil((579 + 150) * sampleRateFactor));

        for (auto& combFilter : combFiltersL) {
            combFilter.prepareToPlay(maxCombDelay);
        }

        for (auto& combFilter : combFiltersR) {
            combFilter.prepareToPlay(maxCombDelay);
        }

        for (auto& allpassFilter : allpassFiltersL) {
            allpassFilter.prepareToPlay(maxAllpassDelay);
        }

        for (auto& allpassFilter : allpassFiltersR) {
            allpassFilter.prepareToPlay(maxAllpassDelay);
        }

        float maxPredelayMs = 500.0f;
        int maxPredelaySamples = static_cast<int>(std::ceil(maxPredelayMs * 0.001f * sampleRate));
        this->predelayLine.prepareToPlay(maxPredelaySamples);

        this->highpassFilterL.prepareToPlay(sampleRate);
        this->highpassFilterR.prepareToPlay(sampleRate);
        this->lowpassFilterL.prepareToPlay(sampleRate);
        this->lowpassFilterR.prepareToPlay(sampleRate);

        this->highpassFilterL.setType("highpass");
        this->highpassFilterR.setType("highpass");
        this->lowpassFilterL.setType("lowpass");
        this->lowpassFilterR.setType("lowpass");

        this->reset();
        this->updateCombFilters();
        this->updateFilters();
    }

    auto reset() -> void {
        for (auto& combFilter : combFiltersL) combFilter.reset();
        for (auto& combFilter : combFiltersR) combFilter.reset();
        for (auto& allpassFilter : allpassFiltersL) allpassFilter.reset();
        for (auto& allpassFilter : allpassFiltersR) allpassFilter.reset();
        this->predelayLine.reset();

        this->highpassFilterL.reset();
        this->highpassFilterR.reset();
        this->lowpassFilterL.reset();
        this->lowpassFilterR.reset();
    }

    auto setDecay(float decay) -> void {
        if (approximatelyEqual(this->decay, decay)) return;
        this->decay = std::max(decay, 0.05f);
        this->updateCombFilters();
    }

    auto setPredelay(float predelay) -> void {
        this->predelay = predelay;

        int samples = static_cast<int>(std::round(this->predelay * 0.001f * this->sampleRate));
        this->predelayLine.setDelaySamples(samples);
    }

    auto setSize(float size) -> void {
        if (approximatelyEqual(this->size, size)) return;
        this->size = size;
        this->updateCombFilters();
    }

    auto setWidth(float width) -> void {
        this->width = width;
    }

    auto setDamping(float damping) -> void {
        if (approximatelyEqual(this->damping, damping)) return;
        this->damping = jmap(damping, 0.2f, 0.85f);
        this->updateCombFilters();
    }

    auto setDiffusion(float diffusion) -> void {
        if (approximatelyEqual(this->diffusion, diffusion)) return;
        this->diffusion = jmap(diffusion, 0.2f, 0.85f);
        this->updateCombFilters();
    }

    auto setHighcutFreq(float highcutFreq) -> void {
        if (approximatelyEqual(this->highcutFreq, highcutFreq)) return;
        this->highcutFreq = highcutFreq;
        this->updateFilters();
    }

    auto setLowcutFreq(float lowcutFreq) -> void {
        if (approximatelyEqual(this->lowcutFreq, lowcutFreq)) return;
        this->lowcutFreq = lowcutFreq;
        this->updateFilters();
    }

    auto updateCombFilters() -> void {
        float sampleRateFactor = static_cast<float>(sampleRate) / 44100.0f;
        float scale = (0.5f + this->size) * sampleRateFactor;

        this->combFiltersL[0].setDelaySamples(1116 * scale);
        this->combFiltersL[1].setDelaySamples(1188 * scale);
        this->combFiltersL[2].setDelaySamples(1277 * scale);
        this->combFiltersL[3].setDelaySamples(1356 * scale);
        this->combFiltersL[4].setDelaySamples(1422 * scale);
        this->combFiltersL[5].setDelaySamples(1491 * scale);
        this->combFiltersL[6].setDelaySamples(1557 * scale);
        this->combFiltersL[7].setDelaySamples(1617 * scale);

        this->combFiltersR[0].setDelaySamples(1139 * scale);
        this->combFiltersR[1].setDelaySamples(1211 * scale);
        this->combFiltersR[2].setDelaySamples(1300 * scale);
        this->combFiltersR[3].setDelaySamples(1379 * scale);
        this->combFiltersR[4].setDelaySamples(1445 * scale);
        this->combFiltersR[5].setDelaySamples(1514 * scale);
        this->combFiltersR[6].setDelaySamples(1580 * scale);
        this->combFiltersR[7].setDelaySamples(1640 * scale);

        this->allpassFiltersL[0].setDelaySamples(556 * scale);
        this->allpassFiltersL[1].setDelaySamples(441 * scale);
        this->allpassFiltersL[2].setDelaySamples(341 * scale);
        this->allpassFiltersL[3].setDelaySamples(225 * scale);

        this->allpassFiltersR[0].setDelaySamples(579 * scale);
        this->allpassFiltersR[1].setDelaySamples(464 * scale);
        this->allpassFiltersR[2].setDelaySamples(364 * scale);
        this->allpassFiltersR[3].setDelaySamples(248 * scale);

        for (auto& combFilter : combFiltersL) {
            float feedback = std::exp((-6.907755f * combFilter.getDelaySamples()) / 
                (this->decay * static_cast<float>(this->sampleRate)));

            combFilter.setFeedback(feedback);
        }

        for (auto& combFilter : combFiltersR) {
            float feedback = std::exp((-6.907755f * combFilter.getDelaySamples()) / 
                (this->decay * static_cast<float>(this->sampleRate)));

            combFilter.setFeedback(feedback);
        }

        for (auto& combFilter : combFiltersL) {
            combFilter.setDamping(this->damping);
        }

        for (auto& combFilter : combFiltersR) {
            combFilter.setDamping(this->damping);
        }

        for (auto& allpassFilter : allpassFiltersL) {
            allpassFilter.setFeedback(this->diffusion);
        }

        for (auto& allpassFilter : allpassFiltersR) {
            allpassFilter.setFeedback(this->diffusion);
        }
    }

    auto updateFilters() -> void {
        float q = 0.707f;

        this->highpassFilterL.setFrequency(this->lowcutFreq);
        this->highpassFilterR.setFrequency(this->lowcutFreq);
        this->lowpassFilterL.setFrequency(this->highcutFreq);
        this->lowpassFilterR.setFrequency(this->highcutFreq);

        this->highpassFilterL.setResonance(q);
        this->highpassFilterR.setResonance(q);
        this->lowpassFilterL.setResonance(q);
        this->lowpassFilterR.setResonance(q);
    }

    auto process(float dryL, float dryR, float& wetL, float& wetR) -> void {
        float mono = 0.5f * (dryL + dryR);
        float input = this->predelayLine.process(mono);

        float left = 0.0f;
        float right = 0.0f;

        for (auto& combFilter : combFiltersL) {
            left += combFilter.process(input);
        }

        for (auto& combFilter : combFiltersR) {
            right += combFilter.process(input);
        }

        left /= static_cast<float>(combFiltersL.size());
        right /= static_cast<float>(combFiltersR.size());

        for (auto& allpassFilter : allpassFiltersL) {
            left = allpassFilter.process(left);
        }

        for (auto& allpassFilter : allpassFiltersR) {
            right = allpassFilter.process(right);
        }

        left = highpassFilterL.processSample(left);
        left = lowpassFilterL.processSample(left);
        right = highpassFilterR.processSample(right);
        right = lowpassFilterR.processSample(right);

        float mid  = 0.5f * (left + right);
        float side = 0.5f * (left - right) * width;

        wetL = mid + side;
        wetR = mid - side;
    }

private:
    double sampleRate = 44100.0;

    float decay = 5.0f;
    float predelay = 20.0f;
    float size = 0.5f;
    float width = 1.0f;
    float damping = 0.3f;
    float diffusion = 0.7f;
    float lowcutFreq = 200.0f;
    float highcutFreq = 20000.0f;

    DelayLine predelayLine;

    std::array<CombFilter, 8> combFiltersL;
    std::array<CombFilter, 8> combFiltersR;

    std::array<AllpassFilter, 4> allpassFiltersL;
    std::array<AllpassFilter, 4> allpassFiltersR;

    BiquadFilter lowpassFilterL;
    BiquadFilter lowpassFilterR;

    BiquadFilter highpassFilterL;
    BiquadFilter highpassFilterR;
};