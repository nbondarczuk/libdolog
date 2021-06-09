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

using namespace std;

//
// TYPEID - values coherent with Perl Oracle DBI
//
typedef enum SQL_VALUE_TYPE
{
    SQL_CHAR_TYPEID            = 1,
//  SQL_NUMERIC_TYPEID         = 2,
//  SQL_DECIMAL_TYPEID         = 3,
    SQL_INTEGER_TYPEID         = 4,
    SQL_SMALLINT_TYPEID        = 5,
    SQL_FLOAT_TYPEID           = 6,
//  SQL_REAL_TYPEID            = 7,
    SQL_DOUBLE_TYPEID          = 8,
    SQL_DATE_TYPEID            = 9,
//  SQL_TIME_TYPEID            =10,
//  SQL_TIMESTAMP_TYPEID       =11,
    SQL_VARCHAR_TYPEID         =12 

} SQL_VALUE_TYPE;

template <class T>
inline string _2_str_ (const T& t)
{
    stringstream ss;
    ss << t;
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
// SQL_VALUE: representation of sql value, ready to show in XML
////////////////////////////////////////////////////////////////////////////////

class SQL_VALUE
{
protected:
    SQL_VALUE_TYPE m_t;          // mandatory attribute TypeId 
    map <string, string> m_att;  // flexible attributes of the value  
    string m_label;              
    string m_str;
public:
    SQL_VALUE(SQL_VALUE_TYPE t, string label, string v)
            : m_t(t),
              m_label(label),
              m_str(v) {}
    virtual ~SQL_VALUE() {}
    string label() {return m_label;}
    string str() {return m_str;} // type independent format: always string
    virtual string val() =0; // type dependent format
    void att(string att, string att_val) {m_att[att] = att_val;}
    string xml_att_all() {
        stringstream ss;
        ss << " TypeId=" << "\"" << m_t << "\""; // mandatory attribute
        // all optional attributes
        map<string, string>::iterator it = m_att.begin();
        while (it != m_att.end())
        {
            ss << " ";
            ss << it->first
               << "="
               << "\""
               << it->second
               << "\"";
            ++it;
        }
        return ss.str();
    }
    string xml() {
        stringstream ss;
        ss << "<"
           << m_label
           << xml_att_all()
           << ">"
           << m_str
           << "</"
           << m_label
           << ">";
        return ss.str();
    }
    virtual SQL_VALUE* clone() =0; // deep copy of polimorfic object
};

////////////////////////////////////////////////////////////////////////////////
// SQL_CHAR
////////////////////////////////////////////////////////////////////////////////

class SQL_CHAR : public SQL_VALUE
{
public:
    SQL_CHAR(string label, string v) : SQL_VALUE(SQL_CHAR_TYPEID, label, v) {}
    string val() {return "\'" + m_str + "\'";}          
    SQL_CHAR* clone(){return new SQL_CHAR(*this);}
};

////////////////////////////////////////////////////////////////////////////////
// SQL_INTEGER
////////////////////////////////////////////////////////////////////////////////

class SQL_INTEGER : public SQL_VALUE
{
public:
    SQL_INTEGER(string label, int v) : SQL_VALUE(SQL_INTEGER_TYPEID, label, _2_str_(v)) {}
    string val() {return m_str;}
    SQL_INTEGER* clone(){return new SQL_INTEGER(*this);}
};

////////////////////////////////////////////////////////////////////////////////
// SQL_SMALLINT
////////////////////////////////////////////////////////////////////////////////

class SQL_SMALLINT : public SQL_VALUE
{
public:
    SQL_SMALLINT(string label, short v) : SQL_VALUE(SQL_SMALLINT_TYPEID, label, _2_str_(v)) {}
    string val() {return m_str;}    
    SQL_SMALLINT* clone(){return new SQL_SMALLINT(*this);}
};

////////////////////////////////////////////////////////////////////////////////
// SQL_FLOAT
////////////////////////////////////////////////////////////////////////////////

class SQL_FLOAT : public SQL_VALUE
{
public:
    SQL_FLOAT(string label, float v) : SQL_VALUE(SQL_FLOAT_TYPEID, label, _2_str_(v)) {}
    string val() {return m_str;}    
    SQL_FLOAT* clone(){return new SQL_FLOAT(*this);}
};

////////////////////////////////////////////////////////////////////////////////
// SQL_DOUBLE
////////////////////////////////////////////////////////////////////////////////

class SQL_DOUBLE : public SQL_VALUE
{
public:
    SQL_DOUBLE(string label, double v) : SQL_VALUE(SQL_DOUBLE_TYPEID, label, _2_str_(v)) {}
    string val() {return m_str;}    
    SQL_DOUBLE* clone(){return new SQL_DOUBLE(*this);}
};

////////////////////////////////////////////////////////////////////////////////
// SQL_DATE
////////////////////////////////////////////////////////////////////////////////

class SQL_DATE : public SQL_VALUE
{
private:
    string m_format_mask;

public:
    SQL_DATE(string label, string v)
            : SQL_VALUE(SQL_DATE_TYPEID, label, v),
              m_format_mask(string("YYYYMMDDHH24MISS"))
        {       
            SQL_VALUE::att("FormatMask", m_format_mask);
        }
    
    SQL_DATE(string label, string v, string fm)
            : SQL_VALUE(SQL_DATE_TYPEID, label, v),
              m_format_mask(fm)
        {
            SQL_VALUE::att("FormatMask", m_format_mask);
        }        
    string val() {
        stringstream ss;
        string format;
        if (m_format_mask.length() > 0) {
            format = ",\'" + m_format_mask + "\'";
        }
        ss << "TO_DATE(" << "\'" << m_str << "\'" << format << ")";
        return ss.str();
    }    
    SQL_DATE* clone(){return new SQL_DATE(*this);} 
};

////////////////////////////////////////////////////////////////////////////////
// SQL_VARCHAR
////////////////////////////////////////////////////////////////////////////////

class SQL_VARCHAR : public SQL_VALUE
{
public:
    SQL_VARCHAR(string label, string v) : SQL_VALUE(SQL_VARCHAR_TYPEID, label, v) {}
    string val() {return "\'" + m_str + "\'";}
    SQL_VARCHAR* clone(){return new SQL_VARCHAR(*this);}
};
