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

/*! \file ored/portfolio/optiondata.hpp
    \brief trade option data model and serialization
    \ingroup tradedata
*/

#pragma once

#include <ored/portfolio/optionexercisedata.hpp>
#include <ored/portfolio/optionpaymentdata.hpp>
#include <ored/portfolio/premiumdata.hpp>
#include <ored/portfolio/schedule.hpp>

#include <ql/cashflow.hpp>
#include <ql/exercise.hpp>
#include <ql/time/date.hpp>

namespace ore {
namespace data {

//! Serializable object holding option data
/*!
  \ingroup tradedata
*/
class OptionData : public XMLSerializable {
public:
    //! Default constructor
    OptionData() : payoffAtExpiry_(true), automaticExercise_(false) {}
    //! Constructor
    OptionData(string longShort, string callPut, string style, bool payoffAtExpiry, vector<string> exerciseDates,
               string settlement = "Cash", string settlementMethod = "", const PremiumData& premiumData = {},
               vector<double> exerciseFees = vector<Real>(), vector<double> exercisePrices = vector<Real>(),
               string noticePeriod = "", string noticeCalendar = "", string noticeConvention = "",
               const vector<string>& exerciseFeeDates = vector<string>(),
               const vector<string>& exerciseFeeTypes = vector<string>(), string exerciseFeeSettlementPeriod = "",
               string exerciseFeeSettlementCalendar = "", string exerciseFeeSettlementConvention = "",
               string payoffType = "", const boost::optional<bool>& automaticExercise = boost::none,
               const boost::optional<OptionExerciseData>& exerciseData = boost::none,
               const boost::optional<OptionPaymentData>& paymentData = boost::none)
        : longShort_(longShort), callPut_(callPut), payoffType_(payoffType), style_(style),
          payoffAtExpiry_(payoffAtExpiry), exerciseDates_(exerciseDates), noticePeriod_(noticePeriod),
          noticeCalendar_(noticeCalendar), noticeConvention_(noticeConvention), settlement_(settlement),
          settlementMethod_(settlementMethod), premiumData_(premiumData), exerciseFees_(exerciseFees),
          exerciseFeeDates_(exerciseFeeDates), exerciseFeeTypes_(exerciseFeeTypes),
          exerciseFeeSettlementPeriod_(exerciseFeeSettlementPeriod),
          exerciseFeeSettlementCalendar_(exerciseFeeSettlementCalendar),
          exerciseFeeSettlementConvention_(exerciseFeeSettlementConvention), exercisePrices_(exercisePrices),
          automaticExercise_(automaticExercise), exerciseData_(exerciseData), paymentData_(paymentData) {}

    //! \name Inspectors
    //@{
    const string& longShort() const { return longShort_; }
    const string& callPut() const { return callPut_; }
    const string& payoffType() const { return payoffType_; }
    const string& style() const { return style_; }
    const bool& payoffAtExpiry() const { return payoffAtExpiry_; }
    const vector<string>& exerciseDates() const { return exerciseDates_; }
    const string& noticePeriod() const { return noticePeriod_; }
    const string& noticeCalendar() const { return noticeCalendar_; }
    const string& noticeConvention() const { return noticeConvention_; }
    const string& settlement() const { return settlement_; }
    const string& settlementMethod() const { return settlementMethod_; }
    const PremiumData& premiumData() const { return premiumData_; }
    const vector<double>& exerciseFees() const { return exerciseFees_; }
    const vector<string>& exerciseFeeDates() const { return exerciseFeeDates_; }
    const vector<string>& exerciseFeeTypes() const { return exerciseFeeTypes_; }
    const string& exerciseFeeSettlementPeriod() const { return exerciseFeeSettlementPeriod_; }
    const string& exerciseFeeSettlementCalendar() const { return exerciseFeeSettlementCalendar_; }
    const string& exerciseFeeSettlementConvention() const { return exerciseFeeSettlementConvention_; }
    const vector<double>& exercisePrices() const { return exercisePrices_; }
    boost::optional<bool> automaticExercise() const { return automaticExercise_; }
    const boost::optional<OptionExerciseData>& exerciseData() const { return exerciseData_; }
    const boost::optional<OptionPaymentData>& paymentData() const { return paymentData_; }
    //@}

    //! \name Setters
    //@{
    void setExerciseDates(const std::vector<std::string>& exerciseDates) { exerciseDates_ = exerciseDates; }
    void setAutomaticExercise(bool automaticExercise) { automaticExercise_ = automaticExercise; }
    void setPaymentData(const OptionPaymentData& paymentData) { paymentData_ = paymentData; }
    void setCallPut(const string& callPut) { callPut_ = callPut; }
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node) override;
    virtual XMLNode* toXML(XMLDocument& doc) override;
    //@}

    //! Automatic exercise assumed false if not explicitly provided.
    bool isAutomaticExercise() const { return automaticExercise_ ? *automaticExercise_ : false; }

private:
    string longShort_;    // long or short
    string callPut_;      // call or put
    string payoffType_;   // Accumulator, Decumulator, TargetExact, TargetFull, TargetTruncated, ...
    string style_;        // European, Bermudan, American
    bool payoffAtExpiry_; // Y or N
    vector<string> exerciseDates_;
    string noticePeriod_;
    string noticeCalendar_;
    string noticeConvention_;
    string settlement_;       // Cash or Physical, default Cash.
    string settlementMethod_; // QuantLib::Settlement::Method, default empty
    PremiumData premiumData_;
    vector<double> exerciseFees_;
    vector<string> exerciseFeeDates_;
    vector<string> exerciseFeeTypes_;
    string exerciseFeeSettlementPeriod_;
    string exerciseFeeSettlementCalendar_;
    string exerciseFeeSettlementConvention_;
    vector<double> exercisePrices_;
    boost::optional<bool> automaticExercise_;
    boost::optional<OptionExerciseData> exerciseData_;
    boost::optional<OptionPaymentData> paymentData_;
};

/* Helper class to build an exercise instance for Bermudan swaptions and related instruments from given option data.
   Only future exercise dates that exercise into a whole future accrual period of the underlying are kept.
   If no exercise dates are left, exercise() will return a nullptr. */

class ExerciseBuilder {
public:
    ExerciseBuilder(const OptionData& optionData, const std::vector<QuantLib::Leg> legs);

    boost::shared_ptr<QuantLib::Exercise> exercise() const { return exercise_; }
    const std::vector<QuantLib::Date>& exerciseDates() const { return exerciseDates_; }
    const std::vector<QuantLib::Date>& noticeDates() const { return noticeDates_; }

private:
    boost::shared_ptr<QuantLib::Exercise> exercise_;
    std::vector<QuantLib::Date> exerciseDates_;
    std::vector<QuantLib::Date> noticeDates_;
};

} // namespace data
} // namespace ore
