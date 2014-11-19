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
 * DnCli.cpp
 *
 *  Created on: Mar 13, 2012
 *      Author: Michal Simon
 */

#include "DnCli.h"

#include <iostream>
using namespace std;

using namespace fts3::cli;

DnCli::DnCli()
{

    // add fts3-transfer-status specific options
    specific.add_options()
    ("userdn,u", value<string>(), "Restrict to specific user DN.")
    ;
}

DnCli::~DnCli()
{
}

std::string DnCli::getUserDn()
{

    if (vm.count("userdn"))
        {
            return vm["userdn"].as<std::string>();
        }

    return std::string();
}

