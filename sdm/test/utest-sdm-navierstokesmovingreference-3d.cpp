// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the ElementCaches of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::sdm"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/List.hpp"

#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"

#include "sdm/navierstokesmovingreference/Convection3D.hpp"
#include "sdm/navierstokes/Diffusion3D.hpp"
#include "sdm/navierstokesmovingreference/Source3D.hpp"
//#include "sdm/navierstokesmovingreference/SourceCentrifugal2D.hpp"

using namespace cf3;
using namespace cf3::math;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::solver;
using namespace cf3::sdm;

struct sdm_MPITests_Fixture
{
  /// common setup for each test case
  sdm_MPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~sdm_MPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

//struct Datastr
//{
//    RealVector4 solution;
//    RealVector2 coord;
//    Eigen::Matrix<Real, 5, 3> grad_solution;
//};

//void flux_diff(Datastr d1, RealVector2 unit_normal, Real mu_v, RealVector4& F_Dn)
//{
//    Real mu=1; // moet optie van gemaakt worden
//    Real kappa=1; //moet nog een optie van gemaakt worden
//    Real R=287.05; // optie van maken
//    Real gamma=1.4; // optie van maken

//    Real cv = R/(gamma-1);

//    RealVector4 f_d, g_d;
//    Eigen::Matrix<RealVector4, 2, 1> F_D;

//    Real rho = d1.solution[0];
//    Real rhou = d1.solution[1];
//    Real rhov = d1.solution[2];
//    Real rhoE = d1.solution[3];

//    Real drho_dx = d1.grad_solution(0,0);
//    Real drhou_dx = d1.grad_solution(1,0);
//    Real drhov_dx = d1.grad_solution(2,0);
//    Real drhoE_dx = d1.grad_solution(3,0);

//    Real drho_dy = d1.grad_solution(0,1);
//    Real drhou_dy = d1.grad_solution(1,1);
//    Real drhov_dy = d1.grad_solution(2,1);
//    Real drhoE_dy = d1.grad_solution(3,1);

//    Real dT_dx = 1/(rho*cv)* (drhoE_dx - (rhoE/rho-(1/(rho*rho))*(rhou*rhou+rhov*rhov))*drho_dx - (rhou/rho*drhou_dx + rhov/rho*drhov_dx));
//    Real dT_dy = 1/(rho*cv)*(drhoE_dy - (rhoE/rho-(1/(rho*rho))*(rhou*rhou+rhov*rhov))*drho_dy - (rhou/rho*drhou_dy+rhov*drhov_dy));

//    f_d[0] = 0;
//    f_d[1] = 2*mu*(1/rho*drhou_dx-rhou/(rho*rho)*drho_dx) + mu_v*(1/rho*drhou_dx-rhou/(rho*rho)*drho_dx + 1/rho*drhov_dy-rhov/(rho*rho)*drho_dy);
//    f_d[2] = mu*(1/rho*drhov_dx-rhov/(rho*rho)*drho_dx+1/rho*drhou_dy-rhou/(rho*rho)*drho_dy);
//    f_d[3] = f_d[1]*rhou/rho + f_d[2]*rhov/rho + kappa*dT_dx;

//    g_d[0] = 0;
//    g_d[1] = mu*(1/rho*drhov_dx-rhov/(rho*rho)*drho_dx+1/rho*drhou_dy-rhou/(rho*rho)*drho_dy);
//    g_d[2] = 2*mu*(1/rho*drhov_dy-rhov/(rho*rho)*drho_dy) + mu_v*(1/rho*drhou_dx-rhou/(rho*rho)*drho_dx + 1/rho*drhov_dy-rhov/(rho*rho)*drho_dy);
//    g_d[3] = g_d[1]*rhou/rho + g_d[2]*rhov/rho + kappa*dT_dy;

//    F_D(0) = f_d;
//    F_D(1) = g_d;

//    F_Dn = F_D(0)*unit_normal(0) + F_D(1)*unit_normal(1);

//}


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( sdm_MPITests_TestSuite, sdm_MPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
//  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().set("log_level", (Uint)INFO);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_source )
{
    const Real tol (0.000001);

    PhysDataBase<5, 3> d0;
    Eigen::Matrix<Real, 5, 1> s0;

    std::vector<Real> Vtransoption(3,0), Omegaoption(3,0), a0option(3,0), dOmegadtoption(3,0);

    // Test without relative movement

    d0.solution << 1., 1., 0., 0., 252754.; //P = 101101.4
    d0.coord << 1., 1., 0.;

    Vtransoption[0] = 0.;
    Vtransoption[1] = 0.;
    Vtransoption[2] = 0.;

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 0.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source3D> S0 = allocate_component<cf3::sdm::navierstokesmovingreference::Source3D>("source0");
    S0->options().set("Vtrans", Vtransoption);
    S0->options().set("Omega", Omegaoption);

    S0->compute_source(d0, s0);

    BOOST_CHECK_CLOSE(s0[0], 0., tol);
    BOOST_CHECK_CLOSE(s0[1], 0., tol);
    BOOST_CHECK_CLOSE(s0[2], 0., tol);
    BOOST_CHECK_CLOSE(s0[3], 0., tol);
    BOOST_CHECK_CLOSE(s0[4], 0., tol);


    // Test with relative movement, u only velocity component, no acceleration of the relative reference frame
    PhysDataBase<5, 3> d1;
    Eigen::Matrix<Real, 5, 1> s1;

    d1.solution << 1., 1., 2., -1., 252754.; //P = 101101.4
    d1.coord << 1., 1., 1.;

    Vtransoption[0] = 1.;
    Vtransoption[1] = 1.;
    Vtransoption[2] = -1.;

    Omegaoption[0] = 10.;
    Omegaoption[1] = 5.;
    Omegaoption[2] = 10;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source3D> S1 = allocate_component<cf3::sdm::navierstokesmovingreference::Source3D>("source1");
    S1->options().set("Vtrans", Vtransoption);
    S1->options().set("Omega", Omegaoption);

    S1->compute_source(d1, s1);

    BOOST_CHECK_CLOSE(s1[0],  0., tol);
    BOOST_CHECK_CLOSE(s1[1], 25., tol);
    BOOST_CHECK_CLOSE(s1[2], 60., tol);
    BOOST_CHECK_CLOSE(s1[3],-55., tol);
    BOOST_CHECK_CLOSE(s1[4], 40., tol);

//    // Test with relavite movement, u and v different from zero, no acceleration of the relative reference frame
//    PhysDataBase<4, 2> d2;
//    RealVector4 s2;

//    d2.solution << 1., 2., 2., 252754.; //P = 101101.4
//    d2.coord << 1., 1.;

//    Vtransoption[0] = 1.;
//    Vtransoption[1] = 1.;

//    Omegaoption[0] = 0.;
//    Omegaoption[1] = 0.;
//    Omegaoption[2] = 10;

//    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source2D> S2 = allocate_component<cf3::sdm::navierstokesmovingreference::Source2D>("source2");
//    S2->options().set("Vtrans", Vtransoption);
//    S2->options().set("Omega", Omegaoption);

//    S2->compute_source(d2, s2);

//    BOOST_CHECK_CLOSE(s2[0],   0., tol);
//    BOOST_CHECK_CLOSE(s2[1],140., tol);
//    BOOST_CHECK_CLOSE(s2[2], 60., tol);
//    BOOST_CHECK_CLOSE(s2[3],200., tol);

    // Test without initial relavite velocity, u and v zero, acceleration of the relative reference frame
    PhysDataBase<5, 3> d3;
    Eigen::Matrix<Real, 5, 1> s3;

    d3.solution << 1., 2., 2., 2., 252754.; //P = 101101.4
    d3.coord << 1., 1., 1.;

    Vtransoption[0] = 0.;
    Vtransoption[1] = 0.;
    Vtransoption[2] = 0.;

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 0.;

    a0option[0] = 1.;
    a0option[1] = 1.;
    a0option[2] = 1.;

    dOmegadtoption[0] = 0.;
    dOmegadtoption[1] = 0.;
    dOmegadtoption[2] = 0.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source3D> S3 = allocate_component<cf3::sdm::navierstokesmovingreference::Source3D>("source3");
    S3->options().set("Vtrans", Vtransoption);
    S3->options().set("Omega", Omegaoption);
    S3->options().set("a0", a0option);
    S3->options().set("dOmegadt", dOmegadtoption);

    S3->compute_source(d3, s3);

    BOOST_CHECK_CLOSE(s3[0], 0., tol);
    BOOST_CHECK_CLOSE(s3[1],-1., tol);
    BOOST_CHECK_CLOSE(s3[2],-1., tol);
    BOOST_CHECK_CLOSE(s3[3],-1., tol);
    BOOST_CHECK_CLOSE(s3[4],-6., tol);


    PhysDataBase<5, 3> d4;
    Eigen::Matrix<Real, 5, 1> s4;

    d4.solution << 1., 2., 2., 2., 252754.; //P = 101101.4
    d4.coord << 1., -1., -2.;

    Vtransoption[0] = 0.;
    Vtransoption[1] = 0.;
    Vtransoption[2] = 0.;

    Omegaoption[0] = -500.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 1000.;

    a0option[0] = 0.;
    a0option[1] = 0.;
    a0option[2] = 0.;

    dOmegadtoption[0] = 0.;
    dOmegadtoption[1] = 0.;
    dOmegadtoption[2] = 0.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Source3D> S4 = allocate_component<cf3::sdm::navierstokesmovingreference::Source3D>("source4");
    S4->options().set("Vtrans", Vtransoption);
    S4->options().set("Omega", Omegaoption);
    S4->options().set("a0", a0option);
    S4->options().set("dOmegadt", dOmegadtoption);

    S4->compute_source(d4, s4);

    BOOST_CHECK_CLOSE(s4[0],       0., tol);
    BOOST_CHECK_CLOSE(s4[1],    4000., tol);
    BOOST_CHECK_CLOSE(s4[2],-1256000., tol);
    BOOST_CHECK_CLOSE(s4[3],    2000., tol);
    BOOST_CHECK_CLOSE(s4[4],       0., tol);
}

//////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_convection )
{
    const Real tol (0.000001);

    RealVector3 normal(1, 0, 0);
    Eigen::Matrix<Real, 5, 1> flux;
    Real wavespeed = 0;

    PhysDataBase<5, 3> Data;

    std::vector<Real> Vtransoption(3,0), Omegaoption(3,0);

    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 104.71666666666667;

    Vtransoption[0] = 0.;
    Vtransoption[1] = 0.;
    Vtransoption[2] = 0.;

    Data.coord << 1., 0., 0.;
    Data.solution << 1., 1., 0., 0., 252754.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Convection3D> C1 = allocate_component<cf3::sdm::navierstokesmovingreference::Convection3D>("convection1");

    C1->options().set("Omega", Omegaoption);
    C1->options().set("Vtrans", Vtransoption);
    C1->options().set("gamma", 1.4);

    C1->compute_analytical_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0],      1. , tol);
    BOOST_CHECK_CLOSE(flux[1], 103295.51605555553, tol);
    BOOST_CHECK_CLOSE(flux[2],      0. , tol);
    BOOST_CHECK_CLOSE(flux[3],      0. , tol);
    BOOST_CHECK_CLOSE(flux[4], 356048.5160555556, tol);

//    std::cout << "wavespeed analytical = " << wavespeed << std::endl;
    Omegaoption[0] = 0.;
    Omegaoption[1] = 0.;
    Omegaoption[2] = 10.;

    Vtransoption[0] = 1.;
    Vtransoption[1] = 0.;
    Vtransoption[2] = 0.;

    Data.coord << 1., 0., 0.;
    Data.solution << 1., 1., 0., 0., 252754.;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Convection3D> C2 = allocate_component<cf3::sdm::navierstokesmovingreference::Convection3D>("convection1");

    C2->options().set("Omega", Omegaoption);
    C2->options().set("Vtrans", Vtransoption);
    C2->options().set("gamma", 1.4);

    wavespeed = 0;
    C2->compute_analytical_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0],      1. , tol);
    BOOST_CHECK_CLOSE(flux[1], 101122.6, tol);
    BOOST_CHECK_CLOSE(flux[2],      0. , tol);
    BOOST_CHECK_CLOSE(flux[3],      0. , tol);
    BOOST_CHECK_CLOSE(flux[4], 353875.6, tol);

    normal << 0.5, 0.5, 0.707106781;
    Omegaoption[0] = 0.;
    Omegaoption[1] = 10.;
    Omegaoption[2] = 200.;

    Vtransoption[0] = 0.;
    Vtransoption[1] = 0.;
    Vtransoption[2] = 0.;

    Data.coord << 2.35, -12.8, 3.;
    Data.solution << 3.5, -8.225, 1.75, 3.5, 884639.0;

    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Convection3D> C3 = allocate_component<cf3::sdm::navierstokesmovingreference::Convection3D>("convection1");

    C3->options().set("Omega", Omegaoption);
    C3->options().set("Vtrans", Vtransoption);
    C3->options().set("gamma", 1.4);

    wavespeed = 0;
    C3->compute_analytical_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0],-0.7626262665000001, tol);
    BOOST_CHECK_CLOSE(flux[1],  2602270.509296726, tol);
    BOOST_CHECK_CLOSE(flux[2], 2602268.3358118664, tol);
    BOOST_CHECK_CLOSE(flux[3], 3680162.9491002494, tol);
    BOOST_CHECK_CLOSE(flux[4],-1326790.2543176748, tol);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_diffusion )
{
    const Real tol (0.000001);

    RealVector3 normal(1., 0., 0.);
    Eigen::Matrix<Real, 5, 1>  flux;
    Real wavespeed;

    navierstokes::DiffusionPhysData<5u,3u> Data;

    boost::shared_ptr<cf3::sdm::navierstokes::Diffusion3D> D1 = allocate_component<cf3::sdm::navierstokes::Diffusion3D>("diffusion1");

    Data.coord << 1., 0;
    Data.solution << 1., 1., 0., 0., 100.;
    Data.solution_gradient << 1., 1., 0., 0., 0.,
                              0., 0., 0., 0., 0.,
                              0., 0., 0., 0., 0.;

    D1->options().set("gamma", 1.4);
    D1->options().set("R", 287.05);
    D1->options().set("k", 1.);
    D1->options().set("mu", 0.);

    D1->compute_flux(Data, normal, flux, wavespeed);


    BOOST_CHECK_CLOSE(flux[0], 0., tol);
    BOOST_CHECK_CLOSE(flux[1], 0., tol);
    BOOST_CHECK_CLOSE(flux[2], 0., tol);
    BOOST_CHECK_CLOSE(flux[3], 0., tol);
    BOOST_CHECK_CLOSE(flux[4], -0.139348546, tol);

//////////////////////////////////////////////////////////////////////

//    //mu_v different from zero en drhou_dx verschillend van drho_dx
    normal << 1., 0., 0.;
    wavespeed=0.;

    boost::shared_ptr<cf3::sdm::navierstokes::Diffusion3D> D2 = allocate_component<cf3::sdm::navierstokes::Diffusion3D>("diffusion1");

    Data.coord << 1., 0., 0.;
    Data.solution << 1., 1., 0., 0., 100.;
    Data.solution_gradient << 1., 2., 0., 0., 0.,
                              0., 0., 0., 0., 0.,
                              0., 0., 0., 0., 0.;

    D2->options().set("gamma", 1.4);
    D2->options().set("R", 287.05);
    D2->options().set("k", 1.);
    D2->options().set("mu", 1.);

    D2->compute_flux(Data, normal, flux, wavespeed);

    BOOST_CHECK_CLOSE(flux[0], 0., tol);
    BOOST_CHECK_CLOSE(flux[1], 1.33333333333333333333333, tol);
    BOOST_CHECK_CLOSE(flux[2], 0., tol);
    BOOST_CHECK_CLOSE(flux[3], 0., tol);
    BOOST_CHECK_CLOSE(flux[4], 1.192591302328282, tol);

//// ////////////////////////////////////////////////////////

//    normal << 0., 1.;
//    wavespeed=0;

//    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Diffusion2D> D3 = allocate_component<cf3::sdm::navierstokesmovingreference::Diffusion2D>("diffusion1");

//    Data.coord << 1., 0;
//    Data.solution << 1., 1., 0., 100.;
//    Data.solution_gradient << 0., 0., 0., 0.,
//                              1., 0., 2., 0.;

//    D3->options().set("gamma", 1.4);
//    D3->options().set("R", 287.05);
//    D3->options().set("k", 1.);
//    D3->options().set("mu", 1.);

//    D3->compute_flux(Data, normal, flux, wavespeed);


//    BOOST_CHECK_CLOSE(flux[0], 0., tol);
//    BOOST_CHECK_CLOSE(flux[1], -1., tol);
//    BOOST_CHECK_CLOSE(flux[2], 2.666666666666667, tol);
//    BOOST_CHECK_CLOSE(flux[3], -1.137955060094060, tol);

///////////////////////////////////////////////////////////////
//    normal << 1., 0.;
//    wavespeed=0;

//    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Diffusion2D> D4 = allocate_component<cf3::sdm::navierstokesmovingreference::Diffusion2D>("diffusion1");

//    Data.coord << 1., 0;
//    Data.solution << 1., 1., 0., 100.;
//    Data.solution_gradient << 1., 2., 2., 1.,
//                              1., 2., 2., 0.;

//    D4->options().set("gamma", 1.4);
//    D4->options().set("R", 287.05);
//    D4->options().set("k", 1.);
//    D4->options().set("mu", 1.);

//    D4->compute_flux(Data, normal, flux, wavespeed);

//    BOOST_CHECK_CLOSE(flux[0], 0., tol);
//    BOOST_CHECK_CLOSE(flux[1], 0., tol);
//    BOOST_CHECK_CLOSE(flux[2], 3., tol);
//    BOOST_CHECK_CLOSE(flux[3], -0.13934854559556, tol);
///////////////////////////////////////////////////////////

//    normal << 0., 1.;
//    wavespeed=0;

//    boost::shared_ptr<cf3::sdm::navierstokesmovingreference::Diffusion2D> D5 = allocate_component<cf3::sdm::navierstokesmovingreference::Diffusion2D>("diffusion1");

//    Data.coord << 1., 0;
//    Data.solution << 1., 1., 0., 100.;
//    Data.solution_gradient << 1., 2., 2., 0.,
//                              1., 2., 2., 1.;

//    D5->options().set("gamma", 1.4);
//    D5->options().set("R", 287.05);
//    D5->options().set("k", 1.);
//    D5->options().set("mu", 1.);

//    D5->compute_flux(Data, normal, flux, wavespeed);

//    BOOST_CHECK_CLOSE(flux[0], 0., tol);
//    BOOST_CHECK_CLOSE(flux[1], 3., tol);
//    BOOST_CHECK_CLOSE(flux[2], 2., tol);
//    BOOST_CHECK_CLOSE(flux[3], 2.860651454450444, tol);
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
//  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
