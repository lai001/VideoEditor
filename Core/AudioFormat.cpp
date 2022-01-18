#include "AudioFormat.h"
#include "ThirdParty/spdlog.h"
#include "Utility/FUtility.h"


bool FAudioFormat::operator==(const FAudioFormat & format) const
{
	if (sampleRate != format.sampleRate)
	{
		return false;
	}
	if (formatType != format.formatType)
	{
		return false;
	}
	if (formatFlags != format.formatFlags)
	{
		return false;
	}
	if (bytesPerPacket != format.bytesPerPacket)
	{
		return false;
	}
	if (framesPerPacket != format.framesPerPacket)
	{
		return false;
	}
	if (bytesPerFrame != format.bytesPerFrame)
	{
		return false;
	}
	if (channelsPerFrame != format.channelsPerFrame)
	{
		return false;
	}
	if (bitsPerChannel != format.bitsPerChannel)
	{
		return false;
	}
	return true;
}

AudioSampleType FAudioFormat::getSampleType() const
{
	assert(bitsPerChannel == 16 || bitsPerChannel == 32 || bitsPerChannel == 64);
	if (isFloat())
	{
		return bitsPerChannel == 32 ? AudioSampleType::float32 : AudioSampleType::float64;
	}
	if (isSignedInteger())
	{
		return bitsPerChannel == 16 ? AudioSampleType::sint16 : AudioSampleType::sint32;
	}
	return bitsPerChannel == 16 ? AudioSampleType::uint16 : AudioSampleType::uint32;
}

std::string FAudioFormat::debugDescription() const
{
	std::string sampleType = AudioSampleTypeToString(getSampleType());

	return fmt::format("sampleRate = {}, sampleType = {}, isNonInterleaved = {}, channels = {}",
		sampleRate,
		sampleType,
		(isNonInterleaved() ? "true" : "false"),
		channelsPerFrame);
}

bool FAudioFormat::isNonInterleaved() const
{
	return Bitmask::isContains(formatFlags, AudioFormatFlag::isNonInterleaved);
}

bool FAudioFormat::isFloat() const
{
	return Bitmask::isContains(formatFlags, AudioFormatFlag::isFloat);
}

bool FAudioFormat::isSignedInteger() const
{
	return Bitmask::isContains(formatFlags, AudioFormatFlag::isSignedInteger);
}
