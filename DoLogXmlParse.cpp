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
#include "DoLogXmlParse.hpp" 

extern DoLog dl;    

////////////////////////////////////////////////////////////////////////////////
// Xerrces parser
////////////////////////////////////////////////////////////////////////////////

void XmlParseBatchOpEnt(DOMElement* k_element, string *e_label)
{
    DOMNodeList* k_node_lst = k_element->getChildNodes();
    const XMLSize_t k_node_lst_len = k_node_lst->getLength();
    if ( k_node_lst_len == 0 ) throw(std::runtime_error( "empty XML node list VAL" ));    

    for( XMLSize_t i = 0; i < k_node_lst_len; i++ )
    {
        DOMNode* node = k_node_lst->item(i);
        if (node->getNodeType())
        {
            if (node->getNodeType() == DOMNode::TEXT_NODE ) 
            {
                const XMLCh* ch_value = node->getNodeValue();
                *e_label = XMLString::transcode(ch_value);
                break;
            }
        }
    }
}

void XmlParseBatchOpKeyLabel(DOMElement *v_element, int v_id)
{
    if (!v_element) throw(std::runtime_error( "empty XML element KEY" ));
    string label = XMLString::transcode(v_element->getTagName());
    cout << "XML Parser: XmlParseBatchOpKeyVal: [" << v_id << "] " << label << endl;    
}

string XmlParseBatchOpKeyVal(DOMElement* k_element)
{
    string value;
    
    DOMNodeList* k_node_lst = k_element->getChildNodes();
    const XMLSize_t k_node_lst_len = k_node_lst->getLength();
    if ( k_node_lst_len == 0 ) throw(std::runtime_error( "empty XML node list VAL" ));    

    for( XMLSize_t i = 0; i < k_node_lst_len; i++ )
    {
        DOMNode* node = k_node_lst->item(i);
        if (node->getNodeType())
        {
            if (node->getNodeType() == DOMNode::TEXT_NODE ) 
            {
                const XMLCh* ch_value = node->getNodeValue();
                value = XMLString::transcode(ch_value);
                break;
            }
        }
    }

    return value;
}

void XmlParseBatchOpArg(DOMElement* k_element, DoLogAttValSet *op_arg)
{
    if (!k_element) throw(std::runtime_error( "empty XML element KEY" ));
    string label = XMLString::transcode(k_element->getTagName());
    cout << "XML Parser: XmlParseBatchOpKey: " << label << endl;
    if (label != "KEY" && label != "VALUE") throw(std::runtime_error( "XML document level 3 element not KEY or VALUE" ));    

    // list of column names with values
    DOMNodeList* k_node_lst = k_element->getChildNodes();
    const XMLSize_t k_node_lst_len = k_node_lst->getLength();
    if ( k_node_lst_len == 0 ) throw(std::runtime_error( "empty XML node list KEY" ));    
    for( XMLSize_t i = 0; i < k_node_lst_len; i++ )
    {
        DOMNode* node = k_node_lst->item(i);
        if (node->getNodeType())
        {
            if (node->getNodeType() == DOMNode::ELEMENT_NODE ) 
            {                                                         
                string label = XMLString::transcode(node->getNodeName());
                string value = XmlParseBatchOpKeyVal(dynamic_cast< xercesc::DOMElement* >(node));
                op_arg->Add(new SQL_CHAR(label, value));                
                
                cout << node->getNodeType() << "[" << i << "]" << label << " = " << value << endl;                
            }            
        }
    }    
}

void XmlParseBatchOpInsert(DOMElement* b_o_element, DoLogOpType* op_t)
{
    *op_t = INSERT;
}

void XmlParseBatchOpUpdate(DOMElement* b_o_element, DoLogOpType* op_t)
{
    *op_t = UPDATE;      
}

void XmlParseBatchOpDelete(DOMElement* b_o_element, DoLogOpType* op_t)
{
    *op_t = DELETE;
}

void XmlParseBatchOp(DOMElement* b_o_element, int op_id)
{
    DoLogOpType op_t;
    string op_e_label;
    DoLogAttValSet batch_k, op_key, op_val_before, op_val_after;
    
    if (!b_o_element) throw(std::runtime_error( "empty XML element BATCH" ));
    string label = XMLString::transcode(b_o_element->getTagName());
    cout << "XML Parser: XmlParseBatchOp: [" << op_id << "]" << label << endl;
    if (label == "DIGEST")
    {}
    else if (label == "KEY")
    {
        XmlParseBatchOpArg(b_o_element, &batch_k);         
    }
    else
    {
        if (label == "INSERT")
        {
            XmlParseBatchOpInsert(b_o_element, &op_t);
        }       
        else if (label == "UPDATE")
        {
            XmlParseBatchOpUpdate(b_o_element, &op_t);            
        }
        else if (label == "DELETE")
        {
            XmlParseBatchOpDelete(b_o_element, &op_t);
        }
        else 
        {
            throw(std::runtime_error( "Invalid operation found: " + label));     
        }

        // ENTITY KEY VALUE
        DOMNodeList* o_node_lst = b_o_element->getChildNodes();
        const XMLSize_t o_node_lst_len = o_node_lst->getLength();
        if ( o_node_lst_len == 0 ) throw(std::runtime_error( "empty XML node list OPERATION" ));        
        for( XMLSize_t i = 0; i < o_node_lst_len; i++ )
        {
            DOMNode* node = o_node_lst->item(i);
            if(node->getNodeType() &&  
                node->getNodeType() == DOMNode::ELEMENT_NODE ) 
            {                
                DOMElement* e = dynamic_cast< xercesc::DOMElement* >(node);            
                if (!e) throw(std::runtime_error( "empty XML element OPERATION" ));
                string e_label = XMLString::transcode(e->getTagName());
                cout << "XML Parser: XmlParseBatchOp: [" << op_id << "] " << e_label << endl;
                if (e_label == "ENTITY")
                {
                    XmlParseBatchOpEnt(e, &op_e_label);   
                }       
                else if (e_label == "KEY")
                {
                    XmlParseBatchOpArg(e, &op_key);   
                }
                else if (e_label == "VALUE")
                {
                    XmlParseBatchOpArg(e, &op_val_before);   
                }
                else
                {
                    throw(std::runtime_error( "Invalid operation field tag found: " + e_label));            
                }
            }
        }

        // all needed values collected
        DoLogOp *op = dl.SqlOper(batch_k, op_t, op_e_label);
        op->addKeySet(&op_key);
        op->addValSet(&op_val_before, &op_val_after);
    }
}

void XmlParseBatch(DOMElement* b_element)
{
    if (!b_element) throw(std::runtime_error( "empty XML element BATCH" ));
    string label = XMLString::transcode(b_element->getTagName());
    cout << "XML Parser: XmlParseBatch: " << label << endl;
    if (label != "BATCH") throw(std::runtime_error( "XML document level 2 element not BATCH" ));    

    // DIGEST INSERT UPDATE DELETE 
    DOMNodeList* b_node_lst = b_element->getChildNodes();
    const XMLSize_t b_node_lst_len = b_node_lst->getLength();
    if ( b_node_lst_len == 0 ) throw(std::runtime_error( "empty XML node list BATCH" ));    
    for( XMLSize_t i = 0; i < b_node_lst_len; i++ )
    {
        DOMNode* node = b_node_lst->item(i);
        if(node->getNodeType() &&  
           node->getNodeType() == DOMNode::ELEMENT_NODE ) 
        {                
            XmlParseBatchOp(dynamic_cast< xercesc::DOMElement* >(node), i);            
        }
    }    
}

void XmlParseUndolog(DOMDocument *d)
{
    if( !d ) throw(std::runtime_error( "empty XML document" ));
    DOMElement* d_element = d->getDocumentElement();
    if( !d_element ) throw(std::runtime_error( "empty XML document root element" ));
    string element_dolog = XMLString::transcode(d_element->getTagName());
    cout << "XML Parser: XmlParseUndolog: " << element_dolog << endl;
    if (element_dolog != "UNDOLOG") throw(std::runtime_error( "XML document level 1 root element not UNDOLOG" ));

    // BATCH
    DOMNodeList* c_node_lst = d_element->getChildNodes();
    const XMLSize_t c_node_lst_len = c_node_lst->getLength();
    if ( c_node_lst_len == 0 ) throw(std::runtime_error( "empty XML node list" ));
    
    for( XMLSize_t i = 0; i < c_node_lst_len; i++ )
    {
        DOMNode* node = c_node_lst->item(i);
        if( node->getNodeType() &&  
            node->getNodeType() == DOMNode::ELEMENT_NODE ) 
        {            
            XmlParseBatch(dynamic_cast< xercesc::DOMElement* >(node));            
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// DoLog::XmlParse
////////////////////////////////////////////////////////////////////////////////

void DoLog::XmlParse(const unsigned char *buf, const size_t len)
{
    string doc((char *)buf, (int)len);
    cout << "XML Parser: START-->>\n" << doc << "<<--END" << endl;
    
    XmlDomDocument *xdd = new XmlDomDocument(buf, len);

    // UNDOLOG
    XmlParseUndolog(xdd->getDocument());     

    cout << "OK, no exception, XML document validated" << endl;    
    delete xdd;
}
