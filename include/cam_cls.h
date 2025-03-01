#pragma once

#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

class Camera {
public:
    // 成员变量保存相机名称和设备路径
    std::string name;
    std::string path;

    // loadFromYaml 方法从给定的 YAML 文件中加载 "camera" 节点信息
    // 如果加载成功，返回 true，否则返回 false
    bool loadFromYaml(const std::string &filename) {
        try {
            // 从文件加载 YAML 配置
            YAML::Node config = YAML::LoadFile(filename);

            // 检查是否有 camera 节点
            if (!config["camera"]) {
                std::cerr << "Error: 'camera' section not found in " << filename << "\n";
                return false;
            }
            YAML::Node cameraNode = config["camera"];

            // 检查并转换 name 字段
            if (cameraNode["name"]) {
                name = cameraNode["name"].as<std::string>();
            } else {
                std::cerr << "Error: 'name' not found in 'camera' section\n";
                return false;
            }

            // 检查并转换 path 字段
            if (cameraNode["path"]) {
                path = cameraNode["path"].as<std::string>();
            } else {
                std::cerr << "Error: 'path' not found in 'camera' section\n";
                return false;
            }
        }
        catch (const YAML::Exception &e) {
            std::cerr << "YAML Exception: " << e.what() << "\n";
            return false;
        }
        return true;
    }
};
