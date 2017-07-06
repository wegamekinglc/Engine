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

/*! \file ored/configuration/fxvolcurveconfig.hpp
    \brief FX volatility curve configuration classes
    \ingroup configuration
*/

#pragma once

#include <ored/configuration/curveconfig.hpp>
#include <ql/time/period.hpp>
#include <ql/types.hpp>

using std::string;
using std::vector;
using ore::data::XMLNode;
using ore::data::XMLDocument;
using QuantLib::Period;

namespace ore {
namespace data {

//! FX volatility structure configuration
/*!
  \ingroup configuration
*/
class FXVolatilityCurveConfig : public CurveConfig {
public:
    //! supported volatility structure types
    /*! For ATM we will only load ATM quotes, for Smile we load ATM, 25RR, 25BF
     *  TODO: Add more options (e.g. Delta)
     */
    enum class Dimension { ATM, Smile };

    //! \name Constructors/Destructors
    //@{
    //! Default constructor
    FXVolatilityCurveConfig() {}
    //! Detailed constructor
    FXVolatilityCurveConfig(const string& curveID, const string& curveDescription, const Dimension& dimension,
                            const vector<Period>& expiries);
    //! Default destructor
    virtual ~FXVolatilityCurveConfig() {}
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

    //! \name Inspectors
    //@{
    const Dimension& dimension() const { return dimension_; }
    const vector<Period>& expiries() const { return expiries_; }
    // only required for Smile
    const string& fxSpotID() const { return fxSpotID_; }
    const string& fxForeignYieldCurveID() const { return fxForeignYieldCurveID_; }
    const string& fxDomesticYieldCurveID() const { return fxDomesticYieldCurveID_; }
    //@}

    //! \name Setters
    //@{
    Dimension& dimension() { return dimension_; }
    vector<Period>& expiries() { return expiries_; }
    string& fxSpotID() { return fxSpotID_; }
    string& fxForeignYieldCurveID() { return fxForeignYieldCurveID_; }
    string& fxDomesticYieldCurveID() { return fxDomesticYieldCurveID_; }
    //@}
private:
    Dimension dimension_;
    vector<Period> expiries_;
    string fxSpotID_;
    string fxForeignYieldCurveID_;
    string fxDomesticYieldCurveID_;
};
} // namespace data
} // namespace ore
