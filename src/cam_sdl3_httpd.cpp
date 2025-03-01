// main.cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "SDL3/SDL.h"
#include "httplib.h"  // 请确保 cpp-httplib 的头文件路径正确
#include "cam_cls.h"

// 参数定义
#define DEVICE_NAME "/dev/video0"
#define WIDTH 640
#define HEIGHT 480
#define BUFFER_COUNT 4
#define HTTP_PORT 8888

char deviceName[256];

httplib::Server* g_http_server = nullptr;

// 全局运行状态控制（原子变量便于跨线程控制退出）
std::atomic<bool> running{true};

// v4l2 缓冲区数据结构
struct Buffer {
    void *start;
    size_t length;
};
int video_fd = -1;
Buffer* buffers = nullptr;
unsigned int n_buffers = 0;

// 全局最新 RGB 图像数据与互斥保护（RGB24格式，尺寸： WIDTH * HEIGHT * 3）
uint8_t *latest_rgb = nullptr;
std::mutex frame_mutex;
std::condition_variable frame_cond;

// 控制参数（示例：brightness），及其互斥锁
int g_brightness = 128;  // 默认值
std::mutex control_mutex;

// 错误处理函数
void errno_exit(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

// YUYV 转 RGB24（每 2 像素转换）
void yuyv_to_rgb(const uint8_t *yuyv, uint8_t *rgb, int width, int height)
{
    int frame_size = width * height * 2; // YUYV 每个像素 2 字节
    int i, j;
    for(i = 0, j = 0; i < frame_size; i += 4, j += 6)
    {
        uint8_t y0 = yuyv[i];
        uint8_t u  = yuyv[i + 1];
        uint8_t y1 = yuyv[i + 2];
        uint8_t v  = yuyv[i + 3];

        int c = y0 - 16;
        int d = u - 128;
        int e = v - 128;
        int r = (298 * c           + 409 * e + 128) >> 8;
        int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
        int b = (298 * c + 516 * d           + 128) >> 8;
        rgb[j]   = r > 255 ? 255 : (r < 0 ? 0 : r);
        rgb[j+1] = g > 255 ? 255 : (g < 0 ? 0 : g);
        rgb[j+2] = b > 255 ? 255 : (b < 0 ? 0 : b);

        c = y1 - 16;
        r = (298 * c           + 409 * e + 128) >> 8;
        g = (298 * c - 100 * d - 208 * e + 128) >> 8;
        b = (298 * c + 516 * d           + 128) >> 8;
        rgb[j+3] = r > 255 ? 255 : (r < 0 ? 0 : r);
        rgb[j+4] = g > 255 ? 255 : (g < 0 ? 0 : g);
        rgb[j+5] = b > 255 ? 255 : (b < 0 ? 0 : b);
    }
}

// 初始化 V4L2 设备
void init_device()
{
    struct v4l2_capability cap;
    if(ioctl(video_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        errno_exit("VIDIOC_QUERYCAP");
    }
    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        std::cerr << "Device does not support video capture\n";
        exit(EXIT_FAILURE);
    }
    if(!(cap.capabilities & V4L2_CAP_STREAMING)) {
        std::cerr << "Device does not support streaming I/O\n";
        exit(EXIT_FAILURE);
    }
    // 设置采集格式为 YUYV
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = WIDTH;
    fmt.fmt.pix.height      = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    if(ioctl(video_fd, VIDIOC_S_FMT, &fmt) < 0) {
        errno_exit("VIDIOC_S_FMT");
    }
    // 可在此处设置控制参数（例如 brightness）—此处先不做处理
}

// 初始化 mmap 缓冲区
void init_mmap()
{
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count  = BUFFER_COUNT;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if(ioctl(video_fd, VIDIOC_REQBUFS, &req) < 0) {
        errno_exit("VIDIOC_REQBUFS");
    }
    if(req.count < 2) {
        std::cerr << "Insufficient buffer memory\n";
        exit(EXIT_FAILURE);
    }
    buffers = new Buffer[req.count];
    n_buffers = req.count;
    for (unsigned int i = 0; i < n_buffers; i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;
        if(ioctl(video_fd, VIDIOC_QUERYBUF, &buf) < 0) {
            errno_exit("VIDIOC_QUERYBUF");
        }
        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, video_fd, buf.m.offset);
        if(buffers[i].start == MAP_FAILED) {
            errno_exit("mmap");
        }
    }
}

// 启动采集流
void start_capturing()
{
    for (unsigned int i = 0; i < n_buffers; i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;
        if(ioctl(video_fd, VIDIOC_QBUF, &buf) < 0) {
            errno_exit("VIDIOC_QBUF");
        }
    }
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(video_fd, VIDIOC_STREAMON, &type) < 0) {
        errno_exit("VIDIOC_STREAMON");
    }
}

// 停止采集流并释放资源
void stop_capturing()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(video_fd, VIDIOC_STREAMOFF, &type);
    for (unsigned int i = 0; i < n_buffers; i++) {
        munmap(buffers[i].start, buffers[i].length);
    }
    delete [] buffers;
    buffers = nullptr;
    if(video_fd != -1) close(video_fd);
    video_fd = -1;
}

// 视频采集线程：采集摄像头帧数据，转换格式后更新全局 latest_rgb
void video_capture_thread()
{
    // 打开摄像头设备
    video_fd = open(deviceName, O_RDWR | O_NONBLOCK, 0);
    if(video_fd < 0) {
        perror("open");
        running = false;
        return;
    }
    init_device();
    init_mmap();
    start_capturing();

    // 分配全局 RGB 缓冲区内存
    latest_rgb = new uint8_t[WIDTH * HEIGHT * 3];
    if(!latest_rgb) {
        std::cerr << "Unable to allocate memory for latest_rgb\n";
        running = false;
        return;
    }

    while(running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(video_fd, &fds);
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        int r = select(video_fd + 1, &fds, NULL, NULL, &tv);
        if(r == -1) {
            if(errno == EINTR)
                continue;
            perror("select");
            break;
        }
        if(r == 0) {
            std::cerr << "select timeout\n";
            continue;
        }
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if(ioctl(video_fd, VIDIOC_DQBUF, &buf) < 0) {
            if(errno == EAGAIN)
                continue;
            perror("VIDIOC_DQBUF");
            break;
        }
        // 将采集到的 YUYV 数据转换为 RGB24
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            yuyv_to_rgb(static_cast<uint8_t*>(buffers[buf.index].start), latest_rgb, WIDTH, HEIGHT);
        }
        frame_cond.notify_one();
        if(ioctl(video_fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            break;
        }
    }
    stop_capturing();
}

// HTTP 服务器线程，使用 cpp-httplib 实现简单的控制接口
void http_server_thread()
{
    using namespace httplib;
    Server svr;

    g_http_server = &svr;
    
    svr.Get("/set_parameter", [](const Request& req, Response& res) {
        if (req.has_param("brightness")) {
            try {
                int brightness = std::stoi(req.get_param_value("brightness"));
                {
                    std::lock_guard<std::mutex> lock(control_mutex);
                    g_brightness = brightness;
                    // 调用 V4L2 控制接口设置 brightness（仅当视频设备已打开）
                    if(video_fd != -1) {
                        struct v4l2_control control;
                        memset(&control, 0, sizeof(control));
                        control.id = V4L2_CID_BRIGHTNESS;
                        control.value = brightness;
                        if(ioctl(video_fd, VIDIOC_S_CTRL, &control) < 0) {
                            perror("VIDIOC_S_CTRL");
                        }
                    }
                }
                res.set_content("{\"status\": \"brightness set\"}", "application/json");
            } catch (std::exception& e) {
                res.set_content("{\"error\": \"invalid brightness value\"}", "application/json");
            }
        } else {
            res.set_content("{\"error\": \"brightness parameter missing\"}", "application/json");
        }
    });
    svr.Get("/", [](const Request&, Response& res) {
        res.set_content("{\"msg\": \"Hello from cpp-httplib server\"}", "application/json");
    });
    std::cout << "HTTP server running on port " << HTTP_PORT << "...\n";
    svr.listen("0.0.0.0", HTTP_PORT);
}

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;

    Camera cam;
    // 加载配置文件 config.yaml
    if (cam.loadFromYaml("config.yaml")) {
        std::cout << "Camera Config Loaded Successfully:\n";
        std::cout << "  Name: " << cam.name << "\n";
        std::cout << "  Path: " << cam.path << "\n";
    }
    else {
        std::cerr << "Failed to load camera configuration.\n";
        return 1;
    }

    strcpy(deviceName, cam.path.c_str());

    // 捕获 SIGINT (Ctrl+C) 信号退出
    signal(SIGINT, [](int signum) {
        (void)signum;
        running = false;
    });

    // 启动视频采集线程
    std::thread video_thread(video_capture_thread);
    // 启动 HTTP 服务器线程（使用 cpp-httplib 替代 libmicrohttpd）
    std::thread http_thread(http_server_thread);

    // 初始化 SDL3
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        running = false;
    }
    SDL_Window* window = SDL_CreateWindow("V4L2 + SDL3 Video Display", WIDTH, HEIGHT, 0);
    if(!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        running = false;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if(!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        running = false;
    }
    // 创建与 RGB24 数据格式匹配的 texture
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
                                               WIDTH, HEIGHT);
    if(!texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        running = false;
    }
    
    // 主循环：每次等待新的一帧数据后更新 texture 并渲染
    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        {
            std::unique_lock<std::mutex> lock(frame_mutex);
            // 等待新帧通知
            frame_cond.wait_for(lock, std::chrono::milliseconds(30));
            // 更新 SDL texture（pitch 为每行字节数: WIDTH * 3）
            SDL_UpdateTexture(texture, nullptr, latest_rgb, WIDTH * 3);
        }
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    // 清理 SDL 资源
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // 程序退出前等待其他线程结束
    if(video_thread.joinable())
        video_thread.join();
    if(http_thread.joinable())
        http_thread.join();
    if(latest_rgb) {
        delete [] latest_rgb;
        latest_rgb = nullptr;
    }
    std::cout << "Program terminated.\n";
    return 0;
}
