// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */


#ifndef SETTINGS_H
#define SETTINGS_H

#include "structs.h"

#include <filesystem>

namespace fs = std::filesystem;

fs::path getConfigDir();
void saveConfig(const fs::path& config_dir, const Config& config);
Config loadConfig(const fs::path& config_dir);

#endif // SETTINGS_H
