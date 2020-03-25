/*
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2019 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
*/
#ifndef RDKC_LOGGER_H
#define RDKC_LOGGER_H

#include "rdk_debug.h"

#define LOG_DEBUG(FORMAT, ...)  do { rdkc_logger_log(RDK_LOG_DEBUG, FORMAT, ## __VA_ARGS__); } while (0);
#define LOG_INFO(FORMAT, ...)   do { rdkc_logger_log(RDK_LOG_INFO,  FORMAT, ## __VA_ARGS__); } while (0);
#define LOG_WARN(FORMAT, ...)   do { rdkc_logger_log(RDK_LOG_WARN,  FORMAT, ## __VA_ARGS__); } while (0);
#define LOG_ERROR(FORMAT, ...)  do { rdkc_logger_log(RDK_LOG_ERROR, FORMAT, ## __VA_ARGS__); } while (0);
#define LOG_FATAL(FORMAT, ...)  do { rdkc_logger_log(RDK_LOG_FATAL, FORMAT, ## __VA_ARGS__); } while (0);
#define STDOUT_LEVEL_OFF ENUM_RDK_LOG_COUNT

void rdkc_logger_init(const char*   module,
                      bool          log_pid             = false,
                      rdk_LogLevel  level_stdout        = STDOUT_LEVEL_OFF, 
                      bool          flush_stdout        = false );

void rdkc_logger_log(rdk_LogLevel level, char const* format, ...);

#endif
