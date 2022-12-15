#pragma once

#include <JuceHeader.h>

typedef juce::dsp::IIR::Filter<float> Filter;
typedef juce::dsp::IIR::Coefficients<float> Coefficients;

class BandSplitter
{
public:
	BandSplitter() {}

	void calcCoeffs(double sampleRate, float freq)
	{
		LPF.coefficients = Coefficients::makeLowPass(sampleRate, freq);
		HPF.coefficients = Coefficients::makeHighPass(sampleRate, freq);
	}

	std::array<float, 2> processSample(float sample)
	{
		auto lowSample = LPF.processSample(sample);
		auto highSample = HPF.processSample(sample) * -1.0f;

		return { lowSample, highSample };
	}

private:
	Filter LPF, HPF;
};