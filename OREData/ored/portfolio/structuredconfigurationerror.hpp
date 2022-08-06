/*
 Copyright (C) 2021 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file ored/portfolio/structuredconfigurationerror.hpp
    \brief Class for structured configuration errors
    \ingroup portfolio
*/

#pragma once

#include <boost/algorithm/string.hpp>
#include <ored/utilities/log.hpp>

namespace ore {
namespace data {

//! Utility classes for Structured configuration errors, contains the configuration type and ID (NettingSetId, CounterParty, etc.)
class StructuredConfigurationErrorMessage : public StructuredErrorMessage {
public:
    StructuredConfigurationErrorMessage(const std::string& configurationType, const std::string& configurationId,
                                        const std::string& exceptionType, const std::string exceptionWhat)
        : configurationType_(configurationType), configurationId_(configurationId), exceptionType_(exceptionType),
          exceptionWhat_(exceptionWhat) {}

    const std::string& configurationType() const { return configurationType_; }
    const std::string& configurationId() const { return configurationId_; }
    const std::string& exceptionType() const { return exceptionType_; }
    const std::string& exceptionWhat() const { return exceptionWhat_; }

protected:
    std::string json() const override {
        return "{ \"errorType\":\"Configuration\", \"configType\":\"" + configurationType_ + "\"," +
               " \"configId\":\"" + configurationId_ + "\"," + " \"exceptionType\":\"" + exceptionType_ + "\"," +
               " \"exceptionMessage\":\"" + jsonify(exceptionWhat_) + "\"}";
    }

private:
    std::string configurationType_, configurationId_, exceptionType_, exceptionWhat_;
};

} // namespace data
} // namespace ore
