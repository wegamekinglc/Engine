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

/*! \file ored/marketdata/equityvolcurve.hpp
    \brief Wrapper class for building Equity volatility structures
    \ingroup curves
*/

#pragma once

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ored/marketdata/loader.hpp>
#include <ored/marketdata/curvespec.hpp>
#include <ored/configuration/conventions.hpp>
#include <ored/configuration/curveconfigurations.hpp>

using QuantLib::Date;
using QuantLib::BlackVolTermStructure;
using ore::data::CurveConfigurations;

namespace ore {
namespace data {

//! Wrapper class for building Equity volatility structures
/*!
  \ingroup curves
*/
class EquityVolCurve {
public:
    //! \name Constructors
    //@{
    //! Default constructor
    EquityVolCurve() {}
    //! Detailed constructor
    EquityVolCurve(Date asof, EquityVolatilityCurveSpec spec, const Loader& loader,
                   const CurveConfigurations& curveConfigs);
    //@}

    //! \name Inspectors
    //@{
    const EquityVolatilityCurveSpec& spec() const { return spec_; }

    const boost::shared_ptr<BlackVolTermStructure>& volTermStructure() { return vol_; }
    //@}
private:
    EquityVolatilityCurveSpec spec_;
    boost::shared_ptr<BlackVolTermStructure> vol_;
};
}
}
