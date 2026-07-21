#include "trackerAPI.h"
#include "lighttrack/LightTrack.hpp"
#include "ostrack/OSTrack.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

// Provide body for pure virtual destructor to fix linking error
ITrackIF::~ITrackIF() {}

class LightTrackWrapper : public ITrackIF {
private:
    std::shared_ptr<LightTrack::Tracker> tracker_;

public:
    LightTrackWrapper() {}
    ~LightTrackWrapper() override {}

    bool init_track(int nGpuId) override {
        // GPU id configuration can be handled here if supported by backend
        return true;
    }

    void ex_param_set(const char *param) override {
        // Can be used to parse advanced options
    }

    bool load_model(const char *param) override {
        std::string z_path = "lighttrack-z.trt";
        std::string x_path = "lighttrack-x-head.trt";

        // Optional: Simple parsing of param if format is "z_path,x_path"
        if (param != nullptr && param[0] != '\0') {
            std::string p(param);
            size_t comma = p.find(',');
            if (comma != std::string::npos) {
                z_path = p.substr(0, comma);
                x_path = p.substr(comma + 1);
            } else {
                // If no comma, maybe it's a directory? Just ignore for now
            }
        }

        tracker_ = LightTrack::create_tracker(z_path, x_path);
        return tracker_ != nullptr;
    }

    bool init(cv::Mat &image, cv::Rect &bbox) override {
        if (!tracker_) return false;
        tracker_->init(image, bbox);
        return true;
    }

    float track(cv::Mat &image, cv::Rect &bbox) override {
        if (!tracker_) return 0.0f;
        bbox = tracker_->track(image);
        return 1.0f; // Return 1.0f score as requested
    }

    const char* get_track_name() override {
        return "LightTrack";
    }
};

class OSTrackWrapper : public ITrackIF {
private:
    std::shared_ptr<OSTrack::Tracker> tracker_;

public:
    OSTrackWrapper() {}
    ~OSTrackWrapper() override {}

    bool init_track(int nGpuId) override {
        return true;
    }

    void ex_param_set(const char *param) override {
    }

    bool load_model(const char *param) override {
        std::string engine_path = "ostrack-256.trt";
        if (param != nullptr && param[0] != '\0') {
            engine_path = param;
        }

        tracker_ = OSTrack::create_tracker(engine_path);
        return tracker_ != nullptr;
    }

    bool init(cv::Mat &image, cv::Rect &bbox) override {
        if (!tracker_) return false;
        tracker_->init(image, bbox);
        return true;
    }

    float track(cv::Mat &image, cv::Rect &bbox) override {
        if (!tracker_) return 0.0f;
        bbox = tracker_->track(image);
        return 1.0f; // Return 1.0f score as requested
    }

    const char* get_track_name() override {
        return "OSTrack";
    }
};

// ======================= Export Functions ======================= //

extern "C" {

ITrackIF* create_lighttrack_instance() {
    return new LightTrackWrapper();
}

ITrackIF* create_ostrack_instance() {
    return new OSTrackWrapper();
}

void destroy_tracker_instance(ITrackIF* tracker) {
    if (tracker) {
        delete tracker;
    }
}

}
