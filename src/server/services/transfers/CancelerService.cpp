/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CancelerService.h"

#include <signal.h>

#include <boost/filesystem.hpp>

#include "common/Logger.h"
#include "common/ThreadSafeList.h"
#include "config/ServerConfig.h"
#include "server/DrainMode.h"
#include "server/services/webservice/ws/SingleTrStateInstance.h"


using namespace fts3::common;


namespace fts3 {
namespace server {

extern time_t stallRecords;


CancelerService::CancelerService()
{
}


CancelerService::~CancelerService()
{
}


std::string CancelerService::getServiceName()
{
    return std::string("CancelerService");
}


void CancelerService::markAsStalled()
{
    std::vector<struct message_updater> messages;
    messages.reserve(500);
    ThreadSafeList::get_instance().checkExpiredMsg(messages);
    if (!messages.empty()) {
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Reaping stalled transfers" << commit;

        boost::filesystem::path p("/var/lib/fts3/");
        boost::filesystem::space_info s = boost::filesystem::space(p);
        bool diskFull = (s.free <= 0 || s.available <= 0);
        bool updated = DBSingleton::instance().getDBObjectInstance()->markAsStalled(messages, diskFull);
        if (updated) {
            ThreadSafeList::get_instance().deleteMsg(messages);
            for (auto iter = messages.begin(); iter != messages.end(); ++iter) {
                if (iter->msg_errno == 0 && (*iter).file_id > 0
                    && std::string((*iter).job_id).length() > 0) {
                    SingleTrStateInstance::instance().sendStateMessage((*iter).job_id, (*iter).file_id);
                }
            }
        }
        messages.clear();
    }
}


void CancelerService::killCanceledByUser()
{
    std::vector<int> requestIDs;
    DBSingleton::instance().getDBObjectInstance()->getCancelJob(requestIDs);
    if (!requestIDs.empty())
    {
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Killing transfers canceled by the user" << commit;
        killRunningJob(requestIDs);
    }
}


void CancelerService::applyQueueTimeouts()
{
    std::vector<std::string> jobs;
    DBSingleton::instance().getDBObjectInstance()->setToFailOldQueuedJobs(jobs);
    if (!jobs.empty())
    {
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Applying queue timeouts" << commit;
        std::vector<std::string>::const_iterator iter2;
        for (iter2 = jobs.begin(); iter2 != jobs.end(); ++iter2)
        {
            SingleTrStateInstance::instance().sendStateMessage((*iter2), -1);
        }
        jobs.clear();
    }
}


void CancelerService::applyActiveTimeouts()
{
    std::map<int, std::string> collectJobs;
    DBSingleton::instance().getDBObjectInstance()->forceFailTransfers(collectJobs);
    if (!collectJobs.empty())
    {
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Applying ACTIVE timeouts" << commit;
        std::map<int, std::string>::const_iterator iterCollectJobs;
        for (iterCollectJobs = collectJobs.begin(); iterCollectJobs != collectJobs.end(); ++iterCollectJobs)
        {
            SingleTrStateInstance::instance().sendStateMessage(
                    (*iterCollectJobs).second,
                    (*iterCollectJobs).first);
        }
        collectJobs.clear();
    }
}


void CancelerService::applyWaitingTimeouts()
{
    std::set<std::string> canceled;
    DBSingleton::instance().getDBObjectInstance()->cancelWaitingFiles(canceled);
    if (!canceled.empty())
    {
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Canceling expired waiting files" << commit;
        for (auto iterCan = canceled.begin(); iterCan != canceled.end(); ++iterCan)
        {
            SingleTrStateInstance::instance().sendStateMessage((*iterCan), -1);
        }
        canceled.clear();
    }
}


void CancelerService::runService()
{
    unsigned int counterActiveTimeouts = 0;
    unsigned int counterQueueTimeouts = 0;
    unsigned int countReverted = 0;
    unsigned int counterWaitingTimeouts = 0;
    unsigned int counterCanceled = 0;


    while (!boost::this_thread::interruption_requested())
    {
        stallRecords = time(0);
        try
        {
            //if we drain a host, no need to check if url_copy are reporting being alive
            if (DrainMode::instance())
            {
                FTS3_COMMON_LOGGER_NEWLOG(INFO) << "Set to drain mode, no more checking url_copy for this instance!" << commit;
                boost::this_thread::sleep(boost::posix_time::seconds(15));
                continue;
            }

            markAsStalled();

            if (boost::this_thread::interruption_requested())
                return;

            /*also get jobs which have been canceled by the client*/
            counterCanceled++;
            if (counterCanceled == 10)
            {
                killCanceledByUser();
                counterCanceled = 0;
            }

            if (boost::this_thread::interruption_requested())
                return;

            /*revert to SUBMITTED if stayed in READY for too long (300 secs)*/
            countReverted++;
            if (countReverted == 300)
            {
                DBSingleton::instance().getDBObjectInstance()->revertToSubmitted();
                countReverted = 0;
            }

            if (boost::this_thread::interruption_requested())
                return;

            /*this routine is called periodically every 300 seconds*/
            counterWaitingTimeouts++;
            if (counterWaitingTimeouts == 300)
            {
                applyWaitingTimeouts();
                counterWaitingTimeouts = 0;
            }

            if (boost::this_thread::interruption_requested())
                return;

            /*force-fail stalled ACTIVE transfers*/
            if (config::ServerConfig::instance().get<bool>("CheckStalledTransfers")) {
                counterActiveTimeouts++;
                if (counterActiveTimeouts == 300)
                {
                    applyActiveTimeouts();
                    counterActiveTimeouts = 0;
                }
            }

            if (boost::this_thread::interruption_requested())
                return;

            /*set to fail all old queued jobs which have exceeded max queue time*/
            counterQueueTimeouts++;
            if (counterQueueTimeouts == 300)
            {
                applyQueueTimeouts();
                counterQueueTimeouts = 0;
            }
        }
        catch (const std::exception& e)
        {
            FTS3_COMMON_LOGGER_NEWLOG(ERR) << "CancelerService caught exception " << e.what() << commit;
            boost::this_thread::sleep(boost::posix_time::seconds(10));
            counterActiveTimeouts = 0;
            counterQueueTimeouts = 0;
            countReverted = 0;
            counterWaitingTimeouts = 0;
            counterCanceled = 0;
        }
        catch (...)
        {
            FTS3_COMMON_LOGGER_NEWLOG(ERR) << "CancelerService caught uknown exception" << commit;
            boost::this_thread::sleep(boost::posix_time::seconds(10));
            counterActiveTimeouts = 0;
            counterQueueTimeouts = 0;
            countReverted = 0;
            counterWaitingTimeouts = 0;
            counterCanceled = 0;
        }
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
}


void CancelerService::killRunningJob(const std::vector<int>& pids)
{
    for (auto iter = pids.begin(); iter != pids.end(); ++iter)
    {
        int pid = *iter;
        FTS3_COMMON_LOGGER_NEWLOG(INFO) << "Canceling and killing running processes: " << pid << commit;
        kill(pid, SIGTERM);
    }
}

} // end namespace server
} // end namespace fts3