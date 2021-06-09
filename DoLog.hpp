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
#include <vector>
#include <map>
#include <list>
#include <stdexcept>  
#include <algorithm>
#include <sstream>
#include <functional>

#include "sql_value.hpp"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// SQL mapping
///////////////////////////////////////////////////////////////////////////////

// DoLogOpType - recordable DB operations 
typedef enum DoLogOpType
{
    INSERT = 0,
    UPDATE = 1,
    DELETE = 2

} DoLogOpType;
// ValState - status of the value, post or pre op value
typedef enum DoLogOpValState
{
    PREOPVAL,
    POSTOPVAL

} DoLogOpValState;

///////////////////////////////////////////////////////////////////////////////
// DoLogAttValSet - set of values with attributes labelling them
///////////////////////////////////////////////////////////////////////////////

typedef vector<SQL_VALUE *>::iterator DoLogAttValSeqIt;
typedef vector<SQL_VALUE *>::const_iterator DoLogAttValSeqConstIt;
class DoLogAttValSet
{
private:
    vector<SQL_VALUE *> m_v_seq;//set of polymorfic sql labelled values with attributes
public:
    DoLogAttValSet();
    ~DoLogAttValSet();
    string Xml();
    void Add(SQL_VALUE *pv);
    string Digest();
    DoLogAttValSet& operator+=(SQL_VALUE *prv);//add new sql labelled value
    DoLogAttValSet& operator=(DoLogAttValSet &rv);//deep copy
    string SqlAttSeq();
    string SqlValSeq();
    string SqlKeyEqCond();
    string SqlValAssign();
};

///////////////////////////////////////////////////////////////////////////////
// DoLogOp - trace of single operation on an entity, superclass with a key
// implementation of the inserface of adding the value is left to subclasses
// the class knows oly how to handle the keys
///////////////////////////////////////////////////////////////////////////////

class DoLogOp
{
private:
    // private abstract interface to be used only by class interface methods
    virtual void AddVal(SQL_VALUE *pv, const DoLogOpValState s) = 0;
protected:
    // key values must be avalable in sub-classes 
    string m_e;
    DoLogAttValSet m_k;
    string m_k_digest;
public:
    DoLogOp(const string e);
    virtual string XmlRedo() = 0;
    virtual string XmlUndo() = 0;
    void AddKey(SQL_VALUE *pv);
    DoLogOp& operator+=(SQL_VALUE *prv); // add key value
    virtual DoLogOp& operator*=(SQL_VALUE *prv) = 0; // add value PREOP or POSTOP
    virtual DoLogOp& operator/=(SQL_VALUE *prv) = 0; // add value POSTOP
    void addKeySet(DoLogAttValSet *p);
    virtual void addValSet(DoLogAttValSet *p_lv_before, DoLogAttValSet *p_lv_after) = 0;
    virtual string Sql() = 0;
};

///////////////////////////////////////////////////////////////////////////////
// specific fields for subclass of Op: Insert (subclass with a value)
///////////////////////////////////////////////////////////////////////////////

class DoLogOp_Insert: public DoLogOp
{
private:
    void AddVal(SQL_VALUE *pv, const DoLogOpValState s);
protected:
    DoLogAttValSet m_lv_after;        
public:
    DoLogOp_Insert(const string e);
    string XmlRedo();
    string XmlUndo();
    DoLogOp& operator*=(SQL_VALUE *prv); // add value PREOP or POSTOP
    DoLogOp& operator/=(SQL_VALUE *prv); // add value POSTOP    
    void addValSet(DoLogAttValSet *p_lv_before, DoLogAttValSet *p_lv_after);
    string Sql();
};      

///////////////////////////////////////////////////////////////////////////////
// specific fields for subclass of Op: Delete (subclass with a value)
///////////////////////////////////////////////////////////////////////////////

class DoLogOp_Delete : public DoLogOp
{
private:
    void AddVal(SQL_VALUE *pv, const DoLogOpValState s);
protected:
    DoLogAttValSet m_lv_before;    
public:
    DoLogOp_Delete(const string e);
    string XmlRedo();
    string XmlUndo();
    DoLogOp& operator*=(SQL_VALUE *prv); // add value PREOP or POSTOP
    DoLogOp& operator/=(SQL_VALUE *prv); // add value POSTOP    
    void addValSet(DoLogAttValSet *p_lv_before, DoLogAttValSet *p_lv_after);
    string Sql();
};

///////////////////////////////////////////////////////////////////////////////
// specific fields for subclass of Op: Update (subclass with a value)
///////////////////////////////////////////////////////////////////////////////

class DoLogOp_Update : public DoLogOp
{
private:
    void AddVal(SQL_VALUE *v, const DoLogOpValState s);
protected:
    DoLogAttValSet m_lv_before;
    DoLogAttValSet m_lv_after;
public:
    DoLogOp_Update(const string e);
    string XmlRedo();
    string XmlUndo();
    DoLogOp& operator*=(SQL_VALUE *prv); // add value PREOP or POSTOP
    DoLogOp& operator/=(SQL_VALUE *prv); // add value POSTOP    
    void addValSet(DoLogAttValSet *p_lv_before, DoLogAttValSet *p_lv_after);
    string Sql();
};        

///////////////////////////////////////////////////////////////////////////////
// batch of db operations done in a sequential order, factory has access to data
///////////////////////////////////////////////////////////////////////////////

typedef list<DoLogOp *>::iterator DoLogOpListIt;                // REDO order
typedef list<DoLogOp *>::reverse_iterator DoLogOpListRevIt;     // UNDO order
class DoLogBatch
{
    friend class DoLog;
private:
    string m_digest;
    DoLogAttValSet m_batch_k;
    list <DoLogOp *> m_op; // order of adding the operation must be kept
    int flushed_ops_no;
public:
    DoLogBatch(const string digest, const DoLogAttValSet batch_k);
    ~DoLogBatch();
    string XmlRedo();
    string XmlUndo();
    string Sql();
};    

///////////////////////////////////////////////////////////////////////////////
// XML logger: set of db operations regiested on digest key stored in batches
///////////////////////////////////////////////////////////////////////////////

typedef map<string, DoLogBatch *>::iterator DoLogBatchIt;
class DoLog
{
private:        
    map <string, DoLogBatch *> m_dlt;// dictionary of digest of keys of batches
    int m_flushed_batch_no;
    bool DbLongVarcharLoad(int seqno, int image_len);
    bool DbLongVarcharInsert(string image, string digest);
public:
    DoLog();    
    ~DoLog();
    void Clean();
    class DoLogOp *SqlOper(const DoLogAttValSet k, const DoLogOpType t, const string e); // factory
    string XmlRedo();
    string XmlUndo();
    void XmlParse(const unsigned char *buf, const size_t len);
    int DbSave();
    int DbLoad();
    bool DbCreateDoLogTab();
    string Sql();
    void SqlApply();
};                      
               
