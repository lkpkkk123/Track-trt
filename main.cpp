
#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include <opencv2/opencv.hpp>
#include "trackerAPI.h"

using namespace std;


inline static cv::Rect read_gt(std::string &gt_file) {
    std::string line;
    cv::Rect gt_box;
    std::ifstream fin(gt_file);  //真实值文件
    getline(fin, line);
    if (!fin)
        std::cout << " Do not read groundtruth!!! " << std::endl;

    std::stringstream line_ss = std::stringstream(line);
    if (line.find(',') != std::string::npos) {
        std::vector<std::string> data;
        std::string tmp;
        while (getline(line_ss, tmp, ','))
            data.push_back(tmp);
        gt_box.x = stoi(data[0]);
        gt_box.y = stoi(data[1]);
        gt_box.width = stoi(data[2]);
        gt_box.height = stoi(data[3]);
    } else
        line_ss >> gt_box.x >> gt_box.y >> gt_box.width >> gt_box.height;

    return gt_box;
}


void LaunchTrack(ITrackIF* tracker, int Mode, const string& path){

    cv::VideoCapture cap;
    string display_name = "Track";

    if (Mode == 0 || Mode == 1) {
        cv::Mat img;
        cap.open(path);
        if (!cap.isOpened()) {
            cout << "----------Failed to open video/camera!!!----------" << endl;
            return;
        }

        bool is_tracking = false;
        cv::Rect bbox;

        while (true) {
            bool ret = cap.read(img);
            if (!ret) {
                cout << "----------End of video or read failed!!!----------" << endl;
                break;
            }

            if (is_tracking) {
                auto start = std::chrono::steady_clock::now();
                tracker->track(img, bbox);
                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                double time = 1000 * elapsed.count();
                printf("all infer time: %f ms\n", time);
                cv::rectangle(img, bbox, cv::Scalar(0, 255, 0), 2);
            }

            cv::Mat disp_img = img;
            bool need_resize = (img.cols > 1280 || img.rows > 720);
            if (need_resize) {
                cv::resize(img, disp_img, cv::Size(1280, 720));
            }
            cv::imshow(display_name, disp_img);

            int delay = is_tracking ? 1 : 30;
            int key = cv::waitKey(delay);
            if (key == 'r' || key == 'R') {
                cv::Mat select_img = disp_img.clone();
                cv::putText(select_img, "Select target ROI and press ENTER", cv::Point2i(20, 30),
                            cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(0, 0, 255), 1);
                cv::Rect init_bbox = cv::selectROI(display_name, select_img, false);
                if (init_bbox.width > 0 && init_bbox.height > 0) {
                    if (need_resize) {
                        float scale_x = (float)img.cols / 1280.0f;
                        float scale_y = (float)img.rows / 720.0f;
                        cv::Rect mapped_bbox;
                        mapped_bbox.x = std::round(init_bbox.x * scale_x);
                        mapped_bbox.y = std::round(init_bbox.y * scale_y);
                        mapped_bbox.width = std::round(init_bbox.width * scale_x);
                        mapped_bbox.height = std::round(init_bbox.height * scale_y);
                        tracker->init(img, mapped_bbox);
                    } else {
                        tracker->init(img, init_bbox);
                    }
                    is_tracking = true;
                }
            } else if (key == 27) { // ESC
                break;
            }
        }
    }
    else if (Mode == 2) {
        string gt_path = "Woman/groundtruth_rect.txt";
        boost::format fmt(path.data());  //数据集图片
        cv::Mat frame;
        frame = cv::imread((fmt % 1).str(),1);
        cv::Rect init_bbox = read_gt(gt_path);
        tracker->init(frame, init_bbox);

        cv::Mat disp_frame = frame;
        if (frame.cols > 1280 || frame.rows > 720) {
            cv::resize(frame, disp_frame, cv::Size(1280, 720));
        }
        cv::imshow(display_name, disp_frame);

        cv::Rect bbox;
        cv::Mat img;
        for (int i = 1; i < 597; ++i) {
            img = cv::imread((fmt % i).str(),1);
            auto start = std::chrono::steady_clock::now();
            tracker->track(img, bbox);
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            double time = 1000 * elapsed.count();
            printf("all infer time: %f ms\n", time);
            cv::rectangle(img, bbox, cv::Scalar(0,255,0), 2);

            cv::Mat disp_img = img;
            if (img.cols > 1280 || img.rows > 720) {
                cv::resize(img, disp_img, cv::Size(1280, 720));
            }
            cv::imshow(display_name, disp_img);
            cv::waitKey(1);
        }
        return;
    }
    else {
        printf("Mode错误，0：视频文件；1：摄像头；2：数据集");
        return;
    }
}


int main(int argc, char* argv[]){
    if (argc < 3){
        fprintf(stderr, "usage: %s [mode] [path] [tracker_type (optional, default: lighttrack)]. \n"
                        " For video, mode=0, path=/xxx/xxx/*.mp4; \n"
                        " For webcam mode=1, path is cam id; \n"
                        " For image dataset, mode=2, path=Woman/img/%%04d.jpg; \n"
                        " tracker_type: 'lighttrack' (or 'light'), 'ostrack' (or 'os') \n", argv[0]);
        return -1;
    }
    // Mode=0: 视频文件   Mode=1: 摄像头   Mode=2: 数据集
    int Mode = atoi(argv[1]);
    string path = argv[2];

    string tracker_type = "lighttrack";
    if (argc >= 4) {
        tracker_type = argv[3];
        for (auto &c : tracker_type) c = tolower(c);
    }

    ITrackIF* tracker = nullptr;

    if (tracker_type == "lighttrack" || tracker_type == "light") {
        tracker = create_lighttrack_instance();
        if (tracker) tracker->load_model("lighttrack-z.trt,lighttrack-x-head.trt");
    } else if (tracker_type == "ostrack" || tracker_type == "os") {
        tracker = create_ostrack_instance();
        if (tracker) tracker->load_model("ostrack-256.trt");
    } else {
        fprintf(stderr, "Unknown tracker type: %s. Use 'lighttrack' or 'ostrack'.\n", tracker_type.c_str());
        return -1;
    }

    if (tracker == nullptr) {
        printf("Tracker creation failed.\n");
        return -1;
    }

    printf("Using %s algorithm via ITrackIF interface...\n", tracker_type.c_str());
    LaunchTrack(tracker, Mode, path);

    destroy_tracker_instance(tracker);
    return 0;
}
