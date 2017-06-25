#include "ProfilerTimeline.h"

#include "Profiler.h"
#include "IMGui.h"
#include "ColorDefines.h"

namespace jcpe
{

struct ProfilerTimeline::State
{
	int lol;
};


unique_ptr<ProfilerTimeline> ProfilerTimeline::create()
{
	void* const baseAddr = malloc(sizeof(ProfilerTimeline) + sizeof(State));
	void* const stateAddr = (void*)((uint8*)baseAddr + sizeof(ProfilerTimeline));

	auto* state = new (stateAddr) State();
	auto* obj = new (baseAddr) ProfilerTimeline(state);
	return unique_ptr<ProfilerTimeline>(obj);
}

ProfilerTimeline::ProfilerTimeline(State* state)
	: m_state(state)
{
}

ProfilerTimeline::~ProfilerTimeline()
{
}

void ProfilerTimeline::draw(not_null<IMGui*> gui)
{
	PROFILER_SCOPE("ProfilerTimelineDraw", &Profiler::kProfilerCategoryProfiler);

	const vec2 size(800, 600);

	gui->filledRect(Rect2(0, 0, size), Color32(0,0,0,0.5f));

	const auto profiler = Profiler::getProfiler();
	const auto frameData = profiler->getLastFrameData();

	if (!frameData)
		return;

	ASSERT(frameData->samples.size() > 0);

	const auto frameStart = frameData->samples[0].startTime;
	const auto frameLength = frameData->samples[0].duration;
	const float invFrameLength = 1.0f/(float)frameLength.count();

	vector<int> childCountStack;
	childCountStack.push_back(1);
	for (const auto& sample : frameData->samples)
	{
		const int depth = childCountStack.size() - 1;
		const auto relStart = sample.startTime - frameStart;
		const Point2 sPos = Point2((relStart.count() * invFrameLength) * size.x, depth * 16);
		const vec2 sSize = vec2((sample.duration.count() * invFrameLength) * size.x, 16);
		Color32 color = sample.info->category->color;
		gui->filledRect(Rect2(sPos, sSize), color);
		gui->text(Rect2(sPos, sSize), sample.info->name, Color32(0,0,0,1));

		if (sample.childCount > 0)
		{
			childCountStack.push_back(sample.childCount);
		}
		else
		{
			while (childCountStack.size() > 0 && --childCountStack.back() == 0)
			{
				childCountStack.pop_back();
			}
		}
	}
	ASSERT(childCountStack.size() == 0);
}

}
