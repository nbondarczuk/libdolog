///////////////////////////////////////////////////////////////////////////////
//Copyright (c) 2014 Ericsson
//
//The copyright in this work is vested in Ericsson Telekommunikation GmbH
//(hereafter "Ericsson"). The information contained in this work (either in whole
//or in part) is confidential and must not be modified, reproduced, disclosed or
//disseminated to others or used for purposes other than that for which it is
//supplied, without the prior written permission of Ericsson. If this work or any
//part hereof is furnished to a third party by virtue of a contract with that
//party, use of this work by such party shall be governed by the express
//contractual terms between Ericsson, which is party to that contract and the
//said party.
//
//The information in this document is subject to change without notice and
//should not be construed as a commitment by Ericsson. Ericsson assumes no
//responsibility for any errors that may appear in this document. With the
//appearance of a new version of this document all older versions become
//invalid.
//
//All rights reserved.
///////////////////////////////////////////////////////////////////////////////
/*
 * static char *SCCS_VERSION = "%I%";
 */

#include <string>
#include <iostream>
#include <map>
#include <list>
#include <stdexcept>  
#include <algorithm>
#include <sstream>
#include <functional>

#include "DoLog.hpp"

DoLog dl;

////////////////////////////////////////////////////////////////////////////////
// DoLogAttValSet
////////////////////////////////////////////////////////////////////////////////

DoLogAttValSet::DoLogAttValSet() {}

DoLogAttValSet::~DoLogAttValSet()//deep delete 
{
    SQL_VALUE *ptr;
    DoLogAttValSeqIt it = m_v_seq.begin();
    while (it != m_v_seq.end())
    {
        ptr = *it;
        delete ptr;
        ++it;
    }
    
    m_v_seq.clear();
}

string DoLogAttValSet::Xml()
{
    SQL_VALUE *ptr;
    stringstream ss;
    DoLogAttValSeqIt it = m_v_seq.begin();
    while (it != m_v_seq.end())
    {
        ptr = *it;
        ss << ptr->xml();        
        ss << "\n";
        ++it;
    }
    return ss.str();
}

void DoLogAttValSet::Add(SQL_VALUE *pv)
{
    m_v_seq.push_back(pv);
}

string DoLogAttValSet::Digest()
{
    SQL_VALUE *ptr;
    string k;    
    DoLogAttValSeqIt it = m_v_seq.begin();
    while (it != m_v_seq.end())
    {
        ptr = *it;
        k += ptr->label();
        k += ".";
        k += ptr->str();        
        if (++it != m_v_seq.end()) k += ".";            
    }
    return k;
}

DoLogAttValSet& DoLogAttValSet::operator+=(SQL_VALUE *prv)
{
    Add(prv);
    return *this;
}

DoLogAttValSet& DoLogAttValSet::operator=(DoLogAttValSet &rv)
{
    if (this != &rv) // same, same? nooo
    {
        SQL_VALUE *ptr;
        DoLogAttValSeqConstIt it = rv.m_v_seq.begin();
        // destination object is cleaned before operation
        this->m_v_seq.clear();
        while (it != rv.m_v_seq.end())
        {
            ptr = *it;
            this->m_v_seq.push_back(ptr->clone()); // deep copy
            ++it;
        }
        // no release of source object
    }

    return *this;
}

// List of attributes
string DoLogAttValSet::SqlAttSeq()
{
    SQL_VALUE *ptr;
    stringstream ss;
    DoLogAttValSeqIt it = m_v_seq.begin();
    while (it != m_v_seq.end())
    {
        ptr = *it;
        ss << ptr->label();        
        if (++it != m_v_seq.end()) ss << ",";            
    }
    return ss.str();
}

// List of values 
string DoLogAttValSet::SqlValSeq()
{
    SQL_VALUE *ptr;
    stringstream ss;
    DoLogAttValSeqIt it = m_v_seq.begin();
    while (it != m_v_seq.end())
    {
        ptr = *it;
        ss << ptr->val();        
        if (++it != m_v_seq.end()) ss << ",";            
    }
    return ss.str();
}

// Equi condition list 
string DoLogAttValSet::SqlKeyEqCond()
{
    SQL_VALUE *ptr;
    stringstream ss;
    DoLogAttValSeqIt it = m_v_seq.begin();
    while (it != m_v_seq.end())
    {
        ptr = *it;     
        ss << ptr->label();        
        ss << " = ";
        ss << ptr->val();        
        if (++it != m_v_seq.end()) ss << " AND ";            
    }
    return ss.str();    
}

// List of values to be assigned to the attributes
string DoLogAttValSet::SqlValAssign()
{
    SQL_VALUE *ptr;
    stringstream ss;
    DoLogAttValSeqIt it = m_v_seq.begin();
    while (it != m_v_seq.end())
    {
        ptr = *it;
        ss << ptr->label();        
        ss << " = ";
        ss << ptr->val();        
        if (++it != m_v_seq.end()) ss << ", ";            
    }
    return ss.str();    
}

////////////////////////////////////////////////////////////////////////////////
// DoLogOp
////////////////////////////////////////////////////////////////////////////////

DoLogOp::DoLogOp(const string e): m_e(e) {}

void DoLogOp::AddKey(SQL_VALUE *pv){m_k.Add(pv);}

DoLogOp& DoLogOp::operator+=(SQL_VALUE *prv)
{
    AddKey(prv);
    return *this;
}

void DoLogOp::addKeySet(DoLogAttValSet *pvs)
{
    m_k = *pvs;
}

////////////////////////////////////////////////////////////////////////////////
// DoLogOp_Insert
////////////////////////////////////////////////////////////////////////////////

DoLogOp_Insert::DoLogOp_Insert(const string e): DoLogOp(e) {}

string DoLogOp_Insert::XmlRedo()
{
    stringstream ss;
    ss << "<INSERT>\n";
    ss << "<ENTITY>" << m_e << "</ENTITY>\n";
    ss << "<KEY>\n";        
    ss << m_k.Xml();
    ss << "</KEY>\n";
    ss << "<VALUE>\n";        
    ss << m_lv_after.Xml();
    ss << "\n</VALUE>\n";   
    ss << "</INSERT>\n";
    return ss.str();
}

string DoLogOp_Insert::XmlUndo()
{
    stringstream ss;
    ss << "<DELETE>\n";
    ss << "<ENTITY>" << m_e << "</ENTITY>\n";
    ss << "<KEY>\n";        
    ss << m_k.Xml();
    ss << "</KEY>\n";
    ss << "<VALUE>\n";        
    ss << m_lv_after.Xml();
    ss << "\n</VALUE>\n";   
    ss << "</DELETE>\n";    
    return ss.str();
}

void DoLogOp_Insert::AddVal(SQL_VALUE *pv, const DoLogOpValState s)
{
    switch(s)   
    {
        case POSTOPVAL: 
            m_lv_after.Add(pv);
            break;
            
        default:
            throw invalid_argument("Incompatible state selected");
    }    
}

DoLogOp& DoLogOp_Insert::operator*=(SQL_VALUE *prv)
{
    AddVal(prv, PREOPVAL); // should fail in exception
    return *this;
}

DoLogOp& DoLogOp_Insert::operator/=(SQL_VALUE *prv)
{
    AddVal(prv, POSTOPVAL);
    return *this;
}

void DoLogOp_Insert::addValSet(DoLogAttValSet *p_lv_before, DoLogAttValSet *p_lv_after)
{
    m_lv_after = *p_lv_after;
}

string DoLogOp_Insert::Sql()
{
    stringstream ss;
    ss << "INSERT INTO " << m_e;
    ss << "(" << m_lv_after.SqlAttSeq() << ")"; 
    ss << " VALUES ";
    ss << "(" << m_lv_after.SqlValSeq() << ")";
    ss << "\n";
    return ss.str();
}
    
////////////////////////////////////////////////////////////////////////////////
// DoLogOp_Delete
////////////////////////////////////////////////////////////////////////////////

DoLogOp_Delete::DoLogOp_Delete(const string e): DoLogOp(e) {}

string
DoLogOp_Delete::XmlRedo()
{
    stringstream ss;
    ss << "<DELETE>\n";
    ss << "<ENTITY>" << m_e << "</ENTITY>\n";
    ss << "<KEY>\n";        
    ss << m_k.Xml();
    ss << "</KEY>\n";
    ss << "<VALUE>\n";        
    ss << m_lv_before.Xml();
    ss << "</VALUE>\n";   
    ss << "</DELETE>\n";
    return ss.str();
}

string DoLogOp_Delete::XmlUndo()
{
    stringstream ss;
    ss << "<INSERT>\n";
    ss << "<ENTITY>" << m_e << "</ENTITY>\n";
    ss << "<KEY>\n";        
    ss << m_k.Xml();
    ss << "</KEY>\n";
    ss << "<VALUE>\n";        
    ss << m_lv_before.Xml();
    ss << "</VALUE>\n";   
    ss << "</INSERT>\n";
    return ss.str();
}

void DoLogOp_Delete::AddVal(SQL_VALUE *pv, const DoLogOpValState s)
{
    switch(s)   
    {
        case PREOPVAL: 
            m_lv_before.Add(pv);
            break;
            
        default:
            throw invalid_argument("Incompatible state selected");
    }    
}

DoLogOp& DoLogOp_Delete::operator*=(SQL_VALUE *prv)
{
    AddVal(prv, PREOPVAL); 
    return *this;
}

DoLogOp& DoLogOp_Delete::operator/=(SQL_VALUE *prv)
{
    AddVal(prv, POSTOPVAL); // should fail in exception
    return *this;
}

void DoLogOp_Delete::addValSet(DoLogAttValSet *p_lv_before, DoLogAttValSet *p_lv_after)
{
    m_lv_before = *p_lv_before;
}

string DoLogOp_Delete::Sql()
{
    stringstream ss;
    ss << "DELETE FROM " << m_e << " WHERE ";
    ss << m_k.SqlKeyEqCond();
    ss << " AND ";
    ss << m_lv_before.SqlKeyEqCond();
    ss << "\n";
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
// DoLogOp_Update
////////////////////////////////////////////////////////////////////////////////

DoLogOp_Update::DoLogOp_Update(const string e): DoLogOp(e) {}

string DoLogOp_Update::XmlRedo()
{
    stringstream ss;
    ss << "<UPDATE>\n";
    ss << "<ENTITY>" << m_e << "</ENTITY>\n";
    ss << "<KEY>\n";        
    ss << m_k.Xml();
    ss << "</KEY>\n";
    ss << "<VALUE>\n";        
    ss << "<BEFORE>\n";        
    ss << m_lv_before.Xml();
    ss << "</BEFORE>\n";        
    ss << "<AFTER>\n";        
    ss << m_lv_after.Xml();
    ss << "</AFTER>\n";
    ss << "</VALUE>\n";   
    ss << "</UPDATE>\n";
    return ss.str();
}

string DoLogOp_Update::XmlUndo()
{
    stringstream ss;
    ss << "<UPDATE>\n";
    ss << "<ENTITY>" << m_e << "</ENTITY>\n";
    ss << "<KEY>\n";        
    ss << m_k.Xml();
    ss << "</KEY>\n";
    ss << "<VALUE>\n";        
    ss << m_lv_before.Xml();
    ss << "</VALUE>\n";   
    ss << "</UPDATE>\n";
    return ss.str();
}

void DoLogOp_Update::AddVal(SQL_VALUE *pv, const DoLogOpValState s)
{
    switch(s)   
    {
        case PREOPVAL:
            m_lv_before.Add(pv); 
            break;
            
        case POSTOPVAL: 
            m_lv_after.Add(pv);
            break;
            
        default:
            throw invalid_argument("Incompatible state selected");            
    }        
}

DoLogOp& DoLogOp_Update::operator*=(SQL_VALUE *prv)
{
    AddVal(prv, PREOPVAL); // should fail in exception
    return *this;
}

DoLogOp& DoLogOp_Update::operator/=(SQL_VALUE *prv)
{
    AddVal(prv, POSTOPVAL);
    return *this;
}

void DoLogOp_Update::addValSet(DoLogAttValSet *p_lv_before, DoLogAttValSet *p_lv_after)
{
    m_lv_before = *p_lv_before;
    m_lv_after = *p_lv_after;
}

string DoLogOp_Update::Sql()
{
    stringstream ss;
    ss << "UPDATE " << m_e << " SET ";
    ss << m_lv_before.SqlValAssign();
    ss << " WHERE ";
    ss << m_k.SqlKeyEqCond();    
    ss << "\n";
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
// DoLogBatch
////////////////////////////////////////////////////////////////////////////////

DoLogBatch::DoLogBatch(const string digest, const DoLogAttValSet batch_k)
        : m_digest(digest),
          m_batch_k(batch_k)
{}

DoLogBatch::~DoLogBatch()
{
    DoLogOp *op;
    for(DoLogOpListIt it = m_op.begin(); it != m_op.end(); ++it) {op = *it; delete op;}
    m_op.clear();    
}       

string DoLogBatch::XmlRedo()
{
    DoLogOp *op;
    stringstream ss;
    for(DoLogOpListIt it = m_op.begin(); it != m_op.end(); ++it)
    {
        op = *it;
        ss << op->XmlRedo();
    }
    return ss.str();
}

string DoLogBatch::XmlUndo()
{
    DoLogOp *op;
    stringstream ss;    
    for(DoLogOpListRevIt it = m_op.rbegin(); it != m_op.rend(); ++it)
    {
        op = *it;
        ss << op->XmlUndo();
    }
    return ss.str();
}

string DoLogBatch::Sql()
{
    DoLogOp *op;
    stringstream ss;
    for(DoLogOpListIt it = m_op.begin(); it != m_op.end(); ++it)
    {
        op = *it;
        ss << op->Sql();
    }
    return ss.str();   
}

////////////////////////////////////////////////////////////////////////////////
// DoLog
////////////////////////////////////////////////////////////////////////////////

DoLog::DoLog() {}

DoLog::~DoLog()
{
    Clean();
}

void DoLog::Clean()
{
    for(DoLogBatchIt it = m_dlt.begin(); it != m_dlt.end(); ++it){delete it->second;}        
    m_dlt.clear();
}

// subclass factory of DoLogOp producting operations stored in indexed batches
DoLogOp *DoLog::SqlOper(DoLogAttValSet batch_k, DoLogOpType op_t, string op_e)
{
    // always new operation to be produced by the factory upon call
    DoLogOp *op = NULL;    
    switch(op_t)
    {
        case INSERT: op = new DoLogOp_Insert(op_e); break;
        case DELETE: op = new DoLogOp_Delete(op_e); break;   
        case UPDATE: op = new DoLogOp_Update(op_e); break;           
        default: throw invalid_argument("Wrong op type, only INSERT, DELETE, UPDATE");
    }           

    // create new or reuse existing batch
    DoLogBatch *batch;              
    string batch_k_digest = batch_k.Digest();
    DoLogBatchIt it = m_dlt.find(batch_k_digest);
    if (it == m_dlt.end())
    { // new operation for a given key
        batch = new DoLogBatch(batch_k_digest, batch_k);        
        m_dlt.insert(pair<string, DoLogBatch *>(batch_k_digest, batch));
    }   
    else
    {   // operation indexed by the key was already found in the map       
        batch = it->second;
    }   

    // the operations must be kept sorted by priority of the entities    
    batch->m_op.push_back(op);  
    
    return op;
}

string DoLog::XmlRedo()
{
    stringstream ss;
    ss << "<REDOLOG>\n";
    for(DoLogBatchIt it = m_dlt.begin(); it != m_dlt.end(); ++it)
    {
        DoLogBatch *batch = it->second;
        ss << "<BATCH>\n";        
        ss << "<DIGEST>" << it->first << "</DIGEST>\n";
        ss << "<KEY>\n";        
        ss << batch->m_batch_k.Xml();
        ss << "</KEY>\n";
        ss << batch->XmlRedo();
        ss << "</BATCH>\n";
    }           
    ss << "</REDOLOG>\n";        
    return ss.str();
}

string DoLog::XmlUndo()
{
    stringstream ss;
    ss << "<UNDOLOG>\n";
    for(DoLogBatchIt it = m_dlt.begin(); it != m_dlt.end(); ++it)
    {
        DoLogBatch *batch = it->second;
        ss << "<BATCH>\n";        
        ss << "<DIGEST>" << it->first << "</DIGEST>\n";
        ss << "<KEY>\n";        
        ss << batch->m_batch_k.Xml();
        ss << "</KEY>\n";
        ss << batch->XmlUndo();
        ss << "</BATCH>\n";
    }           
    ss << "</UNDOLOG>\n";        
    return ss.str();
}

string DoLog::Sql()
{
    stringstream ss;
    for(DoLogBatchIt it = m_dlt.begin(); it != m_dlt.end(); ++it)
    {
        DoLogBatch *batch = it->second;
        ss << batch->Sql();        
    }
    return ss.str();
}
