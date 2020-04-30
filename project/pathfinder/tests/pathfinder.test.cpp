#include "gtest/gtest.h"
#include "pathfinder.hpp"
#include "planetScript.hpp"
#include "planetScriptSimple.hpp"



struct pathfinder_tests : public testing::Test
{};


TEST_F(pathfinder_tests, circularOrbits)
{
	using namespace Pathfinder;

	auto scripts = std::vector{
		std::make_shared<PlanetScript::PlanetScriptSimple>(1.327E+20, 0., 0., 0.),
		std::make_shared<PlanetScript::PlanetScriptSimple>(3.986E+14, 149.6E+9, 31.6E+6, 0.),
		std::make_shared<PlanetScript::PlanetScriptSimple>(4.282E+13, 227.9E+9, 59.4E+6, 0.776)
	};

	auto A = std::make_unique<NodeDeparture>();
	auto B = std::make_unique<NodeArrival>();
	A->ParkingRadius = 6.6e+6;
	B->ParkingRadius = 3.8e+6;
	A->SphereRadius = 2.6e+8;
	B->SphereRadius = 1.3e+8;
	A->ImpulseLimit = 7000;
	B->ImpulseLimit = 3000;
	A->Script = scripts[1];
	B->Script = scripts[2];

	auto mission = Mission();
	mission.GM = scripts[0]->GetGM(0);
	mission.normalFlyPeriodFactor = 1;
	mission.points_f0 = 60;
	mission.timeStep = 3600. * 24 * 15;
	mission.timeTol = 3600. * 24;
	mission.t0 = 0;
	mission.nodes.push_back(std::move(A));
	mission.nodes.push_back(std::move(B));

	auto solver = PathFinder(std::move(mission));
	auto paths = solver.FirstApprox();

	auto links = std::map<FReal, Link::Link>();
	for (auto& path : paths)
	{
		links[path.totalImpulse] = path.chain[0].link;
	}
	ASSERT_GE(paths.size(), 1);

	auto& [v_min, top] = *links.begin();
	EXPECT_NEAR(v_min , 5486, 10);
	EXPECT_NEAR(top.v0, 32726, 10);
	EXPECT_NEAR(top.v1, 21482, 10);
	EXPECT_NEAR(top.t1, 2.23e+7, 0.1e+7);
}

TEST_F(pathfinder_tests, realPlanets)
{
	using namespace Pathfinder;
	// this date is a mid of a E-M launch window
	constexpr auto startDate = "2011-11-26, 12:00:00 TDB";

	auto scripts = std::vector{
		std::make_shared<PlanetScript::PlanetScript>(PlanetScript::EPlanet::eSun  , startDate),
		std::make_shared<PlanetScript::PlanetScript>(PlanetScript::EPlanet::eEarth, startDate),
		std::make_shared<PlanetScript::PlanetScript>(PlanetScript::EPlanet::eMars , startDate)
	};

	auto A = std::make_unique<NodeDeparture>();
	auto B = std::make_unique<NodeArrival>();
	A->ParkingRadius = 6.6e+6;
	B->ParkingRadius = 3.8e+6;
	A->SphereRadius = 2.6e+8;
	B->SphereRadius = 1.3e+8;
	A->ImpulseLimit = 7000;
	B->ImpulseLimit = 7000;
	A->Script = scripts[1];
	B->Script = scripts[2];

	auto mission = Mission();
	mission.GM = scripts[0]->GetGM(0);
	mission.normalFlyPeriodFactor = 2;
	mission.points_f0 = 360;
	mission.timeStep = 3600. * 24 * 15;
	mission.timeTol = 3600. * 24;
	mission.t0 = 0;
	mission.nodes.push_back(std::move(A));
	mission.nodes.push_back(std::move(B));

	auto solver = PathFinder(std::move(mission));
	auto paths  = solver.FirstApprox();

	auto links = std::map<FReal, Link::Link>();
	for (auto& path : paths)
	{
		links[path.totalImpulse] = path.chain[0].link;
	}
	ASSERT_GE(paths.size(), 1);

	auto& [v_min, top] = *links.begin();
	EXPECT_LT(v_min, 6150);
	EXPECT_NEAR(top.v0, 33141, 2e+2);
	EXPECT_NEAR(top.v1, 21194, 2e+2);
	EXPECT_NEAR(top.t1, 2.19e+7, 0.2e+7);
}


TEST_F(pathfinder_tests, EVJ)
{
	using namespace Pathfinder;
	// this date is a mid of a E-M launch window
	constexpr auto startDate = "2021-10-11, 12:00:00 TDB";

	auto scripts = std::vector{
		std::make_shared<PlanetScript::PlanetScript>(PlanetScript::EPlanet::eSun   , startDate),
		std::make_shared<PlanetScript::PlanetScript>(PlanetScript::EPlanet::eVenus , startDate),
		std::make_shared<PlanetScript::PlanetScript>(PlanetScript::EPlanet::eEarth , startDate),
		std::make_shared<PlanetScript::PlanetScript>(PlanetScript::EPlanet::eJupter, startDate)
	};

	auto E = std::make_unique<NodeDeparture>();
	{
		E->ParkingRadius = 6.6e+6;
		E->SphereRadius = 2.6e+8;
		E->ImpulseLimit = 7000;
		E->Script = scripts[2];
	}
	
	auto V = std::make_unique<NodeFlyBy>();
	{
		V->MismatchLimit = 1e+3;
		V->SphereRadius = 1.7e+8;
		V->PlanetRadius = 6e+6;
		V->Script = scripts[1];
	}

	auto J = std::make_unique<NodeArrival>();
	{
		J->ParkingRadius = 90e+6;
		J->SphereRadius = 2.4e+10;
		J->ImpulseLimit = 10e+3;
		J->Script = scripts[3];
	}

	auto mission = Mission();
	mission.GM = scripts[0]->GetGM(0);
	mission.normalFlyPeriodFactor = 1;
	mission.points_f0 = 360;
	mission.timeStep = 3600. * 24 * 15;
	mission.timeTol = 3600. * 24;
	mission.t0 = 0;
	mission.nodes.push_back(std::move(E));
	mission.nodes.push_back(std::move(V));
	mission.nodes.push_back(std::move(J));

	auto solver = PathFinder(std::move(mission));
	/*auto paths  = solver.FirstApprox();

	auto links = std::map<FReal, Link::Link>();
	for (auto& path : paths)
	{
		links[path.totalImpulse] = path.chain[1].link;
	}
	ASSERT_GE(paths.size(), 1);

	auto& [v_min, top] = *links.begin();
	EXPECT_LT(v_min, 6150);
	EXPECT_NEAR(top.t1, 72576000, 3600 * 24 * 31);*/
}
