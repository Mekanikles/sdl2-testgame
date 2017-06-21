#include "Profiler.h"

#include "ColorDefines.h"

namespace jcpe
{

namespace Profiler
{

const CategoryInfo kProfilerCategoryRoot = CategoryInfo { "Root", Color::kBarbiePink };
const CategoryInfo kProfilerCategoryProfiler = CategoryInfo { "Profiler", Color::kWillpowerOrange };
const CategoryInfo kProfilerCategoryIdle = CategoryInfo { "Idle", Color::kAshGrey };
const CategoryInfo kProfilerCategoryUncategorized = CategoryInfo { "Uncategorized", Color::kBDazzledBlue };

static Profiler* s_profiler = nullptr;

void setProfiler(not_null<Profiler*> profiler)
{
	s_profiler = std::move(profiler);
}

not_null<Profiler*> getProfiler()
{
	return s_profiler;
}

// TODO: Add namespace protection or make members of class
struct FrameData
{
	vector<Sample> samples;
};

struct Profiler::State
{
	vector<FrameData> frames;

	struct SampleStackInfo
	{
		uint index;
		uint childCount;
	};
	vector<SampleStackInfo> sampleStack;


};

unique_ptr<Profiler> Profiler::createProfiler()
{
	void* const memAddr = malloc(sizeof(Profiler) + sizeof(Profiler::State));
	Profiler* prof = new (memAddr) Profiler((uint8*)memAddr + sizeof(Profiler));
	return unique_ptr<Profiler>(prof);
}

Profiler::Profiler(void* stateMemAddr)
	: m_state(new (stateMemAddr) Profiler::State())
{
}

Profiler::~Profiler()
{
    m_state.release();
     
}

void Profiler::beginFrame()
{
	m_state->frames.push_back(FrameData());
	m_state->sampleStack.clear();

	// Root sample, encapsulates whole frame
	FrameData* currentFrame = &m_state->frames.back();
	currentFrame->samples.push_back(Sample());
	m_state->sampleStack.push_back(Profiler::State::SampleStackInfo{0, 0});
	Sample& sample = currentFrame->samples.back();

	static SampleInfo rootInfo{"Root", &kProfilerCategoryRoot};
	sample.info = &rootInfo;
	sample.startTime = getTime();
}

void Profiler::endFrame()
{
	endSample(getTime());
	ASSERT_DESC(m_state->sampleStack.size() == 0, "Unmatched sample begin/end at end of frame");
}

not_null<Sample*> Profiler::beginSampleWithoutStartTime(not_null<const SampleInfo*> info)
{
	FrameData* currentFrame = &m_state->frames.back();
	FATAL_ASSERT_DESC(currentFrame, "No active profiler frame, make sure begin/endFrame is being called");
	currentFrame->samples.push_back(Sample());

	auto& sampleStack = m_state->sampleStack;

	// Increase parent sample child count
	sampleStack.back().childCount++;

	// TODO: Investigate push_back() uninit
	sampleStack.push_back(Profiler::State::SampleStackInfo{(uint)currentFrame->samples.size() - 1, 0});
	Sample& sample = currentFrame->samples.back();

	sample.info = info;
	return &sample;
}

void Profiler::endSample(const TimeStamp& endTime)
{
	FrameData& currentFrame = m_state->frames.back();
	auto& sampleStack = m_state->sampleStack;
	Sample& sample = currentFrame.samples[sampleStack.back().index];

	sample.duration = endTime - sample.startTime;
	sample.childCount = sampleStack.back().childCount;

	sampleStack.pop_back();	
}

} // namespace Profiler

}

