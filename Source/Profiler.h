#include "Core.h"
#include <chrono>

namespace jcpe
{
namespace Profiler
{

using Duration = std::chrono::high_resolution_clock::duration;
using TimeStamp = std::chrono::time_point<std::chrono::high_resolution_clock>;

struct CategoryInfo
{
	string name;
	Color32 color;
};

struct SampleInfo
{
	string name;
	not_null<const CategoryInfo*> category;
};

struct Sample
{
	TimeStamp startTime;
	Duration duration;
	int childCount;
	not_null<const SampleInfo*> info;

	Sample(not_null<const SampleInfo*> info) : info(info) {}
};

struct FrameData
{
	// Sample tree stored in a Preorder traversal
	//	First sample is root sampe, encompassing whole frame
	vector<Sample> samples;
};

class Profiler
{
public:
	static unique_ptr<Profiler> createProfiler();

	~Profiler();

	void beginFrame();
	void endFrame();

	// TODO: shared_ptr profiler info?
	not_null<Sample*> beginSampleWithoutStartTime(not_null<const SampleInfo*> info);
	void endSample(const TimeStamp& endTime);

	const FrameData* getLastFrameData();

private:
	Profiler(void* stateMemAddr);

private:
	struct State;
	unique_ptr<State> m_state;
};

void setProfiler(not_null<Profiler*> profiler);
not_null<Profiler*> getProfiler();

extern const CategoryInfo kProfilerCategoryProfiler;
extern const CategoryInfo kProfilerCategoryIdle;
extern const CategoryInfo kProfilerCategoryUncategorized;

inline TimeStamp getTime()
{
	return std::chrono::high_resolution_clock::now();
}

} // namespace profiler

#define PROFILER_SCOPE_INFO(infoPtr)															   \
	auto p = Profiler::getProfiler(); 															   \
	auto sample = p->beginSampleWithoutStartTime(infoPtr);										   \
	sample->startTime = Profiler::getTime();													   \
	SCOPE_EXIT( const auto t2 = Profiler::getTime(); p->endSample(t2); )

#define PROFLIER_SCOPE_NAME_CATEGORY_INTERNAL(infoName, name, categoryPtr)						   \
	static const Profiler::SampleInfo info{ name, categoryPtr };								   \
	PROFILER_SCOPE_INFO(&info)

#define PROFLIER_SCOPE_NAME_CATEGORY(name, categoryPtr) 										   \
	PROFLIER_SCOPE_NAME_CATEGORY_INTERNAL(UNIQUE_SYMBOL(_profiledScope_), name, categoryPtr)

#define PROFILER_SCOPE_NAME(name)																   \
	PROFLIER_SCOPE_NAME_CATEGORY(name, &Profiler::kProfilerCategoryUncategorized)

// Macro trickety, choose function name based on arg count
#define GET_3TH_ARG(arg1, arg2, arg3, ...) arg3

#define PROFILER_SCOPE_MACRO_CHOOSER(...) 														   \
    GET_3TH_ARG(__VA_ARGS__, PROFLIER_SCOPE_NAME_CATEGORY, PROFILER_SCOPE_NAME)

#define PROFILER_SCOPE(...)																		   \
    PROFILER_SCOPE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

}

