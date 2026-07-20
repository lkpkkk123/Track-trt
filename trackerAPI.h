#pragma once

#include "trackerIF.h"

#ifdef __cplusplus
extern "C" {
#endif

// 创建 LightTrack 跟踪器实例
ITrackIF* create_lighttrack_instance();

// 创建 OSTrack 跟踪器实例
ITrackIF* create_ostrack_instance();

// 销毁跟踪器实例
void destroy_tracker_instance(ITrackIF* tracker);

#ifdef __cplusplus
}
#endif
