#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include <gamemachine.h>
using namespace gm;

class Timeline
{
public:
	Timeline(GMGameWorld* world);

public:
	// 解析一个Timeline文件
	void parse(const GMString& timelineContent);
	void play();
	void pause();

private:
	GMGameWorld* m_world;
};

#endif