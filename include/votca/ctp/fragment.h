/*
 * Copyright 2009-2011 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __VOTCA_CTP_FRAGMENT_H
#define	__VOTCA_CTP_FRAGMENT_H

#include <votca/ctp/atom.h>

namespace votca { namespace ctp {

class Segment;
class Molecule;
    
/**
    \brief Rigid fragment. One conjugated segment contains several rigid fragments.

 * Apart from the position and orientation it has a pointer to a conjugated segment 
 * it belongs to as well as positions of atoms which belong to it of two types those 
 * which are generated by MD and those obtained by substitution of rigid copies
 */

class Fragment {
public:
     /// Constructor
     Fragment(Segment *segment){}
     /// Destructor
    ~Fragment(){}
    /// Returns a pointer to a segment this fragment belongs to 
    Segment* getSegment(){
        return _segment;
    }
    /// Returns a pointer to a segment this fragment belongs to 
    Molecule* getMolecule(){
        return _molecule;
    }
    /// Rotates the fragment with respect to the center defined by the Map
    void Rotate (){ 
        throw runtime_error( string("Not implemented") ); 
    }
    /// Translates the fragment by a specified vector
    void Translate(){ 
        throw runtime_error( string("Not implemented") ); 
    }
    

private:

    /// List of pointers to atoms belonging to this fragment
    vector < Atom* > _atoms;
    /// Conjugated segment this fragment belongs to
    Segment *_segment;
    /// Molecule this Fragment belongs to
    Molecule *_molecule;
   /// Name of the rigid fragment 
    string _name;
    /// Rigid fragment ID
    int _id;
    /// Weights used to calculate fragment center
    vector< double > _weights;
    
};

}}

#endif	/* __VOTCA_CTP_FRAGMENT_H */

