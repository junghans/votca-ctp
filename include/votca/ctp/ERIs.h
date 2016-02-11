/* 
 *            Copyright 2009-2012 The VOTCA Development Team
 *                       (http://www.votca.org)
 *
 *      Licensed under the Apache License, Version 2.0 (the "License")
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _VOTCA_CTP_ERIS_H
#define	_VOTCA_CTP_ERIS_H


#include <votca/ctp/aomatrix.h>
#include <votca/ctp/aobasis.h>
#include <votca/ctp/threecenters.h>
#include <string>
#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <votca/tools/linalg.h>
#ifdef _OPENMP
#include <omp.h>
#endif


using namespace std;
using namespace votca::tools;

namespace votca { namespace ctp {
    namespace ub = boost::numeric::ublas;
/**
* \brief Takes a density matrix and and an auxillary basis set and calculates the electron repulsion integrals. 
*
* 
* 
*/    
    class ERIs{    
        
    public:
      
        
        
        
        void Initialize(AOBasis &_dftbasis, AOBasis &_auxbasis, AOOverlap &_auxAOoverlap, AOCoulomb &_auxAOcoulomb);
        void Initialize_Symmetric(AOBasis &_dftbasis, AOBasis &_auxbasis, AOOverlap &_auxAOoverlap, AOCoulomb &_auxAOcoulomb);
        
        ub::matrix<double> getERIs(){return _ERIs;}
        double& getERIsenergy(){return _ERIsenergy;}
        
        void CalculateERIs(ub::matrix<double> &DMAT, AOOverlap &_auxAOoverlap, ub::matrix<double>& _AOIntegrals);
        
        
        int getSize1(){return _ERIs.size1();}
        int getSize2(){return _ERIs.size2();}
        
        void printERIs();
        
    private:
        
        TCMatrix_dft _threecenter;
        ub::matrix<double> _Vcoulomb; //resolution of the 1/r operator in aux basis functions
        ub::matrix<double> _ERIs;
        ub::matrix<double> _ERIs_Symmetric;
        double _ERIsenergy;
        void CalculateEnergy(ub::vector<double> &dmatasarray);
    };
    
    
    
    

}}

#endif	/* ERIS_H */

