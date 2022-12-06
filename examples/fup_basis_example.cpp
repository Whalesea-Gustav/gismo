/** @file basis_example.cpp

    @brief Tutorial on gsBasis class.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#include <iostream>
#include <gismo.h>

#include <gsFupBasis/gsFupBasis.h>

using namespace gismo;

int main(int argc, char* argv[])
{
    std::string output;
    index_t numknots(7), deg(1);

    gsCmdLine cmd("Tutorial on gsFupBasis class.");
    cmd.addInt("interior", "Interior knots.", numknots);
    cmd.addInt("deg", "Degree.", deg);
    // cmd.addPlainString("input", "G+Smo input basis file.", input);
     cmd.addString("o", "output", "Name of the output file.", output);
    try { cmd.getValues(argc,argv); } catch (int rv) { return rv; }

    // ======================================================================
    // reading the basis
    // ======================================================================

    gsFupBasis<real_t> fup(0,1,numknots,deg);

    // printing the basis
    gsInfo << "The file contains: \n" << fup << "\n";


    // printing some properties of the basis
    gsInfo << "Dimension of the parameter space: " << fup.dim() << "\n"
           << "Number of basis functions: " << fup.size() << "\n"
           //<< "Number of elements: " << fup.numElements() << "\n"
           << "Max degree of the basis: " << fup.maxDegree() << "\n"
           << "Min degree of the basis: " << fup.minDegree() << "\n"
           << "\n";


    // support of the basis
    // (dim x 2 matrix, the parametric domain)
    gsMatrix<> support = fup.support();
    gsInfo << "Support: \n"
           << support << "\n\n";

    gsMatrix<> anchors = fup.anchors();
    gsInfo << "Anchors : \n"
           << anchors << "\n\n";

    // ======================================================================
    // evaluation
    // ======================================================================


    // ----------------------------------------------------------------------
    // values
    // ----------------------------------------------------------------------

    gsMatrix<> u = gsPointGrid(support(0,0), support(0,1), 15);
    gsInfo << "Evaluation points: \n" << u << "\n\n";

    // indices of active (nonzero) functions at parameter u
    gsMatrix<index_t> active = fup.active(u);
    gsInfo << "Active functions at each point (per column): \n"
           << active << "\n\n";

    // values of each basis function
    for (index_t j = 0; j != fup.size(); j++)
    {
        gsMatrix<> val = fup.evalAllDersSingle(j, u, 1);
        gsInfo << "++++basis fun. index:  " << j
               << "\n  --values:\n" << val.row(0)
               << "\n  --deriv:\n" << val.row(1) << "\n";
    }
    gsInfo << "\n";

       for (index_t j = 0; j != fup.size(); j++)
    {
        gsInfo << "++++basis fun. index:  " << j
               << "\n  --support:\n" << fup.support(j)<< "\n";
    }
    gsInfo << "\n";

    // values of all active functions at u
    gsMatrix<> values = fup.eval(u);
    gsInfo << "Values at u (" <<values.rows() << "): \n"
           << values << "\n\n";
    gsInfo << "Sum: \n"
           << values.colwise().sum() << "\n\n";

    gsMatrix<> derivs = fup.deriv(u);
    gsInfo << "Derivatives at u " << derivs.cols() << ": \n"
           << derivs << "\n\n";

    // values of single basis functions
    for (index_t i = 0; i != active.rows(); i++)
    {
        gsMatrix<> val = fup.evalSingle(active(i), u);

        gsInfo << "basis fun. index:  " << active(i)
               << "   value: " << val(0, 0) << "\n";
    }
    gsInfo << "\n";

    // derivatives of single basis function
    for (index_t i = 0; i != active.rows(); i++)
    {
        gsMatrix<> der = fup.derivSingle(active(i), u);

        gsInfo << "basis fun. index:  " << active(i)
               << "   value: " << std::setw(15) <<  der(0, 0) << "\n";

        for (index_t row = 1; row != der.rows(); row++)
        {
            gsInfo << std::setw(46) << der(row, 0) << "\n";
        }
    }
    gsInfo << "\n";


    // ----------------------------------------------------------------------
    // second derivatives
    // ----------------------------------------------------------------------


    gsMatrix<> derivs2 = fup.deriv2(u);
    gsInfo << "Second derivatives at u " << derivs2.cols() << ": \n"
           << derivs2 << "\n\n";

    for (index_t i = 0; i != active.rows(); i++)
    {
        gsMatrix<> der2 = fup.deriv2Single(active(i), u);

        gsInfo << "basis fun. index:  " << active(i)
               << "   value: " << std::setw(15) << der2(0, 0) << "\n";

        for (index_t row = 1; row != der2.rows(); row++)
        {
            gsInfo << std::setw(46) << der2(row, 0) << "\n";
        }
    }

    gsInfo << "\nFor more information about evaluation "
           << "(and order of derivatives) look at doxygen documentation."
           << "\n\n";

    // ======================================================================
    // writing to a paraview file
    // ======================================================================

    if (output != "")
    {
        gsInfo << "Writing the basis to a paraview file: " << output
               << "\n\n";
        gsWriteParaview(fup, output);
    }
    else
    {
        gsInfo << "Done. No output created, re-run with --output <filename> to get a ParaView "
                  "file containing the solution.\n";
    }

    return 0;
}



