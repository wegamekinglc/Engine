/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
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

#include <ored/configuration/cdsvolcurveconfig.hpp>
#include <ql/errors.hpp>

using ore::data::XMLUtils;

namespace ore {
namespace data {

CDSVolatilityCurveConfig::CDSVolatilityCurveConfig(const string& curveID, const string& curveDescription,
                                                   const vector<string>& expiries)
    : curveID_(curveID), curveDescription_(curveDescription), expiries_(expiries) {}

void CDSVolatilityCurveConfig::fromXML(XMLNode* node) {
    XMLUtils::checkNode(node, "CDSVolatility");

    curveID_ = XMLUtils::getChildValue(node, "CurveId", true);
    curveDescription_ = XMLUtils::getChildValue(node, "CurveDescription", true);
    expiries_ = XMLUtils::getChildrenValuesAsStrings(node, "Expiries", true);
}

XMLNode* CDSVolatilityCurveConfig::toXML(XMLDocument& doc) {
    XMLNode* node = doc.allocNode("CDSVolatility");

    XMLUtils::addChild(doc, node, "CurveId", curveID_);
    XMLUtils::addChild(doc, node, "CurveDescription", curveDescription_);
    XMLUtils::addGenericChildAsList(doc, node, "Expiries", expiries_);

    return node;
}
} // namespace data
} // namespace ore