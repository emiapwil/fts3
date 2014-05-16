/*
 *	Copyright notice:
 *	Copyright © Members of the EMI Collaboration, 2010.
 *
 *	See www.eu-emi.eu for details on the copyright holders
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 * fts3-set-priority.cpp
 *
 *  Created on: Dec 19, 2012
 *      Author: Michał Simon
 */


#include "GSoapContextAdapter.h"
#include "ui/PriorityCli.h"
#include "exception/cli_exception.h"

#include <string>
#include <vector>

#include <boost/scoped_ptr.hpp>

using namespace std;
using namespace boost;
using namespace fts3::cli;


/**
 * This is the entry point for the fts3-debug-set command line tool.
 */
int main(int ac, char* av[])
{
    try
        {
            // create and initialize the command line utility
            scoped_ptr<PriorityCli> cli (
                getCli<PriorityCli>(ac, av)
            );
            if (!cli->validate()) return 0;

            // validate command line options, and return respective gsoap context
            GSoapContextAdapter& ctx = cli->getGSoapContext();

            ctx.prioritySet(
                cli->getJobId(),
                cli->getPriority()
            );

        }
    catch(cli_exception const & ex)
        {
            cout << ex.what() << endl;
            return 1;
        }
    catch(std::exception& e)
        {
            cerr << "error: " << e.what() << "\n";
            return 1;
        }
    catch(...)
        {
            cerr << "Exception of unknown type!\n";
            return 1;
        }

    return 0;
}

