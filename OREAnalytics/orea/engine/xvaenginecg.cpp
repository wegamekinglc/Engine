/*
 Copyright (C) 2023 Quaternion Risk Management Ltd
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

#include <orea/app/reportwriter.hpp>
#include <orea/cube/npvcube.hpp>
#include <orea/cube/npvsensicube.hpp>
#include <orea/cube/sensicube.hpp>
#include <orea/cube/sensitivitycube.hpp>
#include <orea/engine/sensitivitycubestream.hpp>
#include <orea/engine/xvaenginecg.hpp>
#include <orea/scenario/deltascenariofactory.hpp>

#include <ored/report/inmemoryreport.hpp>
#include <ored/scripting/engines/scriptedinstrumentpricingenginecg.hpp>
#include <ored/utilities/to_string.hpp>

#include <qle/ad/backwardderivatives.hpp>
#include <qle/ad/forwardderivatives.hpp>
#include <qle/ad/forwardevaluation.hpp>
#include <qle/ad/ssaform.hpp>
#include <qle/methods/multipathvariategenerator.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/weighted_sum.hpp>
#include <boost/timer/timer.hpp>

namespace ore {
namespace analytics {

namespace {
std::size_t numberOfStochasticRvs(const std::vector<RandomVariable>& v) {
    return std::count_if(v.begin(), v.end(),
                         [](const RandomVariable& r) { return r.initialised() && !r.deterministic(); });
}
} // namespace

XvaEngineCG::XvaEngineCG(const Size nThreads, const Date& asof, const QuantLib::ext::shared_ptr<ore::data::Loader>& loader,
                         const QuantLib::ext::shared_ptr<ore::data::CurveConfigurations>& curveConfigs,
                         const QuantLib::ext::shared_ptr<ore::data::TodaysMarketParameters>& todaysMarketParams,
                         const QuantLib::ext::shared_ptr<ore::analytics::ScenarioSimMarketParameters>& simMarketData,
                         const QuantLib::ext::shared_ptr<ore::data::EngineData>& engineData,
                         const QuantLib::ext::shared_ptr<ore::analytics::CrossAssetModelData>& crossAssetModelData,
                         const QuantLib::ext::shared_ptr<ore::analytics::ScenarioGeneratorData>& scenarioGeneratorData,
                         const QuantLib::ext::shared_ptr<ore::data::Portfolio>& portfolio, const string& marketConfiguration,
                         const string& marketConfigurationInCcy,
                         const QuantLib::ext::shared_ptr<ore::analytics::SensitivityScenarioData>& sensitivityData,
                         const QuantLib::ext::shared_ptr<ReferenceDataManager>& referenceData,
                         const IborFallbackConfig& iborFallbackConfig,
			 const bool bumpCvaSensis, const bool continueOnCalibrationError,
                         const bool continueOnError, const std::string& context)
    : asof_(asof), loader_(loader), curveConfigs_(curveConfigs),
      todaysMarketParams_(todaysMarketParams), simMarketData_(simMarketData), engineData_(engineData),
      crossAssetModelData_(crossAssetModelData), scenarioGeneratorData_(scenarioGeneratorData), portfolio_(portfolio),
      marketConfiguration_(marketConfiguration), marketConfigurationInCcy_(marketConfigurationInCcy),
      sensitivityData_(sensitivityData), referenceData_(referenceData), iborFallbackConfig_(iborFallbackConfig),
      bumpCvaSensis_(bumpCvaSensis), continueOnCalibrationError_(continueOnCalibrationError), continueOnError_(continueOnError), context_(context) {

    // Just for performance testing, duplicate the trades in input portfolio as specified by env var N

    // portfolio_ = QuantLib::ext::make_shared<Portfolio>();
    // std::string pfxml = portfolio->toXMLString();
    // for (Size i = 0; i < atoi(getenv("N")); ++i) {
    //     auto p = QuantLib::ext::make_shared<Portfolio>();
    //     p->fromXMLString(pfxml);
    //     for (auto const& [id, t] : p->trades()) {
    //         t->id() += "_" + std::to_string(i + 1);
    //         portfolio_->add(t);
    //     }
    // }

    // Start Engine

    LOG("XvaEngineCG: started");
    boost::timer::cpu_timer timer;

    // Build T0 market

    LOG("XvaEngineCG: build init market");

    initMarket_ = QuantLib::ext::make_shared<ore::data::TodaysMarket>(asof_, todaysMarketParams_, loader_, curveConfigs_,
                                                              continueOnError_, true, true, referenceData_, false,
                                                              iborFallbackConfig_, false, true);

    boost::timer::nanosecond_type timing1 = timer.elapsed().wall;

    // Build sim market

    LOG("XvaEngineCG: build sim market");

    // note: take "use spreaded term structures" from sensitivity config instead of hardcoding to true here?
    simMarket_ = QuantLib::ext::make_shared<ore::analytics::ScenarioSimMarket>(
        initMarket_, simMarketData_, marketConfiguration_, *curveConfigs_, *todaysMarketParams_, continueOnError_, true,
        false, false, iborFallbackConfig_, true);

    boost::timer::nanosecond_type timing2 = timer.elapsed().wall;

    // Set up cam builder against sim market

    LOG("XvaEngineCG: build cam model builder");

    // note: sim market has one config only, no in-ccy config to calibrate IR components
    camBuilder_ = QuantLib::ext::make_shared<CrossAssetModelBuilder>(
        simMarket_, crossAssetModelData_, marketConfigurationInCcy_, marketConfiguration_, marketConfiguration_,
        marketConfiguration_, marketConfiguration_, marketConfiguration_, false, continueOnCalibrationError_,
        std::string(), SalvagingAlgorithm::Spectral, "xva engine cg - cam builder");

    // Set up gaussian cam cg model

    LOG("XvaEngineCG: build cam cg model");

    QL_REQUIRE(
        crossAssetModelData_->discretization() == CrossAssetModel::Discretization::Euler,
        "XvaEngineCG: cam is required to use discretization 'Euler', please update simulation parameters accordingly.");

    std::vector<std::string> currencies;                                                   // from cam
    std::vector<Handle<YieldTermStructure>> curves;                                        // from cam
    std::vector<Handle<Quote>> fxSpots;                                                    // from cam
    std::vector<std::pair<std::string, QuantLib::ext::shared_ptr<InterestRateIndex>>> irIndices;   // from trade building
    std::vector<std::pair<std::string, QuantLib::ext::shared_ptr<ZeroInflationIndex>>> infIndices; // from trade building
    std::vector<std::string> indices;                                                      // from trade building
    std::vector<std::string> indexCurrencies;                                              // from trade building

    // note: for the PoC we populate the containers with hardcoded values ... temp hack ...
    currencies.push_back("EUR");
    curves.push_back(camBuilder_->model()->irModel(0)->termStructure());
    irIndices.push_back(std::make_pair("EUR-EURIBOR-6M", *simMarket_->iborIndex("EUR-EURIBOR-6M")));

    // note: these must be fine enough for Euler, e.g. weekly over the whole simulation period
    std::set<Date> simulationDates(scenarioGeneratorData_->getGrid()->dates().begin(),
                                   scenarioGeneratorData_->getGrid()->dates().end());
    // note: this should be added to CrossAssetModelData
    Size timeStepsPerYear = 1;

    // note: projectedStateProcessIndices can be removed from GaussianCamCG constructor most probably?
    model_ = QuantLib::ext::make_shared<GaussianCamCG>(camBuilder_->model(), scenarioGeneratorData_->samples(), currencies,
                                               curves, fxSpots, irIndices, infIndices, indices, indexCurrencies,
                                               simulationDates, timeStepsPerYear, iborFallbackConfig,
                                               std::vector<Size>(), std::vector<std::string>(), true);
    model_->calculate();
    boost::timer::nanosecond_type timing3 = timer.elapsed().wall;

    // Build trades against global cg cam model

    LOG("XvaEngineCG: build trades against global cam cg model");

    auto edCopy = QuantLib::ext::make_shared<EngineData>(*engineData_);
    edCopy->globalParameters()["GenerateAdditionalResults"] = "false";
    edCopy->globalParameters()["RunType"] = "NPV";
    map<MarketContext, string> configurations;
    configurations[MarketContext::irCalibration] = marketConfigurationInCcy_;
    configurations[MarketContext::fxCalibration] = marketConfiguration_;
    configurations[MarketContext::pricing] = marketConfiguration_;
    auto factory =
        QuantLib::ext::make_shared<EngineFactory>(edCopy, simMarket_, configurations, referenceData_, iborFallbackConfig_,
                                          EngineBuilderFactory::instance().generateAmcCgEngineBuilders(
                                              model_, scenarioGeneratorData_->getGrid()->dates()),
                                          true);

    portfolio_->build(factory, "xva engine cg", true);

    boost::timer::nanosecond_type timing4 = timer.elapsed().wall;

    // Build computation graph for all trades ("part B") and
    // - store npv, amc npv nodes

    LOG("XvaEngineCG: build computation graph for all trades");

    std::vector<std::vector<std::size_t>> amcNpvNodes; // includes time zero npv

    auto g = model_->computationGraph();

    for (auto const& [id, trade] : portfolio_->trades()) {
        auto qlInstr = QuantLib::ext::dynamic_pointer_cast<ScriptedInstrument>(trade->instrument()->qlInstrument());
        QL_REQUIRE(qlInstr, "XvaEngineCG: expeced trade to provide ScriptedInstrument, trade '" << id << "' does not.");
        auto engine = QuantLib::ext::dynamic_pointer_cast<ScriptedInstrumentPricingEngineCG>(qlInstr->pricingEngine());
        QL_REQUIRE(engine, "XvaEngineCG: expected to get ScriptedInstrumentPricingEngineCG, trade '"
                               << id << "' has a different engine.");
        g->startRedBlock();
        engine->buildComputationGraph();
        std::vector<std::size_t> tmp;
        tmp.push_back(cg_add(*g, cg_const(*g, 0.0), g->variable(engine->npvName() + "_0")));
        for (std::size_t i = 0; i < simulationDates.size(); ++i) {
            tmp.push_back(cg_add(*g, cg_const(*g, 0.0), g->variable("_AMC_NPV_" + std::to_string(i))));
        }
        amcNpvNodes.push_back(tmp);
        g->endRedBlock();
    }

    boost::timer::nanosecond_type timing5 = timer.elapsed().wall;

    // Add nodes that sum the exposure over trades, both pathwise and conditional expectations
    // This constitutes part C of the computation graph spanning "trade m range end ... lastExposureNode"
    // - pfPathExposureNodes: path amc sim values, aggregated over trades
    // - pfExposureNodes:     the corresponding conditional expectations

    std::vector<std::size_t> pfPathExposureNodes, pfExposureNodes;
    for (Size i = 0; i < simulationDates.size() + 1; ++i) {
        std::size_t sumNode = cg_const(*g, 0.0);
        for (auto const& v : amcNpvNodes) {
            sumNode = cg_add(*g, sumNode, v[i]);
        }
        pfPathExposureNodes.push_back(sumNode);
        pfExposureNodes.push_back(
            model_->npv(sumNode, i == 0 ? model_->referenceDate() : *std::next(simulationDates.begin(), i - 1),
                        cg_const(*g, 1.0), boost::none, ComputationGraph::nan, ComputationGraph::nan));
    }

    boost::timer::nanosecond_type timing6 = timer.elapsed().wall;

    // Add post processor
    // This constitues part D of the computation graph from lastExposureNode ... g->size()
    // The cvaNode is the ultimate result w.r.t. which we want to compute sensitivities

    // note: very simplified calculation, for testing, just multiply the EPE on each date by fixed default prob
    auto defaultCurve = simMarket_->defaultCurve("BANK")->curve();
    model_->registerWith(defaultCurve);
    Size cvaNode = cg_const(*g, 0.0);
    for (Size i = 0; i < simulationDates.size(); ++i) {
        Date d = i == 0 ? model_->referenceDate() : *std::next(simulationDates.begin(), i - 1);
        Date e = *std::next(simulationDates.begin(), i);
        std::size_t defaultProb =
            addModelParameter(*g, model_->modelParameterFunctors(), "__defaultprob_" + std::to_string(i),
                              [defaultCurve, d, e]() { return defaultCurve->defaultProbability(d, e); });
        cvaNode = cg_add(*g, cvaNode, cg_mult(*g, defaultProb, cg_max(*g, pfExposureNodes[i], cg_const(*g, 0.0))));
    }

    boost::timer::nanosecond_type timing7 = timer.elapsed().wall;

    LOG("XvaEngineCG: graph building complete, size is " << g->size());
    LOG("XvaEngineCG: got " << g->redBlockDependencies().size() << " red block dependencies.");
    std::size_t sumRedNodes = 0;
    for (auto const& r : g->redBlockRanges()) {
        DLOG("XvaEngineCG: red block range " << r.first << " ... " << r.second);
        sumRedNodes += r.second - r.first;
    }

    // Create values and derivatives containers

    std::vector<RandomVariable> values(g->size(), RandomVariable(model_->size(), 0.0));
    std::vector<RandomVariable> derivatives(g->size(), RandomVariable(model_->size(), 0.0));

    // Populate random variates

    populateRandomVariates(values);
    boost::timer::nanosecond_type timing8 = timer.elapsed().wall;

    // Populate constants and model parameters

    baseModelParams_ = model_->modelParameters();
    populateConstants(values);
    populateModelParameters(values, baseModelParams_);
    boost::timer::nanosecond_type timing9 = timer.elapsed().wall;

    std::size_t rvMemMax = numberOfStochasticRvs(values) + numberOfStochasticRvs(derivatives);

    // Do a forward evaluation, keep the following values nodes
    // - constants
    // - model parameters
    // - values needed for derivatives (except in red blocks, by their definition)
    // - red block dependencies
    // - the random variates for bump sensis
    // - the pfExposureNodes to dump out the epe profile

    LOG("XvaEngineCG: do forward evaluation");

    Real eps = 0.0; // smoothing parameter for indicator functions
    ops_ = getRandomVariableOps(model_->size(), 4, QuantLib::LsmBasisSystem::Monomial, bumpCvaSensis_ ? eps : 0.0,
                                Null<Real>()); // todo set regression variance cutoff
    grads_ = getRandomVariableGradients(model_->size(), 4, QuantLib::LsmBasisSystem::Monomial, eps);
    opNodeRequirements_ = getRandomVariableOpNodeRequirements();

    std::vector<bool> keepNodes(g->size(), false);

    for (auto const& c : g->constants()) {
        keepNodes[c.second] = true;
    }

    for (auto const& [n, v] : baseModelParams_) {
        keepNodes[n] = true;
    }

    for (auto const n : g->redBlockDependencies()) {
        keepNodes[n] = true;
    }

    if (bumpCvaSensis_) {
        for (auto const& rv : model_->randomVariates())
            for (auto const& v : rv)
                keepNodes[v] = true;
    }

    for (auto const& n : pfExposureNodes) {
        keepNodes[n] = true;
    }

    forwardEvaluation(*g, values, ops_, RandomVariable::deleter, true, opNodeRequirements_, keepNodes);

    boost::timer::nanosecond_type timing10 = timer.elapsed().wall;

    // Write epe / ene profile out

    {
        epeReport_ = QuantLib::ext::make_shared<InMemoryReport>();
        epeReport_->addColumn("Date", Date()).addColumn("EPE", double(), 4).addColumn("ENE", double(), 4);

        for (Size i = 0; i < simulationDates.size() + 1; ++i) {
            epeReport_->next();
            epeReport_->add(i == 0 ? model_->referenceDate() : *std::next(simulationDates.begin(), i - 1))
                .add(expectation(max(values[pfExposureNodes[i]], RandomVariable(model_->size(), 0.0))).at(0))
                .add(expectation(max(-values[pfExposureNodes[i]], RandomVariable(model_->size(), 0.0))).at(0));
        }
        epeReport_->end();
        epeReport_->toFile("Output/xvacg-exposure.csv");
    }

    Real cva = expectation(values[cvaNode]).at(0);
    LOG("XvaEngineCG: Calcuated CVA (node " << cvaNode << ") = " << cva);

    rvMemMax = std::max(rvMemMax, numberOfStochasticRvs(values) + numberOfStochasticRvs(derivatives));

    // Do backward derivatives run

    boost::timer::nanosecond_type timing11 = timer.elapsed().wall;

    std::vector<double> modelParamDerivatives(baseModelParams_.size());

    if (!bumpCvaSensis_) {

        LOG("XvaEngineCG: run backward derivatives");

        derivatives[cvaNode] = RandomVariable(model_->size(), 1.0);

        std::vector<bool> keepNodesDerivatives(g->size(), false);

        for (auto const& [n, _] : baseModelParams_)
            keepNodesDerivatives[n] = true;

        // backward derivatives run

        backwardDerivatives(*g, values, derivatives, grads_, RandomVariable::deleter, keepNodesDerivatives, ops_,
                            opNodeRequirements_, keepNodes, RandomVariableOpCode::ConditionalExpectation,
                            ops_[RandomVariableOpCode::ConditionalExpectation]);

        // read model param derivatives

        Size i = 0;
        for (auto const& [n, v] : baseModelParams_) {
            modelParamDerivatives[i++] = expectation(derivatives[n]).at(0);
        }

        // get mem consumption

        rvMemMax = std::max(rvMemMax, numberOfStochasticRvs(values) + numberOfStochasticRvs(derivatives));

        LOG("XvaEngineCG: got " << modelParamDerivatives.size()
                                << " model parameter derivatives from run backward derivatives");

        timing11 = timer.elapsed().wall;

        // Delete values and derivatives vectors, they are not needed from this point on
        // except we are doing a full revaluation!

        values.clear();
        derivatives.clear();
    }

    // 13 generate sensitivity scenarios

    LOG("XvaEngineCG: running sensi scenarios");

    sensiScenarioGenerator_ = QuantLib::ext::make_shared<SensitivityScenarioGenerator>(
        sensitivityData_, simMarket_->baseScenario(), simMarketData_, simMarket_,
        QuantLib::ext::make_shared<DeltaScenarioFactory>(simMarket_->baseScenario()), false, std::string(), continueOnError,
        simMarket_->baseScenarioAbsolute());

    simMarket_->scenarioGenerator() = sensiScenarioGenerator_;

    auto resultCube = QuantLib::ext::make_shared<DoublePrecisionSensiCube>(std::set<std::string>{"CVA"}, asof_,
                                                                   sensiScenarioGenerator_->samples());
    resultCube->setT0(cva, 0, 0);

    model_->alwaysForwardNotifications();

    Size activeScenarios = 0;
    for (Size sample = 0; sample < resultCube->samples(); ++sample) {

        // update sim market to next scenario

        simMarket_->preUpdate();
        simMarket_->updateScenario(asof_);
        simMarket_->postUpdate(asof_, false);

        // recalibrate the model

        camBuilder_->recalibrate();

        Real sensi = 0.0;

        // calculate sensi if model was notified of a change

        if (!model_->isCalculated()) {

            model_->calculate();
            ++activeScenarios;

            if (!bumpCvaSensis_) {

                // calcuate CVA sensi using ad derivatives

                auto modelParameters = model_->modelParameters();
                Size i = 0;
                boost::accumulators::accumulator_set<
                    double, boost::accumulators::stats<boost::accumulators::tag::weighted_sum>, double>
                    acc;
                for (auto const& [n, v0] : baseModelParams_) {
                    Real v1 = modelParameters[i].second;
                    acc(modelParamDerivatives[i], boost::accumulators::weight = (v1 - v0));
                    ++i;
                }
                sensi = boost::accumulators::weighted_sum(acc);

            } else {

                // calcuate CVA sensi doing full recalc of CVA

                populateModelParameters(values, model_->modelParameters());
                forwardEvaluation(*g, values, ops_, RandomVariable::deleter, true, opNodeRequirements_, keepNodes);
                sensi = expectation(values[cvaNode]).at(0) - cva;

            }
        }

        // set result in cube

        resultCube->set(cva + sensi, 0, 0, sample, 0);
    }

    boost::timer::nanosecond_type timing12 = timer.elapsed().wall;

    LOG("XvaEngineCG: finished running " << resultCube->samples() << " sensi scenarios, thereof " << activeScenarios
                                         << " active.");

    // write out sensi report

    {
        sensiReport_ = QuantLib::ext::make_shared<InMemoryReport>();
        auto sensiCube = QuantLib::ext::make_shared<SensitivityCube>(
            resultCube, sensiScenarioGenerator_->scenarioDescriptions(), sensiScenarioGenerator_->shiftSizes(),
            sensiScenarioGenerator_->shiftSizes(), sensiScenarioGenerator_->shiftSchemes());
        auto sensiStream = QuantLib::ext::make_shared<SensitivityCubeStream>(sensiCube, simMarketData_->baseCcy());
        ReportWriter().writeScenarioReport(*sensiReport_, {sensiCube}, 0.0);
        sensiReport_->toFile("Output/xvacg-cva-sensi-scenario.csv");
    }

    // Output statistics

    LOG("XvaEngineCG: graph size               : " << g->size());
    LOG("XvaEngineCG: red nodes                : " << sumRedNodes);
    LOG("XvaEngineCG: red node dependendices   : " << g->redBlockDependencies().size());
    LOG("XvaEngineCG: Peak mem usage           : " << ore::data::os::getPeakMemoryUsageBytes() / 1024 / 1024 << " MB");
    LOG("XvaEngineCG: Peak theoretical rv mem  : " << static_cast<double>(rvMemMax) / 1024 / 1024 * 8 * model_->size()
                                                   << " MB");
    LOG("XvaEngineCG: T0 market build          : " << std::fixed << std::setprecision(1) << timing1 / 1E6 << " ms");
    LOG("XvaEngineCG: Sim market build         : " << std::fixed << std::setprecision(1) << (timing2 - timing1) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Part A CG build          : " << std::fixed << std::setprecision(1) << (timing3 - timing2) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Portfolio build          : " << std::fixed << std::setprecision(1) << (timing4 - timing3) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Part B CG build          : " << std::fixed << std::setprecision(1) << (timing5 - timing4) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Part C CG build          : " << std::fixed << std::setprecision(1) << (timing6 - timing5) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Part D CG build          : " << std::fixed << std::setprecision(1) << (timing7 - timing6) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: RV gen                   : " << std::fixed << std::setprecision(1) << (timing8 - timing7) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Const and Model params   : " << std::fixed << std::setprecision(1) << (timing9 - timing8) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Forward eval             : " << std::fixed << std::setprecision(1) << (timing10 - timing9) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Backward deriv           : " << std::fixed << std::setprecision(1) << (timing11 - timing10) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: Sensi Cube Gen           : " << std::fixed << std::setprecision(1) << (timing12 - timing11) / 1E6
                                                   << " ms");
    LOG("XvaEngineCG: total                    : " << std::fixed << std::setprecision(1) << timing12 / 1E6 << " ms");
}

void XvaEngineCG::populateRandomVariates(std::vector<RandomVariable>& values) const {

    DLOG("XvaEngineCG: populate random variates");

    auto const& rv = model_->randomVariates();
    if (!rv.empty()) {
        auto gen = makeMultiPathVariateGenerator(scenarioGeneratorData_->sequenceType(), rv.size(), rv.front().size(),
                                                 scenarioGeneratorData_->seed(), scenarioGeneratorData_->ordering(),
                                                 scenarioGeneratorData_->directionIntegers());
        for (Size path = 0; path < model_->size(); ++path) {
            auto p = gen->next();
            for (Size j = 0; j < rv.front().size(); ++j) {
                for (Size k = 0; k < rv.size(); ++k) {
                    values[rv[k][j]].set(path, p.value[j][k]);
                }
            }
        }
        DLOG("XvaEngineCG: generated rvs for " << rv.size() << " underlyings and " << rv.front().size()
                                               << " time steps.");
    }
}

void XvaEngineCG::populateConstants(std::vector<RandomVariable>& values) const {

    DLOG("XvaEngineCG: populate constants");

    auto g = model_->computationGraph();
    for (auto const& c : g->constants()) {
        values[c.second] = RandomVariable(model_->size(), c.first);
    }

    DLOG("XvaEngineCG: set " << g->constants().size() << " constants");
}

void XvaEngineCG::populateModelParameters(std::vector<RandomVariable>& values,
                                          const std::vector<std::pair<std::size_t, double>>& modelParameters) const {

    DLOG("XvaEngineCG: populate model parameters");

    for (auto const& [n, v] : modelParameters) {
        values[n] = RandomVariable(model_->size(), v);
    }

    DLOG("XvaEngineCG: set " << modelParameters.size() << " model parameters.");
}

} // namespace analytics
} // namespace ore
