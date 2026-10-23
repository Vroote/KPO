#pragma once

#include <istream>
#include <ostream>

#include <boost/di.hpp>

#include "Beginner_policy.h"
#include "Equipment_factory.h"
#include "Fleet.h"
#include "Fleet_reports.h"
#include "Inspection_service_interface.h"
#include "Intake_service.h"
#include "ServiceCenter_service.h"

inline auto makeInjector(std::istream &in, std::ostream &out) {
    namespace di = boost::di;
    return di::make_injector(di::bind<IInspectionService>().to<ServiceCenter>().in(di::singleton),
                             di::bind<Fleet>().in(di::singleton), di::bind<VehicleFactory>().in(di::singleton),
                             di::bind<ItemFactory>().in(di::singleton), di::bind<BeginnerPolicy>().in(di::singleton),
                             di::bind<IntakeService>().in(di::singleton), di::bind<FleetReports>().in(di::singleton),
                             di::bind<std::istream>().to(in), di::bind<std::ostream>().to(out));
}
