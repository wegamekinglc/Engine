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

/*! \file irlgm1fpiecewiseconstantparametrization.hpp
    \brief piecewise constant model parametrization
    \ingroup models
*/

#ifndef quantext_piecewiseconstant_irlgm1f_parametrization_hpp
#define quantext_piecewiseconstant_irlgm1f_parametrization_hpp

#include <qle/models/irlgm1fparametrization.hpp>
#include <qle/models/piecewiseconstanthelper.hpp>

namespace QuantExt {

//! LGM 1F Piecewise Constant Parametrization
/*! \ingroup models
 */
template <class TS>
class Lgm1fPiecewiseConstantParametrization : public Lgm1fParametrization<TS>,
                                              private PiecewiseConstantHelper1,
                                              private PiecewiseConstantHelper2 {
public:
    Lgm1fPiecewiseConstantParametrization(const Currency& currency, const Handle<TS>& termStructure,
                                          const Array& alphaTimes, const Array& alpha, const Array& kappaTimes,
                                          const Array& kappa, const std::string& name = std::string(),
        const boost::shared_ptr<QuantLib::Constraint>& alphaConstraint = boost::make_shared<QuantLib::NoConstraint>(),
        const boost::shared_ptr<QuantLib::Constraint>& kappaConstraint = boost::make_shared<QuantLib::NoConstraint>());
    Lgm1fPiecewiseConstantParametrization(const Currency& currency, const Handle<TS>& termStructure,
                                          const std::vector<Date>& alphaDates, const Array& alpha,
                                          const std::vector<Date>& kappaDates, const Array& kappa,
                                          const std::string& name = std::string(),
        const boost::shared_ptr<QuantLib::Constraint>& alphaConstraint = boost::make_shared<QuantLib::NoConstraint>(),
        const boost::shared_ptr<QuantLib::Constraint>& kappaConstraint = boost::make_shared<QuantLib::NoConstraint>());
    
    Real zeta(const Time t) const override;
    Real H(const Time t) const override;
    Real alpha(const Time t) const override;
    Real kappa(const Time t) const override;
    Real Hprime(const Time t) const override;
    Real Hprime2(const Time t) const override;
    const Array& parameterTimes(const Size) const override;
    const boost::shared_ptr<Parameter> parameter(const Size) const override;
    void update() const override;

protected:
    Real direct(const Size i, const Real x) const override;
    Real inverse(const Size j, const Real y) const override;

private:
    void initialize(const Array& alpha, const Array& kappa);
};

// implementation

template <class TS>
Lgm1fPiecewiseConstantParametrization<TS>::Lgm1fPiecewiseConstantParametrization(
    const Currency& currency, const Handle<TS>& termStructure, const Array& alphaTimes, const Array& alpha,
    const Array& kappaTimes, const Array& kappa, const std::string& name,
    const boost::shared_ptr<QuantLib::Constraint>& alphaConstraint,
    const boost::shared_ptr<QuantLib::Constraint>& kappaConstraint)
    : Lgm1fParametrization<TS>(currency, termStructure, name),
      PiecewiseConstantHelper1(alphaTimes, alphaConstraint),
      PiecewiseConstantHelper2(kappaTimes, kappaConstraint) {
    initialize(alpha, kappa);
}

template <class TS>
Lgm1fPiecewiseConstantParametrization<TS>::Lgm1fPiecewiseConstantParametrization(
    const Currency& currency, const Handle<TS>& termStructure, const std::vector<Date>& alphaDates, const Array& alpha,
    const std::vector<Date>& kappaDates, const Array& kappa, const std::string& name,
    const boost::shared_ptr<QuantLib::Constraint>& alphaConstraint,
    const boost::shared_ptr<QuantLib::Constraint>& kappaConstraint)
    : Lgm1fParametrization<TS>(currency, termStructure, name),
      PiecewiseConstantHelper1(alphaDates, termStructure, alphaConstraint),
      PiecewiseConstantHelper2(kappaDates, termStructure, kappaConstraint) {
    initialize(alpha, kappa);
}

template <class TS> void Lgm1fPiecewiseConstantParametrization<TS>::initialize(const Array& alpha, const Array& kappa) {
    QL_REQUIRE(PiecewiseConstantHelper1::t().size() + 1 == alpha.size(),
               "alpha size (" << alpha.size() << ") inconsistent to times size ("
                              << PiecewiseConstantHelper1::t().size() << ")");
    QL_REQUIRE(PiecewiseConstantHelper2::t().size() + 1 == kappa.size(),
               "kappa size (" << kappa.size() << ") inconsistent to times size ("
                              << PiecewiseConstantHelper2::t().size() << ")");
    // store raw parameter values
    for (Size i = 0; i < PiecewiseConstantHelper1::y_->size(); ++i) {
        PiecewiseConstantHelper1::y_->setParam(i, inverse(0, alpha[i]));
    }
    for (Size i = 0; i < PiecewiseConstantHelper2::y_->size(); ++i) {
        PiecewiseConstantHelper2::y_->setParam(i, inverse(1, kappa[i]));
    }
    update();
}

// inline

template <class TS> inline Real Lgm1fPiecewiseConstantParametrization<TS>::direct(const Size i, const Real x) const {
    return i == 0 ? PiecewiseConstantHelper1::direct(x) : PiecewiseConstantHelper2::direct(x);
}

template <class TS> inline Real Lgm1fPiecewiseConstantParametrization<TS>::inverse(const Size i, const Real y) const {
    return i == 0 ? PiecewiseConstantHelper1::inverse(y) : PiecewiseConstantHelper2::inverse(y);
}

template <class TS> inline Real Lgm1fPiecewiseConstantParametrization<TS>::zeta(const Time t) const {
    return PiecewiseConstantHelper1::int_y_sqr(t) / (this->scaling_ * this->scaling_);
}

template <class TS> inline Real Lgm1fPiecewiseConstantParametrization<TS>::H(const Time t) const {
    return this->scaling_ * PiecewiseConstantHelper2::int_exp_m_int_y(t) + this->shift_;
}

template <class TS> inline Real Lgm1fPiecewiseConstantParametrization<TS>::alpha(const Time t) const {
    return PiecewiseConstantHelper1::y(t) / this->scaling_;
}

template <class TS> inline Real Lgm1fPiecewiseConstantParametrization<TS>::kappa(const Time t) const {
    return PiecewiseConstantHelper2::y(t);
}

template <class TS> inline Real Lgm1fPiecewiseConstantParametrization<TS>::Hprime(const Time t) const {
    return this->scaling_ * PiecewiseConstantHelper2::exp_m_int_y(t);
}

template <class TS> inline Real Lgm1fPiecewiseConstantParametrization<TS>::Hprime2(const Time t) const {
    return -this->scaling_ * PiecewiseConstantHelper2::exp_m_int_y(t) * kappa(t);
}

template <class TS> inline void Lgm1fPiecewiseConstantParametrization<TS>::update() const {
    Lgm1fParametrization<TS>::update();
    PiecewiseConstantHelper1::update();
    PiecewiseConstantHelper2::update();
}

template <class TS> inline const Array& Lgm1fPiecewiseConstantParametrization<TS>::parameterTimes(const Size i) const {
    QL_REQUIRE(i < 2, "parameter " << i << " does not exist, only have 0..1");
    if (i == 0)
        return PiecewiseConstantHelper1::t_;
    else
        return PiecewiseConstantHelper2::t_;
}

template <class TS>
inline const boost::shared_ptr<Parameter> Lgm1fPiecewiseConstantParametrization<TS>::parameter(const Size i) const {
    QL_REQUIRE(i < 2, "parameter " << i << " does not exist, only have 0..1");
    if (i == 0)
        return PiecewiseConstantHelper1::y_;
    else
        return PiecewiseConstantHelper2::y_;
}

// typedef

typedef Lgm1fPiecewiseConstantParametrization<YieldTermStructure> IrLgm1fPiecewiseConstantParametrization;

} // namespace QuantExt

#endif
