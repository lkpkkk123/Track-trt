#pragma once
#include <opencv2/core/core.hpp>
struct ITrackIF
{
    //param 参数为一个字符串，通常格式为: param1:sub_parm1,sub_parm2,sub_parm3...;param2:sub_parm1,sub_parm2,sub_parm3...;...
    //也可以是一个json,接口实现者和调用者自行约定参数具体含义。接口实现内部解析出参数使用，以实现接口通用与灵活性
    virtual bool init_track(int nGpuId) = 0;
    virtual void ex_param_set(const char *param) = 0;
    virtual bool load_model(const char *param) = 0;
    virtual bool init(cv::Mat &image, cv::Rect &bbox) = 0;
    virtual float track(cv::Mat &image, cv::Rect &bbox) = 0;
    virtual const char* get_track_name() = 0;
    virtual ~ITrackIF() = 0;
};
