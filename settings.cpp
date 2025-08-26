// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */

#include "settings.h"

#include <fstream>

fs::path getConfigDir() {
#if defined(_WIN32)
    const char* appData = std::getenv("APPDATA");
    if (appData) {
        return fs::path(appData) / "remote-mndset";
    }
#else // Linux/Unix
    const char* home = std::getenv("HOME");
    if (home) {
        return fs::path(home) / ".config" / "remote-mndset";
    }
#endif
    return fs::current_path() / "config"; // Fallback
}


void saveConfig(const fs::path& config_dir, const Config& config) {
    fs::create_directories(config_dir);
    auto config_path = config_dir / "config.txt";
    std::ofstream out(config_path);

    out << "HMDLinearVelocity=" << config.hmd_lin_vel << "\n";
    out << "HMDAngularVelocity=" << config.hmd_ang_vel << "\n";
    out << "ControllerLinearVelocity=" << config.controller_lin_vel << "\n";
    out << "ControllerAngularVelocity=" << config.controller_ang_vel << "\n";
    out << "MouseSensivity=" << config.mouse_sens << "\n";
    out << "GamepadAxisSensivity=" << config.gamepad_axis_sens<< "\n";
    out << "GamepadDeadZone=" << config.gamepad_dead_zone<< "\n";
    out << "ServerIP=" << config.server_ip<< "\n";
}

Config loadConfig(const fs::path& config_dir) {
    Config config {1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 0.1f, "127.0.0.1"};
    auto config_path = config_dir / "config.txt";

    if (!fs::exists(config_path)) {
        return config; // Return default values if the file does not exist
    }

    std::ifstream in(config_path);
    std::string line;

    while (std::getline(in, line)) {
        size_t delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);

            if (key == "HMDLinearVelocity") {
                config.hmd_lin_vel = std::stof(value);
            } else if (key == "HMDAngularVelocity") {
                config.hmd_ang_vel = std::stof(value);
            } else if (key == "ControllerLinearVelocity") {
                config.controller_lin_vel = std::stof(value);
            } else if (key == "ControllerLinearVelocity") {
                config.controller_lin_vel = std::stof(value);
            } else if (key == "ControllerAngularVelocity") {
                config.controller_ang_vel = std::stof(value);
            } else if (key == "MouseSensivity") {
                config.mouse_sens = std::stof(value);
            } else if (key == "GamepadAxisSensivity") {
                config.gamepad_axis_sens = std::stof(value);
            } else if (key == "GamepadDeadZone") {
                config.gamepad_dead_zone = std::stof(value);
            } else if (key == "ServerIP") {
                config.server_ip = value;
            }
        }
    }
    return config;
}
