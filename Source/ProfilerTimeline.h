#include "Core.h"

namespace jcpe
{

class IMGui;

class ProfilerTimeline
{
public:
	static unique_ptr<ProfilerTimeline> create();
	~ProfilerTimeline();

	void draw(not_null<IMGui*> gui);

private:
	struct State;	
	ProfilerTimeline(State* state);

	State* m_state;
};


}
