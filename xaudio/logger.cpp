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
#include "logger.h"
#include <string>

static std::string g_module;
static rdk_LogLevel g_level_stdout  = ENUM_RDK_LOG_COUNT;
static bool g_flush_stdout          = false;
static bool g_log_pid               = false;
static long g_pid                   = 0;

void rdkc_logger_init(const char*   module,
                      bool          log_pid,
                      rdk_LogLevel  level_stdout, 
                      bool          flush_stdout )
{
  static bool init_rdk_logger_once = true;

  if(init_rdk_logger_once)
  {
    rdk_logger_init("/etc/debug.ini");
    init_rdk_logger_once = false;
  }

  g_module        = module;
  g_log_pid       = log_pid;
  g_level_stdout  = level_stdout;
  g_flush_stdout  = flush_stdout;
}

bool rdkc_stdout_enabled(rdk_LogLevel level)
{
  return g_level_stdout != STDOUT_LEVEL_OFF && level <= g_level_stdout;
}

void rdkc_logger_log(rdk_LogLevel level, char const* format, ...)
{
  if(!rdk_dbg_enabled(g_module.c_str(), level) && !rdkc_stdout_enabled(level))
    return;

  static const size_t BUFFER_SIZE = 1028;
  static char buffer[BUFFER_SIZE];

  va_list args;
  va_start(args, format);

  if(vsnprintf(buffer, BUFFER_SIZE, format, args) < 0)
  {
    perror(buffer);
  }

  va_end(args);

  
  if(g_log_pid)
  {
    RDK_LOG(level, g_module.c_str(), "(%ld)%s\n", g_pid, buffer);
  }
  else
  {
    RDK_LOG(level, g_module.c_str(), "%s\n", buffer);
  }

  if(rdkc_stdout_enabled(level))
  {
    if(g_log_pid)
    {
      printf("(%ld)%s\n", g_pid, buffer);
    }
    else
    {
      printf("%s\n", buffer);
    }

    if(g_flush_stdout)
    {
      fflush(stdout);
    }
  }

}
