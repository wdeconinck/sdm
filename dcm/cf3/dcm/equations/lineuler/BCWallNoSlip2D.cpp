// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/common/Builder.hpp"

#include "cf3/dcm/equations/lineuler/BCWallNoSlip2D.hpp"
#include "solver/Solver.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace dcm {
namespace equations {
namespace lineuler {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<BCWallNoSlip2D,BC,LibLinEuler> BCWallNoSlip2D_builder;

/////////////////////////////////////////////////////////////////////////////

} // lineuler
} // equations
} // dcm
} // cf3
