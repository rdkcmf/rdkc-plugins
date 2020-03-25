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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include "rdkc_daemon.h"


/*
class audio_client
{
};

class audio_recorder
{
  void client_connect(audio_client* client)
  {
    clients->push_back(client)
    record_if_needed();  
  }

  void client_disconnect(audio_client* client)
  {
    std::list<audio_client*> iterator it = clients.find(client);
    if(it != clients.end())
    {
      clients.erase(it);
    }
    record_if_needed();
  }

  void record_if_needed()
  {
    if(clients->size() > 0)
    {
      if(!is_recording)
        start_recording();
    }
    else
    {
      if(is_recording)
        stop_recording();
    }
  }

  void start_recording()
  {
  }

  void stop_recording()
  {
  }

  std::list<audio_client*> clients;
  bool is_recording;
};
*/

void usage()
{
  LOG_INFO("Usage: xaudiodaemon [OPTIONS]\n");
  LOG_INFO("-f foreground   run in forground rather than as deamon");
  LOG_INFO("-v verbose      enable debug logging");
  LOG_INFO("-h help         print usage");
}

int main(int argc, char *argv[])
{
  LOG_INFO("xaudio starting");

  bool foreground = false;
  bool verbose = false;
  int optch, optidx;
  static struct option long_options[] = 
  {
    {"foreground", required_argument, 0, 'f'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  while( (optch = getopt_long(argc, argv, "fvh", long_options, &optidx)) != -1 )
  {
    switch (optch)
    {
      case 'f': foreground = true; break;
      case 'v': verbose = true; break;
      case 'h': usage(); exit(0);
      default: break;
    }
  }

  rdkc_logger_start("LOG.RDK.XAUDIO", 
                    verbose ? RDK_LOG_DEBUG : RDK_LOG_INFO, 
                    verbose, verbose);

  if(!daemon_start("xaudio", foreground))
  {
    return -1;
  }

  while(daemon_running())
  {
  }

  daemon_stop();

  LOG_INFO("xaudio stopping");

  return 0;
}



