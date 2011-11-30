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

#include <votca/ctp/qmtopology.h>
#include <votca/ctp/qmnblist.h>
#include <votca/tools/globals.h>

namespace votca { namespace ctp {

QMTopology::QMTopology()
    : _db_id(0)
{}

QMTopology::~QMTopology()
{}


void QMTopology::Cleanup()
{
    vector<QMCrgUnit *>::iterator iter;
    for(iter=_crgunits.begin(); iter!=_crgunits.end(); ++iter)
        delete *iter;
    _crgunits.clear();
    _mcharges.clear();
    Topology::Cleanup();
    _db_id=0;
}

void QMTopology::Initialize(Topology& cg_top)
{
    CopyTopologyData(&cg_top);
//    this->InitChargeUnits();
//}

//void QMTopology::InitChargeUnits(){

    if ( tools::globals::verbose ) {
        cout << "Initializing conjugated segments" << endl;
        cout << " segment[name:type] fragment[name:type:position] molecule[name:id] " << endl;
    }
    
    BeadContainer::iterator itb;
    for (itb = _beads.begin() ; itb< _beads.end(); ++itb){
	
        QMBead * bead = dynamic_cast<QMBead *>(*itb);
        string bead_name = bead->getName();
        string bead_type = bead->getType()->getName();
        string molecule_name = bead->getMolecule()->getName();

        // check if the bead (rigid fragment) is in any defined conjugated segment
        if ( (bead->Options()).exists("segment.name")){

	    // type of the conjugated segment to which this fragment belongs
            string segment_type = (bead->Options()).get("segment.type").as<string>();

            // position of the bead in the conjugated segment (starts with zero internally)
            int bead_position = (bead->Options()).get("segment.position").as<int>();
            bead_position--;

            // name of the conjugated segment in a molecule - used to group beads (fragments)
            string segment_name = (bead->Options()).get("segment.name").as<string>();

            // check if the segment has already been created by another fragment
            int molecule_id = bead->getMolecule()->getId();
            string segment_id = boost::lexical_cast<string>(molecule_id)+":" + segment_name;
            QMCrgUnit * segment = GetCrgUnitByName(segment_id);

            // if not, create a new segment
	    if(segment == NULL) segment = CreateCrgUnit(segment_id, segment_type, molecule_id);

            /// TODO: move this to a calculator
            segment->setDouble("lambda_intra_charging", segment->getType()->getOptions()->get("echarging").as<double>());
            segment->setDouble("lambda_intra_discharging", segment->getType()->getOptions()->get("edischarging").as<double>());
            segment->setDouble("energy_intra", segment->getType()->getOptions()->get("energy").as<double>());
            
	    bead->setCrg(segment);
            bead->setiPos(bead_position);


	    if ( tools::globals::verbose ) {
              cout << " segment[" << segment_name  << ":" << segment_type  << "] "
                   << "fragment[" << bead_name  << ":" << bead_type << ":" << bead_position  << "] "
                   << "molecule[" << molecule_name  << ":" << molecule_id  << "] " << endl;
	    }
        }
        else{ // remove the bead from the CG trajectory
            bead->setCrg(NULL);
        }
    }

    if ( tools::globals::verbose ) {
        cout << " segment[name:type] fragment[name:type:position] molecule[name:id] " << endl;
        cout << "Done with initializing conjugated segments" << endl;
    }

}

void QMTopology::Update(Topology& cg_top)
{
    BeadContainer::iterator iter;
    BeadContainer::iterator iter_cg;

    assert(cg_top.Beads().size() == _beads.size());

    setBox(cg_top.getBox());
    _time = cg_top.getTime();
    _step = cg_top.getStep();

    iter_cg = cg_top.Beads().begin();
    for(iter=_beads.begin(); iter!=_beads.end(); ++iter, ++iter_cg) {
        (*iter)->setPos((*iter_cg)->getPos());
        if((*iter_cg)->HasU())
            (*iter)->setU((*iter_cg)->getU());
        if((*iter_cg)->HasV())
            (*iter)->setV((*iter_cg)->getV());
        if((*iter_cg)->HasW())
            (*iter)->setW((*iter_cg)->getW());
        QMBead * b = (QMBead*)(*iter);

        if((*iter_cg)->getSymmetry() == 1) {
            (*iter)->setU(vec(1,0,0));
            (*iter)->setV(vec(0,1,0));
            (*iter)->setW(vec(0,0,1));
        }

        b->UpdateCrg();
    }
}

void QMTopology::LoadListCharges(const string &file)
{
    _jcalc.Initialize(file);
}


Molecule *QMTopology::AddAtomisticBeads(CrgUnit *crg, Topology * totop){
     
    mol_and_orb * atoms = crg->rotate_translate_beads();
    totop->CreateResidue("DUM");
    Molecule *mi = totop->CreateMolecule((crg)->getName());
    mi->setUserData(crg);   //mi->getUserData<CrgUnit>();
    for (int i=0;i<atoms->getN();i++){
        vec pos = unit<bohr,nm>::to(atoms->GetPos(i));
        string atomtype = string( atoms->gettype(i) ); //+ string("-") + lexical_cast<string>(crg->getId());
        BeadType * bt= totop->GetOrCreateBeadType(atomtype);
        Bead * bead = totop ->CreateBead(1, atomtype,bt,0, 0, 0.);
        bead->setPos(pos);
        mi->AddBead(bead,"???");
        if(crg->getType()->GetCrgUnit().getChargesNeutr()) {
            double charge_of_bead_neutral=crg->getType()->GetCrgUnit().getChargesNeutr()->mpls[i];
            bead->setQ(charge_of_bead_neutral);
        }
      
    }
    delete atoms->getorbs();
    delete atoms;
    return mi;
}

Bead *QMTopology::CreateBead(byte_t symmetry, string name, BeadType *type, int resnr, double m, double q)
{
    QMBead *bead = new QMBead(this, _beads.size(), type, symmetry, name, resnr, m, q);
    _beads.push_back(bead);
    return bead;
}

/*
// TODO: this function should not be in qmtopology!
void QMTopology::ComputeAllTransferIntegrals(){
    for(QMNBList::iterator iter = _nblist.begin();
        iter!=_nblist.end();++iter) {
        CrgUnit *crg1 = (*iter)->Crg1PBCCopy();
        CrgUnit *crg2 = (*iter)->Crg2PBCCopy();
        vector <double> Js = GetJCalc().CalcJ(*crg1, *crg2);
        (*iter)->setJs(Js);
    }
}
*/
   // In topology.cc???
//Copy charges to either charged or neutral case

void QMTopology::CopyCharges(CrgUnit *crg, Molecule *mol)
{
    if(mol->BeadCount() != crg->getType()->GetCrgUnit().getN())
        throw std::runtime_error("QMTopology::CopyCharges: number of atoms in crgunit does not match number of beads in molecule");

    if(!crg->getType()->GetCrgUnit().getChargesNeutr())
        throw std::runtime_error("QMTopology::CopyCharges: no charges defined");

    // loop over all beads in that molecule
    for (int i = 0; i < mol->BeadCount(); i++) {
        // get charge
        double charge_of_bead_i_neutral = crg->getType()->GetCrgUnit().getChargesNeutr()->mpls[i];
        //set charge
        mol->getBead(i)->setQ(charge_of_bead_i_neutral);
    }
}

//Copy charges to either charged or neutral case
void QMTopology::CopyChargesOccupied(CrgUnit *crg, Molecule *mol)
{
    if(mol->BeadCount() != crg->getType()->GetCrgUnit().getN())
        throw std::runtime_error("QMTopology::CopyCharges: number of atoms in crgunit does not match number of beads in molecule");

    if(!crg->getType()->GetCrgUnit().getChargesCrged())
        throw std::runtime_error("QMTopology::CopyCharges: no charges defined");

    
    if(crg->getType()->GetCrgUnit().getChargesCrged()->mpls.size() == 0)
        throw std::runtime_error("QMTopology::CopyCharges: no charges defined (mpls.size()==0, that is strange!)");

    //loop over all beads in that molecule
    for (int i = 0; i < mol->BeadCount(); i++) {
        //get charge
        double charge_of_bead_i_charged = crg->getType()->GetCrgUnit().getChargesCrged()->mpls[i];
        //set charge
        mol->getBead(i)->setQ(charge_of_bead_i_charged);
    }
}

QMCrgUnit *QMTopology::CreateCrgUnit(const string &name, const string &type_name, int molid)
{
    CreateCrgUnit(_crgunits_by_id.size()+1, name, type_name, molid);
}

QMCrgUnit *QMTopology::CreateCrgUnit(int id, const string &name, const string &type_name, int molid)
{
    map<int, QMCrgUnit*>::iterator iter;
    iter = _crgunits_by_id.find(id);
    if(iter != _crgunits_by_id.end())
        throw std::runtime_error("charge unit with id " + boost::lexical_cast<string>(id) + " already exists");

    if(GetCrgUnitByName(name))
        throw std::runtime_error("charge unit with name " + name + " already exists");

    QMCrgUnit *crg;

    CrgUnitType *type = _jcalc.GetCrgUnitTypeByName(type_name);
    if(!type)
        throw runtime_error("Charge unit type not found: " + type_name);

    crg = new QMCrgUnit(id, type, molid);

    _mcharges.insert(make_pair(name, crg));
    _crgunits.push_back(crg);
    _crgunits_by_id[id] = crg;
    crg->setName(name);
    return crg;
}

}}
