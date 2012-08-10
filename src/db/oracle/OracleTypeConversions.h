/********************************************//**
 * Copyright @ Members of the EMI Collaboration, 2010.
 * See www.eu-emi.eu for details on the copyright holders.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0 
 * 
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 ***********************************************/

/**
 * @file OracleTypeConversions.h
 * @brief data type conversion between occi types and standard types
 * @author Michail Salichos
 * @date 09/02/2012
 * 
 **/

#pragma once

#include <ctime>
#include <string>
#include "occi.h"

typedef long long longlong;

class OracleTypeConversions{

public:

    OracleTypeConversions(){}
    virtual ~OracleTypeConversions(){}

/*
 * toTimeT
 *
 * Convert a Timestamp into a time value
 */
 time_t toTimeT(const ::oracle::occi::Timestamp& timestamp);
 
 
/*
 * toTimestamp
 *
 * Convert a time value into a Timestamp 
 */
 oracle::occi::Timestamp toTimestamp(time_t t, oracle::occi::Environment* env);
 
 /*
 * toLongLong
 *
 * Convert a Number into a longlong
 */
 longlong toLongLong(const ::oracle::occi::Number& number, oracle::occi::Environment* env);
 
 /*
 * toNumber
 *
 * Convert a longlong into a Number
 */
 oracle::occi::Number toNumber(longlong n, oracle::occi::Environment* env);
 
 /*
 * toBoolean
 *
 * Convert a Char into a boolean
 */
 bool toBoolean(const std::string& str, bool defaultValue) ;
 
 /*
 * toBoolean
 *
 * Convert a Boolean into a String
 */
 const std::string& toBoolean(bool b) ;
 
 /*
 * toString
 *
 * Read a Clob into a String
 */
 void toString(::oracle::occi::Clob clob, std::string& str);

};
