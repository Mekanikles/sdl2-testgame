#include "ProfilerTimeline.h"

#include "IMGui.h"

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
	gui->filledRect(Rect2(Point2(0, 0), vec2(100, 100)), Color32(0,0,0,0.5f));
}

}
