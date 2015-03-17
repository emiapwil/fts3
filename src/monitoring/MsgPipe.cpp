/**
 *  Copyright (c) Members of the EGEE Collaboration. 2004.
 *  See http://www.eu-egee.org/partners/ for details on the copyright
 *  holders.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *
 *
 */


#include "MsgPipe.h"
#include "half_duplex.h" /* For name of the named-pipe */
#include "utility_routines.h"
#include <iostream>
#include <string>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "concurrent_queue.h"
#include "Logger.h"
#include <vector>
#include <boost/filesystem.hpp>
#include "definitions.h"
#include <boost/lexical_cast.hpp>

extern bool stopThreads;
static bool signalReceived = false;

namespace fs = boost::filesystem;

void handler(int sig)
{
    if(!signalReceived)
        {
            signalReceived = true;

            sig = 0;
            stopThreads = true;
            std::queue<std::string> myQueue = concurrent_queue::getInstance()->the_queue;
            std::string ret;
            while(!myQueue.empty())
                {
                    ret = myQueue.front();
                    myQueue.pop();
                    send_message(ret);
                }
            sleep(5);
            exit(0);
        }
}

MsgPipe::MsgPipe()
{
    //register sig handler to cleanup resources upon exiting
    signal(SIGFPE, handler);
    signal(SIGILL, handler);
    signal(SIGSEGV, handler);
    signal(SIGBUS, handler);
    signal(SIGABRT, handler);
    signal(SIGTERM, handler);
    signal(SIGINT, handler);
    signal(SIGQUIT, handler);
}

MsgPipe::~MsgPipe()
{
}


void MsgPipe::run()
{
    std::vector<struct message_monitoring> messages;
    std::vector<struct message_monitoring>::const_iterator iter;

    while (stopThreads==false)
        {
            try
                {

                    if(fs::is_empty(fs::path(MONITORING_DIR)))
                        {
                            sleep(1);
                            continue;
                        }

                    int returnValue = runConsumerMonitoring(messages);
		    if(returnValue != 0)
		    {
		    	 errorMessage = "MSG_ERROR thrown in msg pipe " + boost::lexical_cast<std::string>(returnValue);                    
	                 logger::writeLog(errorMessage);
		    }
		    
                    if(!messages.empty())
                        {
                            for (iter = messages.begin(); iter != messages.end(); ++iter)
                                {
                                    concurrent_queue::getInstance()->push( (*iter).msg );
                                }
                            messages.clear();
                        }
                    sleep(1);
                }
            catch (const fs::filesystem_error& ex)
                {
 		    errorMessage = "MSG_ERROR ex thrown in msg pipe " + std::string(ex.what());
                    cleanup();
                    logger::writeLog(errorMessage);
                    sleep(1);
                }
            catch (...)
                {
                    errorMessage = "MSG_ERROR ex thrown in msg pipe";
                    cleanup();
                    logger::writeLog(errorMessage);
                    sleep(1);
                }
        }
}

void MsgPipe::cleanup()
{
    std::queue<std::string> myQueue = concurrent_queue::getInstance()->the_queue;
    std::string ret;
    while(!myQueue.empty())
        {
            ret = myQueue.front();
            myQueue.pop();
            send_message(ret);
        }
}
