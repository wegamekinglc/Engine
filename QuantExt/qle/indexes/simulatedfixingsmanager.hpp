/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2016 Quaternion Risk Management Ltd.
  All rights reserved.
*/

/*! \file simulatedfixingsmanager.hpp
    \brief global respository for simulated fixings
*/

#ifndef quantext_simulated_fixings_manager_hpp
#define quantext_simulated_fixings_manager_hpp

#include <ql/settings.hpp>
#include <ql/timeseries.hpp>

using namespace QuantLib;

namespace QuantExt {

/*! global repository for simulated fixings, the assumption is
 that after a newPath() call the evaluation date moves forward
 and on the way fixings are added as "backward-fixings"
 (estimation as of the evaluation date) or "forward-fixings"
 (projections as of a future date) */

template <class T = Real>
class SimulatedFixingsManager_t
    : public Singleton<SimulatedFixingsManager_t<T> > {
    friend class Singleton<SimulatedFixingsManager_t<T> >;

  private:
    SimulatedFixingsManager_t() { reset(); }

  public:
    // note that it is in the pricing engine's
    // responsibility to support both the
    // forward and backward method
    enum EstimationMethod {
        Forward,
        Backward,
        BestOfForwardBackward,
        InterpolatedForwardBackward
    };

    /*! If set, estimated fixings are recorded in engines
        that support simulated fixings.
        If a historic fixing is needed and not found in the
        IndexManager as a native fixing and if simulated
        fixings are enabled, a formerly estimated one will
        be used according to the Estimation method.
    */
    bool &simulateFixings();
    bool simulateFixings() const;
    EstimationMethod &estimationMethod();
    EstimationMethod estimationMethod() const;

    /*! If set to a non-zero value, only fixing are
      stored that are less than the given number
      of caledar days in the future. A value greater
      than the greatest simulation step should be
      chosen. Null means no limit. */
    BigInteger &horizon();
    BigInteger horizon() const;

    /*! resets the simulated fixings settings */
    void reset();

    /*! clears recorded fixings and sets the reference
      date to the current evaluation date */
    void newPath() const;

    /*! adds a projected fixing (forward method) */
    void addForwardFixing(const std::string &name, const Date &fixingDate,
                          const T &value) const;

    /*! adds a fixing as of evaluation date (backward method) */
    void addBackwardFixing(const std::string &name, const T &value) const;

    /*! returns true if backward fixing is present */
    bool hasBackwardFixing(const std::string &name) const;

    /*! returns a simulated fixing, might be Null<T>, if data innsufficient */
    T simulatedFixing(const std::string &name, const Date &fixingDate) const;

  private:
    bool simulateFixings_;
    EstimationMethod estimationMethod_;
    BigInteger horizon_;
    mutable Date referenceDate_;
    // forward and backward data
    struct CompHlp {
        bool operator()(const std::pair<Date, T> &o1,
                        const std::pair<Date, T> &o2) {
            return o1.first < o2.first;
        }
    };
    typedef std::map<Date, std::pair<T, Date> > ForwardData;
    typedef std::map<Date, T> BackwardData;
    mutable std::map<std::string, ForwardData> forwardData_;
    mutable std::map<std::string, BackwardData> backwardData_;
};

typedef SimulatedFixingsManager_t<Real> SimulatedFixingsManager;

// inline

template <class T>
inline bool &SimulatedFixingsManager_t<T>::simulateFixings() {
    return simulateFixings_;
}

template <class T>
inline bool SimulatedFixingsManager_t<T>::simulateFixings() const {
    return simulateFixings_;
}

template <class T>
inline typename SimulatedFixingsManager_t<T>::EstimationMethod &
SimulatedFixingsManager_t<T>::estimationMethod() {
    return estimationMethod_;
}

template <class T>
inline typename SimulatedFixingsManager_t<T>::EstimationMethod
SimulatedFixingsManager_t<T>::estimationMethod() const {
    return estimationMethod_;
}

template <class T> inline BigInteger &SimulatedFixingsManager_t<T>::horizon() {
    return horizon_;
}

template <class T>
inline BigInteger SimulatedFixingsManager_t<T>::horizon() const {
    return horizon_;
}

// implementation

template <class T> void SimulatedFixingsManager_t<T>::reset() {
    simulateFixings_ = false;
    estimationMethod_ = Backward;
    horizon_ = Null<BigInteger>();
    referenceDate_ = Null<Date>();
}

template <class T> void SimulatedFixingsManager_t<T>::newPath() const {
    forwardData_.clear();
    backwardData_.clear();
    referenceDate_ = Settings::instance().evaluationDate();
}

template <class T>
void SimulatedFixingsManager_t<T>::addForwardFixing(const std::string &name,
                                                    const Date &fixingDate,
                                                    const T &value) const {

    QL_REQUIRE(referenceDate_ != Null<Date>(),
               "can not add estimation for simulated fixing for "
                   << name << " @ " << value << " on " << fixingDate
                   << ", need a newPath call first");

    QL_REQUIRE(referenceDate_ <= fixingDate,
               "can not add estimation for simulated fixing for "
                   << name << " @ " << value << " on " << fixingDate
                   << ", since reference date (" << referenceDate_
                   << ") is past fixing date");

    if (horizon_ == Null<BigInteger>() ||
        fixingDate - Settings::instance().evaluationDate() < horizon_) {
        forwardData_[name][fixingDate] =
            std::make_pair(value, Settings::instance().evaluationDate());
    }
}

template <class T>
void SimulatedFixingsManager_t<T>::addBackwardFixing(const std::string &name,
                                                     const T &value) const {

    // this might in fact be a holiday, nevertheless we store
    // the fixing on the evaluation date
    Date fixingDate = Settings::instance().evaluationDate();

    QL_REQUIRE(referenceDate_ != Null<Date>(),
               "can not add estimation for simulated fixing for "
                   << name << " @ " << value << " on " << fixingDate
                   << ", need a newPath call first");

    QL_REQUIRE(referenceDate_ <= fixingDate,
               "can not add estimation for simulated fixing for "
                   << name << " @ " << value << " on " << fixingDate
                   << ", since reference date (" << referenceDate_
                   << ") is past fixing date");

    backwardData_[name][fixingDate] = value;
}

template <class T>
bool SimulatedFixingsManager_t<T>::hasBackwardFixing(
    const std::string &name) const {

    // see above
    Date fixingDate = Settings::instance().evaluationDate();
    typename std::map<std::string, BackwardData>::const_iterator it =
        backwardData_.find(name);
    if (it == backwardData_.end())
        return false;
    typename BackwardData::const_iterator it2 = it->second.find(fixingDate);
    return it2 != it->second.end();
}

template <class T>
T SimulatedFixingsManager_t<T>::simulatedFixing(const std::string &name,
                                                const Date &fixingDate) const {

    T bwdTmp = Null<T>();
    Date bwdDate = Null<Date>();
    if (estimationMethod_ == Backward ||
        estimationMethod_ == BestOfForwardBackward ||
        estimationMethod_ == InterpolatedForwardBackward) {
        typename BackwardData::const_iterator it = std::lower_bound(
            backwardData_[name].begin(), backwardData_[name].end(),
            std::make_pair(fixingDate, T(0.0)), CompHlp());
        // fixing date not > all backward data
        if (it != backwardData_[name].end()) {
            bwdDate = it->first;
            bwdTmp = it->second;
        }
        if (estimationMethod_ == Backward) {
            // might be Null<T>
            return bwdTmp;
        }
    }

    T fwdTmp = Null<T>();
    Date fwdDate = Null<Date>();
    typename ForwardData::const_iterator it =
        forwardData_[name].find(fixingDate);
    if (it != forwardData_[name].end()) {
        fwdTmp = it->second.first;
        fwdDate = it->second.second;
    }

    if (estimationMethod_ == Forward) {
        // might be Null<T>
        return fwdTmp;
    }

    // remaining methods are requiring both fwd and bwd estimate
    // if only one is available, we fall back on the respective
    // forward or backward only method
    if (bwdTmp == Null<T>()) {
        return fwdTmp;
    }
    if (fwdTmp == Null<T>()) {
        return bwdTmp;
    }

    // we have both estimates
    BigInteger fwdDistance = fixingDate - fwdDate;
    BigInteger bwdDistance = bwdDate - fixingDate;
    if (estimationMethod_ == BestOfForwardBackward || fwdDistance == 0) {
        if (fwdDistance <= bwdDistance) {
            return fwdTmp;
        } else {
            return bwdTmp;
        }
    }

    if (estimationMethod_ == InterpolatedForwardBackward) {
        T tmp = (fwdTmp * static_cast<T>(bwdDistance) +
                 bwdTmp * static_cast<T>(fwdDistance)) /
                (static_cast<T>(bwdDistance) + static_cast<T>(fwdDistance));
        return tmp;
    }

    QL_FAIL("unexpected estimation method (" << estimationMethod_ << ")");
}

} // namespace QuantLib

#endif
