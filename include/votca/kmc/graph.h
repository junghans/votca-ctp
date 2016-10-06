/*
 * Copyright 2009-2013 The VOTCA Development Team (http://www.votca.org)
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

#ifndef __VOTCA_KMC_GRAPH_H_
#define __VOTCA_KMC_GRAPH_H_

#include <votca/kmc/bnode.h>

namespace votca { namespace kmc {
  
class Graph {
public:
    
    void Load(std::string filename);    
    void Print();
    
    // add a node to the vector of Nodes
    BNode* AddNode() {
        BNode *node = new BNode();
        nodes.push_back( node );
        return node;
    };
    
    BNode* GetNode( int id ) {
        std::vector< BNode* >::iterator node = nodes.begin() ;
        while ( (*node)->id != id ) node++ ;
        return *node;
    };
       
private:

    std::vector< BNode* > nodes;
    std::vector< Edge* > edges;
    
};


void Graph::Load(std::string filename) {
    
    std::cout << "Loading the graph from " << filename << std::endl;
    
    // initialising the database file
    votca::tools::Database db;
    db.Open( filename );
    
    votca::tools::Statement *stmt = db.Prepare("SELECT id-1, posX, posY, posZ FROM segments;");

    while (stmt->Step() != SQLITE_DONE)
    {
        BNode *node = AddNode();
        int id = stmt->Column<int>(0);
        node->id = id+1;
        
        // position in nm
        double x = stmt->Column<double>(1);
        double y = stmt->Column<double>(2);
        double z = stmt->Column<double>(3);
        
        myvec position = myvec(x, y, z); 
        node->position = position;
        
        //node->PrintNode();   
    }
    
    delete stmt;
    
    //List of neighbours (from neighbour list for charge transfer - state.sql "pairs" table) 
    //List of rates for electron transfer from seg 1 to seg 2 and vice versa
    stmt = db.Prepare("SELECT seg1, seg2e, drX, drY, drZ, rate12e, rate21 FROM pairs;");
    while (stmt->Step() != SQLITE_DONE)     
    {           
        int seg1 = stmt->Column<int>(0);
        int seg2 = stmt->Column<int>(1);
        
        BNode* node1 = GetNode( seg1 );
        BNode* node2 = GetNode( seg2 );
        
        double dx_pbc = stmt->Column<double>(2);
        double dy_pbc = stmt->Column<double>(3);
        double dz_pbc = stmt->Column<double>(4);
        
        votca::tools::vec distance_pbc(dx_pbc, dy_pbc, dz_pbc);
        Edge* edge = new Edge(node1, node2, distance_pbc); 
        
        edges.push_back( edge );
        
        // make sure no duplicates are added
        node1->AddNeighbor( node2 );
        node2->AddNeighbor( node1 );
        
        //Rate of electron(e) transfer from seg 1 to seg 2
        double rate12e = stmt->Column<double>(5);
        //Rate of electron transfer from seg 2 to seg 1
        double rate21e = stmt->Column<double>(6);
         
    }
    delete stmt;

}

    void Graph::Print(){
    
        for (std::vector< BNode* >::iterator node = nodes.begin() ; node != nodes.end(); ++node) {
            (*node)->PrintNode();
        }
    };
 
    
}}



#endif

