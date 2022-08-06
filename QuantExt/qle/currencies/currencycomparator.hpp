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

/*! \file qle/currencies/currencycomparator.hpp
    \brief Compare currencies by currency code
        \ingroup currencies
*/

#pragma once

#include <ql/currency.hpp>

namespace QuantExt {

struct CurrencyComparator {
    bool operator()(const QuantLib::Currency& c1, const QuantLib::Currency& c2) const {
        std::string code1 = c1.empty() ? "" : c1.code();
        std::string code2 = c2.empty() ? "" : c2.code();
        return code1 < code2;
    }
};

} // namespace QuantExt
