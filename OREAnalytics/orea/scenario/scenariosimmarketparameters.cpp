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

#include <set>

#include <orea/scenario/scenariosimmarketparameters.hpp>
#include <ored/utilities/log.hpp>
#include <ored/utilities/xmlutils.hpp>

#include <boost/lexical_cast.hpp>

using namespace QuantLib;
using namespace ore::data;
using std::set;

namespace ore {
namespace analytics {

namespace {
const vector<Period>& returnTenors(const map<string, vector<Period>>& m, const string& k) {
    if (m.count(k) > 0) {
        return m.at(k);
    } else if (m.count("") > 0) {
        return m.at("");
    } else
        QL_FAIL("no period vector for key \"" << k << "\" found.");
}
const string& returnDayCounter(const map<string, string>& m, const string& k) {
    if (m.count(k) > 0) {
        return m.at(k);
    } else if (m.count("") > 0) {
        return m.at("");
    } else
        QL_FAIL("no dayCounter for key \"" << k << "\" found.");
}

} // namespace

vector<string> ScenarioSimMarketParameters::paramsLookup(RiskFactorKey::KeyType kt) const {
    vector<string> names;
    for (auto it = params_.begin(); it != params_.end(); ++it) {
        if (it->keytype == kt) {
            names.push_back(it->name);
        }
    }
    return names;
}

bool ScenarioSimMarketParameters::hasParamsKey(RiskFactorKey rfk) const {
    return std::find(params_.begin(), params_.end(), rfk) == params_.end() ? false : true;
}

void ScenarioSimMarketParameters::addParams(RiskFactorKey::KeyType kt, vector<string> names) {
    for (auto name : names) {
        RiskFactorKey rfk(kt, name);
        if (!hasParamsKey(rfk))
            params_.push_back(rfk);
    }
}

void ScenarioSimMarketParameters::setDefaults() {
    // Set default tenors (don't know why but keep it as is)
    capFloorVolExpiries_[""] = vector<Period>();
    defaultTenors_[""] = vector<Period>();
    equityDividendTenors_[""] = vector<Period>();
    equityForecastTenors_[""] = vector<Period>();
    zeroInflationTenors_[""] = vector<Period>();
    yoyInflationTenors_[""] = vector<Period>();
    commodityCurveTenors_[""] = vector<Period>();
    // Default day counters
    yieldCurveDayCounters_[""] = "A365";
    swapVolDayCounters_[""] = "A365";
    fxVolDayCounters_[""] = "A365";
    cdsVolDayCounters_[""] = "A365";
    equityVolDayCounters_[""] = "A365";
    capFloorVolDayCounters_[""] = "A365";
    defaultCurveDayCounters_[""] = "A365";
    baseCorrelationDayCounters_[""] = "A365";
    zeroInflationDayCounters_[""] = "A365";
    yoyInflationDayCounters_[""] = "A365";
    commodityCurveDayCounters_[""] = "A365";
    commodityVolDayCounters_[""] = "A365";
    // Default calendars
    defaultCurveCalendars_[""] = "TARGET";
}

void ScenarioSimMarketParameters::reset() {
    ScenarioSimMarketParameters ssmp;
    std::swap(*this, ssmp);
}

const vector<Period>& ScenarioSimMarketParameters::yieldCurveTenors(const string& key) const {
    return returnTenors(yieldCurveTenors_, key);
}

const string& ScenarioSimMarketParameters::yieldCurveDayCounter(const string& key) const {
    return returnDayCounter(yieldCurveDayCounters_, key);
}

const vector<Period>& ScenarioSimMarketParameters::capFloorVolExpiries(const string& key) const {
    return returnTenors(capFloorVolExpiries_, key);
}

const vector<Period>& ScenarioSimMarketParameters::defaultTenors(const string& key) const {
    return returnTenors(defaultTenors_, key);
}

const string& ScenarioSimMarketParameters::defaultCurveDayCounter(const string& key) const {
    return returnDayCounter(defaultCurveDayCounters_, key);
}

const string& ScenarioSimMarketParameters::defaultCurveCalendar(const string& key) const {
    return returnDayCounter(defaultCurveCalendars_, key);
}

const string& ScenarioSimMarketParameters::swapVolDayCounter(const string& key) const {
    return returnDayCounter(swapVolDayCounters_, key);
}

const string& ScenarioSimMarketParameters::fxVolDayCounter(const string& key) const {
    return returnDayCounter(fxVolDayCounters_, key);
}

const string& ScenarioSimMarketParameters::cdsVolDayCounter(const string& key) const {
    return returnDayCounter(cdsVolDayCounters_, key);
}

const string& ScenarioSimMarketParameters::equityVolDayCounter(const string& key) const {
    return returnDayCounter(equityVolDayCounters_, key);
}

const string& ScenarioSimMarketParameters::capFloorVolDayCounter(const string& key) const {
    return returnDayCounter(capFloorVolDayCounters_, key);
}

const vector<Period>& ScenarioSimMarketParameters::equityDividendTenors(const string& key) const {
    return returnTenors(equityDividendTenors_, key);
}

const vector<Period>& ScenarioSimMarketParameters::equityForecastTenors(const string& key) const {
    return returnTenors(equityForecastTenors_, key);
}

const vector<Period>& ScenarioSimMarketParameters::zeroInflationTenors(const string& key) const {
    return returnTenors(zeroInflationTenors_, key);
}

const string& ScenarioSimMarketParameters::zeroInflationDayCounter(const string& key) const {
    return returnDayCounter(zeroInflationDayCounters_, key);
}

const vector<Period>& ScenarioSimMarketParameters::yoyInflationTenors(const string& key) const {
    return returnTenors(yoyInflationTenors_, key);
}

const string& ScenarioSimMarketParameters::yoyInflationDayCounter(const string& key) const {
    return returnDayCounter(yoyInflationDayCounters_, key);
}

const string& ScenarioSimMarketParameters::baseCorrelationDayCounter(const string& key) const {
    return returnDayCounter(baseCorrelationDayCounters_, key);
}

bool ScenarioSimMarketParameters::commodityCurveSimulate() const {
    return commodityCurveSimulate_;
}

vector<string> ScenarioSimMarketParameters::commodityNames() const {
    return paramsLookup(RiskFactorKey::KeyType::CommoditySpot);
}

const vector<Period>& ScenarioSimMarketParameters::commodityCurveTenors(const string& commodityName) const {
    return returnTenors(commodityCurveTenors_, commodityName);
}

bool ScenarioSimMarketParameters::hasCommodityCurveTenors(const string& commodityName) const {
    return commodityCurveTenors_.count(commodityName) > 0;
}

const string& ScenarioSimMarketParameters::commodityCurveDayCounter(const string& commodityName) const {
    return returnDayCounter(commodityCurveDayCounters_, commodityName);
}

const vector<Period>& ScenarioSimMarketParameters::commodityVolExpiries(const string& commodityName) const {
    return returnTenors(commodityVolExpiries_, commodityName);
}

const vector<Real>& ScenarioSimMarketParameters::commodityVolMoneyness(const string& commodityName) const {
    if (commodityVolMoneyness_.count(commodityName) > 0) {
        return commodityVolMoneyness_.at(commodityName);
    } else {
        QL_FAIL("no moneyness for commodity \"" << commodityName << "\" found.");
    }
}

const string& ScenarioSimMarketParameters::commodityVolDayCounter(const string& commodityName) const {
    return returnDayCounter(commodityVolDayCounters_, commodityName);
}

void ScenarioSimMarketParameters::setYieldCurveTenors(const string& key, const std::vector<Period>& p) {
    yieldCurveTenors_[key] = p;
}

void ScenarioSimMarketParameters::setYieldCurveDayCounters(const string& key, const string& s) {
    yieldCurveDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setCapFloorVolExpiries(const string& key, const std::vector<Period>& p) {
    capFloorVolExpiries_[key] = p;
}

void ScenarioSimMarketParameters::setDefaultTenors(const string& key, const std::vector<Period>& p) {
    defaultTenors_[key] = p;
}

void ScenarioSimMarketParameters::setDefaultCurveDayCounters(const string& key, const string& s) {
    defaultCurveDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setDefaultCurveCalendars(const string& key, const string& s) {
    defaultCurveCalendars_[key] = s;
}

void ScenarioSimMarketParameters::setBaseCorrelationDayCounters(const string& key, const string& s) {
    baseCorrelationDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setEquityDividendTenors(const string& key, const std::vector<Period>& p) {
    equityDividendTenors_[key] = p;
}

void ScenarioSimMarketParameters::setEquityForecastTenors(const string& key, const std::vector<Period>& p) {
    equityForecastTenors_[key] = p;
}

void ScenarioSimMarketParameters::setZeroInflationTenors(const string& key, const std::vector<Period>& p) {
    zeroInflationTenors_[key] = p;
}

void ScenarioSimMarketParameters::setZeroInflationDayCounters(const string& key, const string& s) {
    zeroInflationDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setYoyInflationTenors(const string& key, const std::vector<Period>& p) {
    yoyInflationTenors_[key] = p;
}

void ScenarioSimMarketParameters::setYoyInflationDayCounters(const string& key, const string& s) {
    yoyInflationDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setFxVolDayCounters(const string& key, const string& s) {
    fxVolDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setSwapVolDayCounters(const string& key, const string& s) {
    swapVolDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setCdsVolDayCounters(const string& key, const string& s) {
    cdsVolDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setEquityVolDayCounters(const string& key, const string& s) {
    equityVolDayCounters_[key] = s;
}

void ScenarioSimMarketParameters::setCapFloorVolDayCounters(const string& key, const string& s) {
    capFloorVolDayCounters_[key] = s;
}

bool& ScenarioSimMarketParameters::commodityCurveSimulate() {
    return commodityCurveSimulate_;
}

void ScenarioSimMarketParameters::setCommodityNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::CommoditySpot, names);
}

void ScenarioSimMarketParameters::setCommodityCurveTenors(const string& commodityName, const vector<Period>& p) {
    commodityCurveTenors_[commodityName] = p;
}

void ScenarioSimMarketParameters::setCommodityCurveDayCounter(const string& commodityName, const string& d) {
    commodityCurveDayCounters_[commodityName] = d;
}

void ScenarioSimMarketParameters::setCommodityVolDayCounter(const string& commodityName, const string& d) {
    commodityVolDayCounters_[commodityName] = d;
}

void ScenarioSimMarketParameters::setDiscountCurveNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::DiscountCurve, names);
}

void ScenarioSimMarketParameters::setYieldCurveNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::YieldCurve, names);
}

void ScenarioSimMarketParameters::setIndices(vector<string> names) {
    addParams(RiskFactorKey::KeyType::IndexCurve, names);
}

void ScenarioSimMarketParameters::setFxCcyPairs(vector<string> names) {
    addParams(RiskFactorKey::KeyType::FXSpot, names);
}

void ScenarioSimMarketParameters::setSwapVolCcys(vector<string> names) {
    addParams(RiskFactorKey::KeyType::SwaptionVolatility, names);
}

void ScenarioSimMarketParameters::setCapFloorVolCcys(vector<string> names) {
    addParams(RiskFactorKey::KeyType::OptionletVolatility, names);
}

void ScenarioSimMarketParameters::setDefaultNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::SurvivalProbability, names);
}

void ScenarioSimMarketParameters::setCdsVolNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::CDSVolatility, names);
}

void ScenarioSimMarketParameters::setEquityNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::EquitySpot, names);
}

void ScenarioSimMarketParameters::setEquityForecastCurves(vector<string> names) {
    addParams(RiskFactorKey::KeyType::EquityForecastCurve, names);
}

void ScenarioSimMarketParameters::setEquityDividendCurves(vector<string> names) {
    addParams(RiskFactorKey::KeyType::DividendYield, names);
}

void ScenarioSimMarketParameters::setFxVolCcyPairs(vector<string> names) {
    addParams(RiskFactorKey::KeyType::FXVolatility, names);
}


void ScenarioSimMarketParameters::setEquityVolNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::EquityVolatility, names);
}

void ScenarioSimMarketParameters::setSecurities(vector<string> names) {
    addParams(RiskFactorKey::KeyType::SecuritySpread, names);
}

void ScenarioSimMarketParameters::setRecoveryRates(vector<string> names) {
    addParams(RiskFactorKey::KeyType::RecoveryRate, names);
}

void ScenarioSimMarketParameters::setBaseCorrelationNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::BaseCorrelation, names);
}

void ScenarioSimMarketParameters::setCpiIndices(vector<string> names) {
    addParams(RiskFactorKey::KeyType::CPIIndex, names);
}

void ScenarioSimMarketParameters::setZeroInflationIndices(vector<string> names) {
    addParams(RiskFactorKey::KeyType::ZeroInflationCurve, names);
}

void ScenarioSimMarketParameters::setYoyInflationIndices(vector<string> names) {
    addParams(RiskFactorKey::KeyType::YoYInflationCurve, names);
}

void ScenarioSimMarketParameters::setCommodityVolNames(vector<string> names) {
    addParams(RiskFactorKey::KeyType::CommodityVolatility, names);
}

void ScenarioSimMarketParameters::setCommodityCurves(vector<string> names) {
    addParams(RiskFactorKey::KeyType::CommodityCurve, names);
}

bool ScenarioSimMarketParameters::operator==(const ScenarioSimMarketParameters& rhs) {

    if (baseCcy_ != rhs.baseCcy_ || ccys_ != rhs.ccys_ || params_ != rhs.params_ || 
        yieldCurveDayCounters_ != rhs.yieldCurveDayCounters_ || yieldCurveCurrencies_ != rhs.yieldCurveCurrencies_ ||
        yieldCurveTenors_ != rhs.yieldCurveTenors_ || swapIndices_ != rhs.swapIndices_ ||
        interpolation_ != rhs.interpolation_ || extrapolate_ != rhs.extrapolate_ ||
        swapVolTerms_ != rhs.swapVolTerms_ || swapVolDayCounters_ != rhs.swapVolDayCounters_ || swapVolSimulate_ != rhs.swapVolSimulate_ || 
        swapVolIsCube_ != rhs.swapVolIsCube_ || swapVolSimulateATMOnly_ != rhs.swapVolSimulateATMOnly_ ||
        swapVolExpiries_ != rhs.swapVolExpiries_ || swapVolStrikeSpreads_ != rhs.swapVolStrikeSpreads_ ||
        swapVolDecayMode_ != rhs.swapVolDecayMode_ || capFloorVolSimulate_ != rhs.capFloorVolSimulate_ ||
        capFloorVolDayCounters_ != rhs.capFloorVolDayCounters_ ||
        capFloorVolExpiries_ != rhs.capFloorVolExpiries_ || capFloorVolStrikes_ != rhs.capFloorVolStrikes_ ||
        capFloorVolDecayMode_ != rhs.capFloorVolDecayMode_ || survivalProbabilitySimulate_ != rhs.survivalProbabilitySimulate_ ||
        recoveryRateSimulate_ != rhs.recoveryRateSimulate_ ||
        defaultCurveDayCounters_ != rhs.defaultCurveDayCounters_ || defaultCurveCalendars_ != rhs.defaultCurveCalendars_ || defaultTenors_ != rhs.defaultTenors_ ||
        cdsVolSimulate_ != rhs.cdsVolSimulate_ || cdsVolExpiries_ != rhs.cdsVolExpiries_ || cdsVolDayCounters_ != rhs.cdsVolDayCounters_ ||
        cdsVolDecayMode_ != rhs.cdsVolDecayMode_ || equityDividendTenors_ != rhs.equityDividendTenors_ ||
        equityForecastTenors_ != rhs.equityForecastTenors_ || equityForecastCurveSimulate_ != rhs.equityForecastCurveSimulate_ ||
        dividendYieldSimulate_ != rhs.dividendYieldSimulate_ || fxVolSimulate_ != rhs.fxVolSimulate_ ||
        fxVolIsSurface_ != rhs.fxVolIsSurface_ || fxVolExpiries_ != rhs.fxVolExpiries_ ||
        fxVolDayCounters_ != rhs.fxVolDayCounters_ || fxVolDecayMode_ != rhs.fxVolDecayMode_ ||
        fxSpotSimulate_ != rhs.fxSpotSimulate_ || equityVolSimulate_ != rhs.equityVolSimulate_ ||
        equityVolExpiries_ != rhs.equityVolExpiries_ || equityVolDayCounters_ != rhs.equityVolDayCounters_ ||
        equityVolDecayMode_ != rhs.equityVolDecayMode_ ||
        equityIsSurface_ != rhs.equityIsSurface_ || equityVolSimulateATMOnly_ != rhs.equityVolSimulateATMOnly_ ||
        equityMoneyness_ != rhs.equityMoneyness_ ||
        additionalScenarioDataIndices_ != rhs.additionalScenarioDataIndices_ ||
        additionalScenarioDataCcys_ != rhs.additionalScenarioDataCcys_ || securitySpreadsSimulate_ != rhs.securitySpreadsSimulate_ || 
        baseCorrelationSimulate_ != rhs.baseCorrelationSimulate_ ||
        baseCorrelationTerms_ != rhs.baseCorrelationTerms_ ||
        baseCorrelationDayCounters_ != rhs.baseCorrelationDayCounters_ ||
        baseCorrelationDetachmentPoints_ != rhs.baseCorrelationDetachmentPoints_ ||
        zeroInflationDayCounters_ != rhs.zeroInflationDayCounters_ ||
        zeroInflationTenors_ != rhs.zeroInflationTenors_ || 
        yoyInflationDayCounters_ != rhs.yoyInflationDayCounters_ || yoyInflationTenors_ != rhs.yoyInflationTenors_ || 
        commodityCurveSimulate_ != rhs.commodityCurveSimulate_ || 
        commodityCurveTenors_ != rhs.commodityCurveTenors_ || commodityCurveDayCounters_ != rhs.commodityCurveDayCounters_ ||
        commodityVolSimulate_ != rhs.commodityVolSimulate_ || commodityVolDecayMode_ != rhs.commodityVolDecayMode_ ||
        commodityVolExpiries_ != rhs.commodityVolExpiries_ ||
        commodityVolMoneyness_ != rhs.commodityVolMoneyness_ || commodityVolDayCounters_ != rhs.commodityVolDayCounters_) {
        return false;
    } else {
        return true;
    }
}

bool ScenarioSimMarketParameters::operator!=(const ScenarioSimMarketParameters& rhs) { return !(*this == rhs); }

void ScenarioSimMarketParameters::fromXML(XMLNode* root) {

    // fromXML always uses a "clean" object
    reset();

    DLOG("ScenarioSimMarketParameters::fromXML()");

    XMLNode* sim = XMLUtils::locateNode(root, "Simulation");
    XMLNode* node = XMLUtils::getChildNode(sim, "Market");
    XMLUtils::checkNode(node, "Market");

    // TODO: add in checks (checkNode or QL_REQUIRE) on mandatory nodes
    DLOG("Loading Currencies");
    baseCcy_ = XMLUtils::getChildValue(node, "BaseCurrency");
    ccys_ = XMLUtils::getChildrenValues(node, "Currencies", "Currency");
    setDiscountCurveNames(ccys_);

    DLOG("Loading BenchmarkCurve");
    XMLNode* nodeChild = XMLUtils::getChildNode(node, "BenchmarkCurves");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        vector<string> yields;
        for (XMLNode* n = XMLUtils::getChildNode(nodeChild, "BenchmarkCurve"); n != nullptr;
             n = XMLUtils::getNextSibling(n, "BenchmarkCurve")) {
            yields.push_back(XMLUtils::getChildValue(n, "Name", true));
            yieldCurveCurrencies_[XMLUtils::getChildValue(n, "Name", true)] = XMLUtils::getChildValue(n, "Currency", true);
        }
        setYieldCurveNames(yields);
    }

    DLOG("Loading YieldCurves");
    nodeChild = XMLUtils::getChildNode(node, "YieldCurves");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        for (XMLNode* child = XMLUtils::getChildNode(nodeChild, "Configuration"); 
            child; child = XMLUtils::getNextSibling(child)) {

            // If there is no attribute "curve", this returns "" i.e. the default
            string label = XMLUtils::getAttribute(child, "curve");
            if (label == "") {
                interpolation_ = XMLUtils::getChildValue(child, "Interpolation", true);
                extrapolate_ = XMLUtils::getChildValueAsBool(child, "Extrapolate");
                yieldCurveTenors_[label] = XMLUtils::getChildrenValuesAsPeriods(child, "Tenors", true);
            } else {
                if (XMLUtils::getChildNode(child, "Interpolation")) {
                    WLOG("Only one default interpolation value is allowed for yield curves");
                }
                if (XMLUtils::getChildNode(child, "Extrapolate")) {
                    WLOG("Only one default extrapolation value is allowed for yield curves");
                }
                if (XMLUtils::getChildNode(child, "Tenors")) {
                    yieldCurveTenors_[label] = XMLUtils::getChildrenValuesAsPeriods(child, "Tenors", true);
                }
            }

            if (XMLUtils::getChildNode(child, "DayCounter")) {
                yieldCurveDayCounters_[label] = XMLUtils::getChildValue(child, "DayCounter", true);
            }
        }
    }

    DLOG("Loading Libor indices");
    setIndices(XMLUtils::getChildrenValues(node, "Indices", "Index"));
    
    DLOG("Loading swap indices");
    nodeChild = XMLUtils::getChildNode(node, "SwapIndices");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        for (XMLNode* n = XMLUtils::getChildNode(nodeChild, "SwapIndex"); n != nullptr;
             n = XMLUtils::getNextSibling(n, "SwapIndex")) {
            string name = XMLUtils::getChildValue(n, "Name");
            string disc = XMLUtils::getChildValue(n, "DiscountingIndex");
            swapIndices_[name] = disc;
        }
    }

    DLOG("Loading FX Rates");
    nodeChild = XMLUtils::getChildNode(node, "FxRates");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        XMLNode* fxSpotSimNode = XMLUtils::getChildNode(nodeChild, "Simulate");
        if (fxSpotSimNode)
            fxSpotSimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(fxSpotSimNode));
        // if currency pairs are specified load these, otherwise infer from currencies list and base currency
        XMLNode* ccyPairsNode = XMLUtils::getChildNode(nodeChild, "CurrencyPairs");
        if (ccyPairsNode) {
            setFxCcyPairs(XMLUtils::getChildrenValues(nodeChild, "CurrencyPairs", "CurrencyPair", true));
        } else {
            vector<string> ccys;
            for (auto ccy : ccys_) {
                if (ccy != baseCcy_)
                    ccys.push_back(ccy + baseCcy_);
            }
            setFxCcyPairs(ccys);
        }
    } else {
        // spot simulation turned on by default
        fxSpotSimulate_ = true;
        vector<string> ccys;
        for (auto ccy : ccys_) {
            if (ccy != baseCcy_)
                ccys.push_back(ccy + baseCcy_);
        }
        setFxCcyPairs(ccys);
    }

    DLOG("Loading SwaptionVolatilities");
    nodeChild = XMLUtils::getChildNode(node, "SwaptionVolatilities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        XMLNode* swapVolSimNode = XMLUtils::getChildNode(nodeChild, "Simulate");
        if (swapVolSimNode) {
            swapVolSimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(swapVolSimNode));
            swapVolTerms_ = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Terms", true);
            swapVolExpiries_ = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Expiries", true);
            setSwapVolCcys(XMLUtils::getChildrenValues(nodeChild, "Currencies", "Currency", true));
            swapVolDecayMode_ = XMLUtils::getChildValue(nodeChild, "ReactionToTimeDecay");
            XMLNode* cubeNode = XMLUtils::getChildNode(nodeChild, "Cube");
            if (cubeNode) {
                swapVolIsCube_ = true;
                XMLNode* atmOnlyNode = XMLUtils::getChildNode(cubeNode, "SimulateATMOnly");
                if (atmOnlyNode) {
                    swapVolSimulateATMOnly_ = XMLUtils::getChildValueAsBool(cubeNode, "SimulateATMOnly", true);
                } else {
                    swapVolSimulateATMOnly_ = false;
                }
                if (!swapVolSimulateATMOnly_)
                    swapVolStrikeSpreads_ =
                        XMLUtils::getChildrenValuesAsDoublesCompact(cubeNode, "StrikeSpreads", true);
            } else {
                swapVolIsCube_ = false;
            }
            XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
            if (dc) {
                for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                     child = XMLUtils::getNextSibling(child)) {
                    string label = XMLUtils::getAttribute(child, "ccy");
                    swapVolDayCounters_[label] = XMLUtils::getNodeValue(child);
                }
            }
            QL_REQUIRE(swapVolDayCounters_.find("") != swapVolDayCounters_.end(),
                       "default daycounter is not set for swapVolSurfaces");
        }
    }

    DLOG("Loading CapFloorVolatilities");
    nodeChild = XMLUtils::getChildNode(node, "CapFloorVolatilities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        capFloorVolSimulate_ = false;
        XMLNode* capVolSimNode = XMLUtils::getChildNode(nodeChild, "Simulate");
        if (capVolSimNode)
            capFloorVolSimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(capVolSimNode));
        capFloorVolExpiries_[""] = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Expiries", true);
        // TODO read other keys
        capFloorVolStrikes_ = XMLUtils::getChildrenValuesAsDoublesCompact(nodeChild, "Strikes", true);
        setCapFloorVolCcys(XMLUtils::getChildrenValues(nodeChild, "Currencies", "Currency", true));
        capFloorVolDecayMode_ = XMLUtils::getChildValue(nodeChild, "ReactionToTimeDecay");
        XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
        if (dc) {
            for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "ccy");
                capFloorVolDayCounters_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(capFloorVolDayCounters_.find("") != capFloorVolDayCounters_.end(),
                   "default daycounter is not set for capFloorVolSurfaces");
    }

    DLOG("Loading DefaultCurves Rates");
    nodeChild = XMLUtils::getChildNode(node, "DefaultCurves");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        setDefaultNames(XMLUtils::getChildrenValues(nodeChild, "Names", "Name", true));
        defaultTenors_[""] = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Tenors", true);
        // TODO read other keys
        XMLNode* survivalProbabilitySimNode = XMLUtils::getChildNode(nodeChild, "SimulateSurvivalProbabilities");
        if (survivalProbabilitySimNode)
            survivalProbabilitySimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(survivalProbabilitySimNode));
        XMLNode* recoveryRateSimNode = XMLUtils::getChildNode(nodeChild, "SimulateRecoveryRates");
        if (recoveryRateSimNode)
            recoveryRateSimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(recoveryRateSimNode));

        XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
        if (dc) {
            for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "name");
                defaultCurveDayCounters_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(defaultCurveDayCounters_.find("") != defaultCurveDayCounters_.end(),
                   "default daycounter is not set  for defaultCurves");

        XMLNode* cal = XMLUtils::getChildNode(nodeChild, "Calendars");
        if (cal) {
            for (XMLNode* child = XMLUtils::getChildNode(cal, "Calendar"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "name");
                defaultCurveCalendars_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(defaultCurveCalendars_.find("") != defaultCurveCalendars_.end(),
                   "default calendar is not set for defaultCurves");
    }

    DLOG("Loading Equities Rates");
    nodeChild = XMLUtils::getChildNode(node, "Equities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        XMLNode* equityForecastCurveSimNode = XMLUtils::getChildNode(nodeChild, "SimulateEquityForecastCurve");
        if (equityForecastCurveSimNode)
            equityForecastCurveSimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(equityForecastCurveSimNode));
        else
            equityForecastCurveSimulate_ = true;
        XMLNode* dividendYieldSimNode = XMLUtils::getChildNode(nodeChild, "SimulateDividendYield");
        if (dividendYieldSimNode)
            dividendYieldSimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(dividendYieldSimNode));
        else
            dividendYieldSimulate_ = false;
        vector<string> equityNames = XMLUtils::getChildrenValues(nodeChild, "Names", "Name", true);
        setEquityNames(equityNames);
        setEquityForecastCurves(equityNames);
        setEquityDividendCurves(equityNames);
        equityDividendTenors_[""] = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "DividendTenors", true);
        equityForecastTenors_[""] = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "ForecastTenors", true);
    }

    DLOG("Loading CDSVolatilities Rates");
    nodeChild = XMLUtils::getChildNode(node, "CDSVolatilities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        XMLNode* cdsVolSimNode = XMLUtils::getChildNode(nodeChild, "Simulate");
        if (cdsVolSimNode)
            cdsVolSimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(cdsVolSimNode));
        cdsVolExpiries_ = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Expiries", true);
        setCdsVolNames(XMLUtils::getChildrenValues(nodeChild, "Names", "Name", true));
        cdsVolDecayMode_ = XMLUtils::getChildValue(nodeChild, "ReactionToTimeDecay");
        XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
        if (dc) {
            for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "name");
                cdsVolDayCounters_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(cdsVolDayCounters_.find("") != cdsVolDayCounters_.end(),
                   "default daycounter is not set for cdsVolSurfaces");
    }

    DLOG("Loading FXVolatilities");
    nodeChild = XMLUtils::getChildNode(node, "FxVolatilities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        fxVolSimulate_ = false;
        XMLNode* fxVolSimNode = XMLUtils::getChildNode(nodeChild, "Simulate");
        if (fxVolSimNode)
            fxVolSimulate_ = ore::data::parseBool(XMLUtils::getNodeValue(fxVolSimNode));
        fxVolExpiries_ = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Expiries", true);
        fxVolDecayMode_ = XMLUtils::getChildValue(nodeChild, "ReactionToTimeDecay");
        setFxVolCcyPairs(XMLUtils::getChildrenValues(nodeChild, "CurrencyPairs", "CurrencyPair", true));           
        XMLNode* fxSurfaceNode = XMLUtils::getChildNode(nodeChild, "Surface");
        if (fxSurfaceNode) {
            fxVolIsSurface_ = true;
            fxMoneyness_ = XMLUtils::getChildrenValuesAsDoublesCompact(fxSurfaceNode, "Moneyness", true);
        } else {
            fxVolIsSurface_ = false;
            fxMoneyness_ = {0.0};
        }
        XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
        if (dc) {
            for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "ccyPair");
                fxVolDayCounters_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(fxVolDayCounters_.find("") != fxVolDayCounters_.end(),
                   "default daycounter is not set for fxVolSurfaces");
    }

    DLOG("Loading EquityVolatilities");
    nodeChild = XMLUtils::getChildNode(node, "EquityVolatilities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        equityVolSimulate_ = XMLUtils::getChildValueAsBool(nodeChild, "Simulate", true);
        equityVolExpiries_ = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Expiries", true);
        equityVolDecayMode_ = XMLUtils::getChildValue(nodeChild, "ReactionToTimeDecay");
        setEquityVolNames(XMLUtils::getChildrenValues(nodeChild, "Names", "Name", true));
        XMLNode* eqSurfaceNode = XMLUtils::getChildNode(nodeChild, "Surface");
        if (eqSurfaceNode) {
            equityIsSurface_ = true;
            XMLNode* atmOnlyNode = XMLUtils::getChildNode(eqSurfaceNode, "SimulateATMOnly");
            if (atmOnlyNode) {
                equityVolSimulateATMOnly_ = XMLUtils::getChildValueAsBool(eqSurfaceNode, "SimulateATMOnly", true);
            } else {
                equityVolSimulateATMOnly_ = false;
            }
            if (!equityVolSimulateATMOnly_)
                equityMoneyness_ = XMLUtils::getChildrenValuesAsDoublesCompact(eqSurfaceNode, "Moneyness", true);
        } else {
            equityIsSurface_ = false;
        }
        XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
        if (dc) {
            for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "name");
                equityVolDayCounters_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(equityVolDayCounters_.find("") != equityVolDayCounters_.end(),
                   "default daycounter is not set for equityVolSurfaces");

    }

    DLOG("Loading CpiInflationIndexCurves");
    setCpiIndices(XMLUtils::getChildrenValues(node, "CpiIndices", "Index", false));

    DLOG("Loading ZeroInflationIndexCurves");
    nodeChild = XMLUtils::getChildNode(node, "ZeroInflationIndexCurves");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        setZeroInflationIndices(XMLUtils::getChildrenValues(nodeChild, "Names", "Name", true));
        zeroInflationTenors_[""] = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Tenors", true);

        XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
        if (dc) {
            for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "name");
                zeroInflationDayCounters_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(zeroInflationDayCounters_.find("") != zeroInflationDayCounters_.end(),
                   "default daycounter is not set for zeroInflation Surfaces");
    }

    DLOG("Loading YYInflationIndexCurves");

    nodeChild = XMLUtils::getChildNode(node, "YYInflationIndexCurves");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        setYoyInflationIndices(XMLUtils::getChildrenValues(nodeChild, "Names", "Name", true));
        yoyInflationTenors_[""] = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Tenors", true);
        XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
        if (dc) {
            for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "name");
                yoyInflationDayCounters_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(yoyInflationDayCounters_.find("") != yoyInflationDayCounters_.end(),
                   "default daycounter is not set for yoyInflation Surfaces");
    }

    DLOG("Loading AggregationScenarioDataIndices");
    if (XMLUtils::getChildNode(node, "AggregationScenarioDataIndices")) {
        additionalScenarioDataIndices_ = XMLUtils::getChildrenValues(node, "AggregationScenarioDataIndices", "Index");
    }

    DLOG("Loading AggregationScenarioDataCurrencies");
    if (XMLUtils::getChildNode(node, "AggregationScenarioDataCurrencies")) {
        additionalScenarioDataCcys_ = XMLUtils::getChildrenValues(
            node, "AggregationScenarioDataCurrencies", "Currency", true);
    }

    DLOG("Loading Securities");
    nodeChild = XMLUtils::getChildNode(node, "Securities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        securitySpreadsSimulate_ = XMLUtils::getChildValueAsBool(nodeChild, "Simulate", false);
        vector<string> securities = XMLUtils::getChildrenValues(nodeChild, "Names", "Name");
        setSecurities(securities);
        setRecoveryRates(securities);
    }

    DLOG("Loading BaseCorrelations");
    nodeChild = XMLUtils::getChildNode(node, "BaseCorrelations");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        baseCorrelationSimulate_ = XMLUtils::getChildValueAsBool(nodeChild, "Simulate", true);
        setBaseCorrelationNames(XMLUtils::getChildrenValues(nodeChild, "IndexNames", "IndexName", true));
        baseCorrelationTerms_ = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Terms", true);
        baseCorrelationDetachmentPoints_ =
            XMLUtils::getChildrenValuesAsDoublesCompact(nodeChild, "DetachmentPoints", true);

        XMLNode* dc = XMLUtils::getChildNode(nodeChild, "DayCounters");
        if (dc) {
            for (XMLNode* child = XMLUtils::getChildNode(dc, "DayCounter"); child;
                 child = XMLUtils::getNextSibling(child)) {
                string label = XMLUtils::getAttribute(child, "name");
                baseCorrelationDayCounters_[label] = XMLUtils::getNodeValue(child);
            }
        }
        QL_REQUIRE(baseCorrelationDayCounters_.find("") != baseCorrelationDayCounters_.end(),
                   "default daycounter is not set for baseCorrelation Surfaces");

    }

    DLOG("Loading commodities data");
    nodeChild = XMLUtils::getChildNode(node, "Commodities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        XMLNode* commoditySimNode = XMLUtils::getChildNode(nodeChild, "Simulate");
        commodityCurveSimulate_ = commoditySimNode ? parseBool(XMLUtils::getNodeValue(commoditySimNode)) : false;

        vector<string> commodityNames = XMLUtils::getChildrenValues(nodeChild, "Names", "Name", true);
        setCommodityNames(commodityNames);
        setCommodityCurves(commodityNames);
        commodityCurveTenors_[""] = XMLUtils::getChildrenValuesAsPeriods(nodeChild, "Tenors", true);

        // If present, override DayCounter for _all_ commodity price curves
        XMLNode* commodityDayCounterNode = XMLUtils::getChildNode(nodeChild, "DayCounter");
        if (commodityDayCounterNode) {
            commodityCurveDayCounters_[""] = XMLUtils::getNodeValue(commodityDayCounterNode);
        }
    }

    DLOG("Loading commodity volatility data");
    nodeChild = XMLUtils::getChildNode(node, "CommodityVolatilities");
    if (nodeChild && XMLUtils::getChildNode(nodeChild)) {
        commodityVolSimulate_ = XMLUtils::getChildValueAsBool(nodeChild, "Simulate", true);
        commodityVolDecayMode_ = XMLUtils::getChildValue(nodeChild, "ReactionToTimeDecay");
        
        vector<string> names;
        XMLNode* namesNode = XMLUtils::getChildNode(nodeChild, "Names");
        if (namesNode) {
            for (XMLNode* child = XMLUtils::getChildNode(namesNode, "Name"); child; child = XMLUtils::getNextSibling(child)) {
                // Get the vol configuration for each commodity name
                string name = XMLUtils::getAttribute(child, "id");
                names.push_back(name);
                commodityVolExpiries_[name] = XMLUtils::getChildrenValuesAsPeriods(child, "Expiries", true);
                vector<Real> moneyness = XMLUtils::getChildrenValuesAsDoublesCompact(child, "Moneyness", false);
                if (moneyness.empty()) moneyness = { 1.0 };
                commodityVolMoneyness_[name] = moneyness;
            }
        }
        setCommodityVolNames(names);

        // If present, override DayCounter for _all_ commodity volatilities
        XMLNode* dayCounterNode = XMLUtils::getChildNode(nodeChild, "DayCounter");
        if (dayCounterNode) {
            commodityVolDayCounters_[""] = XMLUtils::getNodeValue(dayCounterNode);
        }

    }

    DLOG("Loaded ScenarioSimMarketParameters");
}

XMLNode* ScenarioSimMarketParameters::toXML(XMLDocument& doc) {

    XMLNode* marketNode = doc.allocNode("Market");

    // currencies
    XMLUtils::addChild(doc, marketNode, "BaseCurrency", baseCcy_);
    XMLUtils::addChildren(doc, marketNode, "Currencies", "Currency", ccys_);

    // benchmark yield curves
    DLOG("Writing benchmaerk yield curves data");
    XMLNode* benchmarkCurvesNode = XMLUtils::addChild(doc, marketNode, "BenchmarkCurves");
    for (Size i = 0; i < yieldCurveNames().size(); ++i) {
        XMLNode* benchmarkCurveNode = XMLUtils::addChild(doc, benchmarkCurvesNode, "BenchmarkCurve");
        XMLUtils::addChild(doc, benchmarkCurveNode, "Currency", yieldCurveCurrencies_[yieldCurveNames()[i]]);
        XMLUtils::addChild(doc, benchmarkCurveNode, "Name", yieldCurveNames()[i]);
    }

    // yield curves
    DLOG("Writing yield curves data");
    XMLNode* yieldCurvesNode = XMLUtils::addChild(doc, marketNode, "YieldCurves");
    
    // Take the keys from the yieldCurveDayCounters_ and yieldCurveTenors_ maps
    set<string> keys;
    for (const auto& kv : yieldCurveTenors_) {
        keys.insert(kv.first);
    }
    for (const auto& kv : yieldCurveDayCounters_) {
        keys.insert(kv.first);
    }
    QL_REQUIRE(keys.count("") > 0, "There is no default yield curve configuration in simulation parameters");
    
    // Add the yield curve configuration nodes
    for (const auto& key : keys) {
        XMLNode* configNode = doc.allocNode("Configuration");
        XMLUtils::addAttribute(doc, configNode, "curve", key);
        if (yieldCurveTenors_.count(key) > 0) {
            XMLUtils::addGenericChildAsList(doc, configNode, "Tenors", yieldCurveTenors_.at(key));
        }
        if (yieldCurveDayCounters_.count(key) > 0) {
            XMLUtils::addChild(doc, configNode, "DayCounter", yieldCurveDayCounters_.at(key));
        }
        if (key == "") {
            XMLUtils::addChild(doc, configNode, "Interpolation", interpolation_);
            XMLUtils::addChild(doc, configNode, "Extrapolation", extrapolate_);
        }
        XMLUtils::appendNode(yieldCurvesNode, configNode);
    }

    // indices
    DLOG("Writing libor indices");
    XMLUtils::addChildren(doc, marketNode, "Indices", "Index", indices());

    // swap indices
    DLOG("Writing swap indices");
    XMLNode* swapIndicesNode = XMLUtils::addChild(doc, marketNode, "SwapIndices");
    for (auto swapIndexInterator : swapIndices_) {
        XMLNode* swapIndexNode = XMLUtils::addChild(doc, swapIndicesNode, "SwapIndex");
        XMLUtils::addChild(doc, swapIndexNode, "Name", swapIndexInterator.first);
        XMLUtils::addChild(doc, swapIndexNode, "DiscountingIndex", swapIndexInterator.second);
    }

    // default curves
    DLOG("Writing default curves");
    XMLNode* defaultCurvesNode = XMLUtils::addChild(doc, marketNode, "DefaultCurves");
    if (!defaultNames().empty()) {
        XMLUtils::addChildren(doc, defaultCurvesNode, "Names", "Name", defaultNames());
        XMLUtils::addGenericChildAsList(doc, defaultCurvesNode, "Tenors", returnTenors(defaultTenors_, ""));
        XMLUtils::addChild(doc, defaultCurvesNode, "SimulateSurvivalProbabilities", survivalProbabilitySimulate_);
        XMLUtils::addChild(doc, defaultCurvesNode, "SimulateRecoveryRates", recoveryRateSimulate_);

        if (defaultCurveDayCounters_.size() > 0) {
            XMLNode* node = XMLUtils::addChild(doc, defaultCurvesNode, "DayCounters");
            for (auto dc : defaultCurveDayCounters_) {
                XMLNode* c = doc.allocNode("DayCounter", dc.second);
                XMLUtils::addAttribute(doc, c, "name", dc.first);
                XMLUtils::appendNode(node, c);
            }
        }

        if (defaultCurveCalendars_.size() > 0) {
            XMLNode* node = XMLUtils::addChild(doc, defaultCurvesNode, "Calendars");
            for (auto dc : defaultCurveCalendars_) {
                XMLNode* c = doc.allocNode("Calendar", dc.second);
                XMLUtils::addAttribute(doc, c, "name", dc.first);
                XMLUtils::appendNode(node, c);
            }
        }
    }

    // equities
    DLOG("Writing equities");
    XMLNode* equitiesNode = XMLUtils::addChild(doc, marketNode, "Equities");
    if (!equityNames().empty()) {
        XMLUtils::addChildren(doc, equitiesNode, "Names", "Name", equityNames());
        XMLUtils::addGenericChildAsList(doc, equitiesNode, "DividendTenors", returnTenors(equityDividendTenors_, ""));
        XMLUtils::addGenericChildAsList(doc, equitiesNode, "ForecastTenors", returnTenors(equityForecastTenors_, ""));
    }

    // swaption volatilities
    DLOG("Writing swaption volatilities");
    XMLNode* swaptionVolatilitiesNode = XMLUtils::addChild(doc, marketNode, "SwaptionVolatilities");
    if (!swapVolCcys().empty()) {
        XMLUtils::addChild(doc, swaptionVolatilitiesNode, "Simulate", swapVolSimulate_);
        XMLUtils::addChild(doc, swaptionVolatilitiesNode, "ReactionToTimeDecay", swapVolDecayMode_);
        XMLUtils::addChildren(doc, swaptionVolatilitiesNode, "Currencies", "Currency", swapVolCcys());
        XMLUtils::addGenericChildAsList(doc, swaptionVolatilitiesNode, "Expiries", swapVolExpiries_);
        XMLUtils::addGenericChildAsList(doc, swaptionVolatilitiesNode, "Terms", swapVolTerms_);        
        if (swapVolIsCube_) {
            XMLNode* swapVolNode = XMLUtils::addChild(doc, swaptionVolatilitiesNode, "Cube");
            XMLUtils::addChild(doc, swapVolNode, "SimulateATMOnly", swapVolSimulateATMOnly_);
            XMLUtils::addGenericChildAsList(doc, swapVolNode, "StrikeSpreads", swapVolStrikeSpreads_);
        }           
        if (swapVolDayCounters_.size() > 0) {
            XMLNode* node = XMLUtils::addChild(doc, swaptionVolatilitiesNode, "DayCounters");
            for (auto dc : swapVolDayCounters_) {
                XMLNode* c = doc.allocNode("DayCounter", dc.second);
                XMLUtils::addAttribute(doc, c, "ccy", dc.first);
                XMLUtils::appendNode(node, c);
            }
        }
    }

    // cap/floor volatilities
    DLOG("Writing cap/floor volatilities");
    XMLNode* capFloorVolatilitiesNode = XMLUtils::addChild(doc, marketNode, "CapFloorVolatilities");
    if (!capFloorVolCcys().empty()) {
        XMLUtils::addChild(doc, capFloorVolatilitiesNode, "Simulate", capFloorVolSimulate_);
        XMLUtils::addChild(doc, capFloorVolatilitiesNode, "ReactionToTimeDecay", capFloorVolDecayMode_);
        XMLUtils::addChildren(doc, capFloorVolatilitiesNode, "Currencies", "Currency", capFloorVolCcys());
        XMLUtils::addGenericChildAsList(doc, capFloorVolatilitiesNode, "Expiries", returnTenors(capFloorVolExpiries_, ""));
        // TODO write other keys
        XMLUtils::addGenericChildAsList(doc, capFloorVolatilitiesNode, "Strikes", capFloorVolStrikes_);
        if (capFloorVolDayCounters_.size() > 0) {
            XMLNode* node = XMLUtils::addChild(doc, capFloorVolatilitiesNode, "DayCounters");
            for (auto dc : capFloorVolDayCounters_) {
                XMLNode* c = doc.allocNode("DayCounter", dc.second);
                XMLUtils::addAttribute(doc, c, "ccy", dc.first);
                XMLUtils::appendNode(node, c);
            }
        }
    }

    // fx volatilities
    DLOG("Writing FX volatilities");
    XMLNode* fxVolatilitiesNode = XMLUtils::addChild(doc, marketNode, "FxVolatilities");
    if (!fxVolCcyPairs().empty()) {
        XMLUtils::addChild(doc, fxVolatilitiesNode, "Simulate", fxVolSimulate_);
        XMLUtils::addChild(doc, fxVolatilitiesNode, "ReactionToTimeDecay", fxVolDecayMode_);
        XMLUtils::addChildren(doc, fxVolatilitiesNode, "CurrencyPairs", "CurrencyPair", fxVolCcyPairs());
        XMLUtils::addGenericChildAsList(doc, fxVolatilitiesNode, "Expiries", fxVolExpiries_);
        if (fxVolDayCounters_.size() > 0) {
            XMLNode* node = XMLUtils::addChild(doc, fxVolatilitiesNode, "DayCounters");
            for (auto dc : fxVolDayCounters_) {
                XMLNode* c = doc.allocNode("DayCounter", dc.second);
                XMLUtils::addAttribute(doc, c, "ccyPair", dc.first);
                XMLUtils::appendNode(node, c);
            }
        }
    }

    // fx rates
    DLOG("Writing FX rates");
    XMLNode* fxRatesNode = XMLUtils::addChild(doc, marketNode, "FxRates");
    XMLUtils::addChildren(doc, fxRatesNode, "CurrencyPairs", "CurrencyPair", fxCcyPairs());

    // eq volatilities
    DLOG("Writing equity volatilities");
    XMLNode* eqVolatilitiesNode = XMLUtils::addChild(doc, marketNode, "EquityVolatilities");
    if (!equityVolNames().empty()) {
        XMLUtils::addChild(doc, eqVolatilitiesNode, "Simulate", equityVolSimulate_);
        XMLUtils::addChild(doc, eqVolatilitiesNode, "ReactionToTimeDecay", equityVolDecayMode_);
        XMLUtils::addChildren(doc, eqVolatilitiesNode, "Names", "Name", equityVolNames());
        XMLUtils::addGenericChildAsList(doc, eqVolatilitiesNode, "Expiries", equityVolExpiries_);
        if (equityIsSurface_) {
            XMLNode* eqSurfaceNode = XMLUtils::addChild(doc, eqVolatilitiesNode, "Surface");
            XMLUtils::addGenericChildAsList(doc, eqSurfaceNode, "Moneyness", equityMoneyness_);
        }
    }

    // additional scenario data currencies
    DLOG("Writing aggregation scenario data currencies");
    if (!additionalScenarioDataCcys_.empty()) {
        XMLUtils::addChildren(doc, marketNode, "AggregationScenarioDataCurrencies", 
            "Currency", additionalScenarioDataCcys_);
    }

    // additional scenario data indices
    DLOG("Writing aggregation scenario data indices");
    if (!additionalScenarioDataIndices_.empty()) {
        XMLUtils::addChildren(doc, marketNode, "AggregationScenarioDataIndices", "Index", 
            additionalScenarioDataIndices_);
    }

    // securities
    DLOG("Writing securities");
    XMLNode* secNode = XMLUtils::addChild(doc, marketNode, "Securities");
    if (!securities().empty()) {
        XMLUtils::addChild(doc, secNode, "Simulate", securitySpreadsSimulate_);
        XMLUtils::addChildren(doc, secNode, "Securities", "Security", securities());
    }

    // base correlations
    DLOG("Writing base correlations");
    XMLNode* bcNode = XMLUtils::addChild(doc, marketNode, "BaseCorrelations");
    if (!baseCorrelationNames().empty()) {
        XMLUtils::addChild(doc, bcNode, "Simulate", baseCorrelationSimulate_);
        XMLUtils::addChildren(doc, bcNode, "IndexNames", "IndexName", baseCorrelationNames());
        XMLUtils::addGenericChildAsList(doc, bcNode, "Terms", baseCorrelationTerms_);
        XMLUtils::addGenericChildAsList(doc, bcNode, "DetachmentPoints", baseCorrelationDetachmentPoints_);
        if (yoyInflationDayCounters_.size() > 0) {
            XMLNode* node = XMLUtils::addChild(doc, bcNode, "DayCounters");
            for (auto dc : baseCorrelationDayCounters_) {
                XMLNode* c = doc.allocNode("DayCounter", dc.second);
                XMLUtils::addAttribute(doc, c, "name", dc.first);
                XMLUtils::appendNode(node, c);
            }
        }
    }

    // inflation indices
    DLOG("Writing inflation indices");
    XMLNode* cpiNode = XMLUtils::addChild(doc, marketNode, "CpiInflationIndices");
    if (!cpiIndices().empty()) {
        XMLUtils::addChildren(doc, cpiNode, "CpiIndices", "Index", cpiIndices());
    }

    // zero inflation
    DLOG("Writing zero inflation");
    XMLNode* zeroNode = XMLUtils::addChild(doc, marketNode, "ZeroInflationIndexCurves");
    if (!zeroInflationIndices().empty()) {
        XMLUtils::addChildren(doc, zeroNode, "Names", "Name", zeroInflationIndices());
        XMLUtils::addGenericChildAsList(doc, zeroNode, "Tenors", returnTenors(zeroInflationTenors_, ""));
        if (zeroInflationDayCounters_.size() > 0) {
            XMLNode* node = XMLUtils::addChild(doc, zeroNode, "DayCounters");
            for (auto dc : zeroInflationDayCounters_) {
                XMLNode* c = doc.allocNode("DayCounter", dc.second);
                XMLUtils::addAttribute(doc, c, "name", dc.first);
                XMLUtils::appendNode(node, c);
            }
        }
    }

    // yoy inflation
    DLOG("Writing year-on-year inflation");
    XMLNode* yoyNode = XMLUtils::addChild(doc, marketNode, "YYInflationIndexCurves");
    if (!yoyInflationIndices().empty()) {
        XMLUtils::addChildren(doc, yoyNode, "Names", "Name", yoyInflationIndices());
        XMLUtils::addGenericChildAsList(doc, yoyNode, "Tenors", returnTenors(yoyInflationTenors_, ""));

        if (yoyInflationDayCounters_.size() > 0) {
            XMLNode* node = XMLUtils::addChild(doc, yoyNode, "DayCounters");
            for (auto dc : yoyInflationDayCounters_) {
                XMLNode* c = doc.allocNode("DayCounter", dc.second);
                XMLUtils::addAttribute(doc, c, "name", dc.first);
                XMLUtils::appendNode(node, c);
            }
        }
    }

    // Commodity price curves
    DLOG("Writing commodity price curves");
    XMLNode* commodityPriceNode = XMLUtils::addChild(doc, marketNode, "Commodities");
    if (!commodityNames().empty()) {
        XMLUtils::addChild(doc, commodityPriceNode, "Simulate", commodityCurveSimulate_);
        XMLUtils::addChildren(doc, commodityPriceNode, "Names", "Name", commodityNames());
        XMLUtils::addGenericChildAsList(doc, commodityPriceNode, "Tenors", commodityCurveTenors_.at(""));
        XMLUtils::addChild(doc, commodityPriceNode, "DayCounter", commodityCurveDayCounters_.at(""));
    }

    // Commodity volatilities
    DLOG("Writing commodity volatilities");
    XMLNode* commodityVolatilitiesNode = XMLUtils::addChild(doc, marketNode, "CommodityVolatilities");
    if (!commodityVolNames().empty()) {
        XMLUtils::addChild(doc, commodityVolatilitiesNode, "Simulate", commodityVolSimulate_);
        XMLUtils::addChild(doc, commodityVolatilitiesNode, "ReactionToTimeDecay", commodityVolDecayMode_);
        XMLNode* namesNode = XMLUtils::addChild(doc, commodityVolatilitiesNode, "Names");
        for (const auto& name : commodityVolNames()) {
            XMLNode* nameNode = doc.allocNode("Name");
            XMLUtils::addAttribute(doc, nameNode, "id", name);
            XMLUtils::addGenericChildAsList(doc, nameNode, "Expiries", commodityVolExpiries_[name]);
            XMLUtils::addGenericChildAsList(doc, nameNode, "Moneyness", commodityVolMoneyness_[name]);
            XMLUtils::appendNode(namesNode, nameNode);
        }
        XMLUtils::addChild(doc, commodityVolatilitiesNode, "DayCounter", commodityVolDayCounters_.at(""));
    }

    return marketNode;
}
} // namespace analytics
} // namespace ore
