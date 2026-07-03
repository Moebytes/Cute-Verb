#pragma once
#include <JuceHeader.h>

class DelayLine {
public:
    DelayLine() = default;
    ~DelayLine() = default;

    auto prepareToPlay(int maximumDelaySamples) -> void {
        this->buffer.setSize(1, maximumDelaySamples);
        this->buffer.clear();
        this->writePos = 0;
    }

    auto reset() -> void {
        this->buffer.clear();
        this->writePos = 0;
    }

    auto setDelaySamples(int delaySamples) -> void {
        this->delaySamples = jlimit(1, this->buffer.getNumSamples() - 1, delaySamples);
    }

    auto process(float input) noexcept -> float {
        auto* data = buffer.getWritePointer(0);

        int readPos = this->writePos - this->delaySamples;
        if (readPos < 0) readPos += this->buffer.getNumSamples();

        float output = data[readPos];

        data[this->writePos] = input;

        if (++this->writePos >= this->buffer.getNumSamples()) {
            this->writePos = 0;
        }

        return output;
    }


private:
    AudioBuffer<float> buffer;
    int writePos = 0;
    int delaySamples = 1;
};