#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include <gamemachine.h>
using namespace gm;

class Timeline
{
public:
	// 解析一个Timeline文件
	void parse(const GMString& timelineContent);
	void loadAssets();
	void update(GMDuration dt);
	void play();
	void pause();
};

#endif