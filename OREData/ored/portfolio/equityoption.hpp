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

/*! \file portfolio/equityoption.hpp
    \brief Equity Option data model and serialization
    \ingroup tradedata
*/

#pragma once

#include <ored/portfolio/oneassetoption.hpp>

namespace ore {
namespace data {
using std::string;

//! Serializable Equity Option
/*!
  \ingroup tradedata
*/
class EquityOption : public OneAssetOption {
public:
    //! Default constructor
    EquityOption() : OneAssetOption(AssetClass::EQ) { tradeType_ = "EquityOption"; }
    //! Constructor
    EquityOption(Envelope& env, OptionData option, string equityName, string currency, double strike, double quantity)
        : OneAssetOption(env, AssetClass::EQ, option, equityName, currency, strike, quantity) { tradeType_ = "EquityOption"; }

    //! Build QuantLib/QuantExt instrument, link pricing engine
    void build(const boost::shared_ptr<EngineFactory>&) override;

    //! \name Inspectors
    //@{
    const string& equityName() const { return assetName_; }
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node) override;
    virtual XMLNode* toXML(XMLDocument& doc) override;
    //@}
};
} // namespace data
} // namespace ore
