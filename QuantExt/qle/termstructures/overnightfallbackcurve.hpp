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

/*! \file qle/termstructures/iborfallbackcurve.hpp
    \brief projection curve for ibor fallback indices
*/

#pragma once

#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantExt {
using namespace QuantLib;

class OvernightFallbackCurve : public QuantLib::YieldTermStructure {
public:
    OvernightFallbackCurve(const boost::shared_ptr<OvernightIndex>& originalIndex,
			   const boost::shared_ptr<OvernightIndex>& rfrIndex, const Real spread, const Date& switchDate);

    boost::shared_ptr<OvernightIndex> originalIndex() const;
    boost::shared_ptr<OvernightIndex> rfrIndex() const;
    Real spread() const;
    const Date& switchDate() const;

    const Date& referenceDate() const override;
    Date maxDate() const override;
    Calendar calendar() const override;
    Natural settlementDays() const override;

private:
    Real discountImpl(QuantLib::Time t) const override;

    boost::shared_ptr<OvernightIndex> originalIndex_;
    boost::shared_ptr<OvernightIndex> rfrIndex_;
    Real spread_;
    Date switchDate_;
};

} // namespace QuantExt
