/*
 Copyright (C) 2020 Quaternion Risk Management Ltd
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

/*! \file qle/utilities/inflation.hpp
    \brief some inflation related utilities.
*/

#ifndef quantext_inflation_hpp
#define quantext_inflation_hpp

#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/instruments/bond.hpp>

namespace QuantExt {

/*! Utility function for calculating the time to a given \p date based on a given inflation index,
    \p inflationIndex, and a given inflation term structure, \p inflationTs. An optional \p dayCounter can be 
    provided to use instead of the inflation term structure day counter.

    \ingroup utilities
*/
QuantLib::Time inflationTime(const QuantLib::Date& date,
    const boost::shared_ptr<QuantLib::InflationTermStructure>& inflationTs,
    bool indexIsInterpolated,
    const QuantLib::DayCounter& dayCounter = QuantLib::DayCounter());

/*! Utility for calculating the ratio \f$ \frac{P_r(0, t)}{P_n(0, t)} \f$ where \f$ P_r(0, t) \f$ is the real zero
    coupon bond price at time zero for maturity \f$ t \f$ and \f$ P_n(0, t) \f$ is the nominal zero coupon bond price.
*/
QuantLib::Real inflationGrowth(const QuantLib::Handle<QuantLib::ZeroInflationTermStructure>& ts,
    QuantLib::Time t,
    const QuantLib::DayCounter& dc,
    bool indexIsInterpolated);

/*! Utility for calculating the ratio \f$ \frac{P_r(0, t)}{P_n(0, t)} \f$ where \f$ P_r(0, t) \f$ is the real zero
    coupon bond price at time zero for maturity \f$ t \f$ and \f$ P_n(0, t) \f$ is the nominal zero coupon bond price.
*/
QuantLib::Real inflationGrowth(const QuantLib::Handle<QuantLib::ZeroInflationTermStructure>& ts,
    QuantLib::Time t, bool indexIsInterpolated);

/*! Calculate the Compound Factor to compute the nominal price from the real price
   I(t_s)/I(t_0) with I(t_s) the CPI at settlement date and I(t_0) the bond's base CPI
*/
QuantLib::Real inflationLinkedBondQuoteFactor(const boost::shared_ptr<QuantLib::Bond>& bond);

}

#endif
