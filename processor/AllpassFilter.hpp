#pragma once
#include <JuceHeader.h>

class AllpassFilter {
public:
    AllpassFilter() = default;
    ~AllpassFilter() = default;

    auto prepareToPlay(int maximumDelaySamples) -> void {
        this->buffer.setSize(1, maximumDelaySamples, false, false, true);
        this->delaySamples = maximumDelaySamples;
        this->reset();
    }

    auto reset() -> void {
        this->buffer.clear();
        this->writePos = 0;
    }

    auto setFeedback(float feedback) -> void {
        this->feedback = jlimit(0.0f, 0.99f, feedback);
    }

    auto setDelaySamples(float delaySamples) -> void {
        this->delaySamples = jlimit(1, this->buffer.getNumSamples() - 1, static_cast<int>(delaySamples));
    }

    auto getDelaySamples() -> float {
        return static_cast<float>(this->delaySamples);
    }

    auto process(float input) noexcept -> float {
        if (this->buffer.getNumSamples() == 0) return input;

        auto* data = buffer.getWritePointer(0);

        int readPos = writePos - this->delaySamples;
        if (readPos < 0) readPos += this->buffer.getNumSamples();

        float delayed = data[readPos];

        float output = delayed - this->feedback * input;

        data[writePos] = input + this->feedback * output;

        if (++this->writePos >= this->buffer.getNumSamples()) {
            this->writePos = 0;
        }

        return output;
    }


private:
    float feedback = 0.5f;

    AudioBuffer<float> buffer;
    int writePos = 0;
    int delaySamples = 0;
};