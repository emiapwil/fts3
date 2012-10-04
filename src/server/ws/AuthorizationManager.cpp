/*
 *	Copyright notice:
 *	Copyright © Members of the EMI Collaboration, 2010.
 *
 *	See www.eu-emi.eu for details on the copyright holders
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use soap file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implcfgied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 * AuthorizationManager.cpp
 *
 *  Created on: Jul 18, 2012
 *      Author: Michal Simon
 */

#include "AuthorizationManager.h"
#include "GSoapDelegationHandler.h"

#include "common/error.h"
#include "common/logger.h"

#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

#include "config/serverconfig.h"

using namespace fts3::ws;
using namespace config;
using namespace boost;
using namespace boost::assign;

AuthorizationManager::AuthorizationManager() :
		voNameList(theServerConfig().get< vector<string> >("AuthorizedVO")),
		voNameSet(voNameList.begin(), voNameList.end()) {
	// initialization list does everything :)
}

AuthorizationManager::~AuthorizationManager() {

}

void AuthorizationManager::authorize(soap* soap, bool submit) {

	if (voNameList.empty()) return;

	GSoapDelegationHandler handler (soap);
	if(!submit && handler.isRoot()) return;

	string vo = handler.getClientVo();
	to_lower(vo);

	if (!voNameSet.count(vo)) {

		string msg = "Authorization failed, access was not granted. ";
		msg += "(Please check if the fts3 configuration file contains the VO: '";
		msg += vo;
		msg += "' and if the right delimiter was used!)";


		throw Err_Custom(msg);
	}
}

