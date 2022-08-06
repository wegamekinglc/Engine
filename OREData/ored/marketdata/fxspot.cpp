/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
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

/*! \file ored/marketdata/fxspot.cpp
    \brief
    \ingroup
*/

#include <ored/marketdata/fxspot.hpp>
#include <ored/marketdata/marketdatum.hpp>
#include <ored/utilities/marketdata.hpp>

namespace ore {
namespace data {

FXSpot::FXSpot(const Date& asof, FXSpotSpec spec, const FXTriangulation& fxTriangulation,
               const Market* market) {

    string ccyPair = spec.unitCcy() + spec.ccy();
    auto spot = fxTriangulation.getQuote(ccyPair);
    
    Natural spotDays;
    Calendar calendar;
    getFxIndexConventions(ccyPair, spotDays, calendar);

    Handle<YieldTermStructure> sorTS, tarTS;
    // get the discount curves for the source and target currencies
    // if spot days are zero we can build a curve without the discount curve
    try {
        sorTS = market->discountCurve(spec.unitCcy(), Market::defaultConfiguration);
    } catch (...) {
        if (spotDays > 0)
            QL_REQUIRE(sorTS.currentLink(),
                       "Discount Curve - " << spec.unitCcy() << " - not found during Fx Spot build");
    }

    try {
        tarTS = market->discountCurve(spec.ccy(), Market::defaultConfiguration);
    } catch (...) {
        if (spotDays > 0)
            QL_REQUIRE(tarTS.currentLink(), "Discount Curve - " << spec.ccy() << " - not found during Fx Spot build");
    }

    index_ = Handle<QuantExt::FxIndex>(
        boost::make_shared<QuantExt::FxIndex>(asof, ccyPair, spotDays, parseCurrency(spec.unitCcy()), 
            parseCurrency(spec.ccy()), calendar, spot, sorTS, tarTS, false));

}
} // namespace data
} // namespace ore
