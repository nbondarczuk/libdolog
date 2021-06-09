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

#include <stdio.h>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <string>

using namespace xercesc;

class XmlDomErrorHandler : public HandlerBase
{
public:
    void fatalError(const SAXParseException &exc)
        {
            printf("Fatal parsing error at line %d\n", (int)exc.getLineNumber());            
        }       
};

class XmlDomDocument
{
private:
    XmlDomDocument();
    XmlDomDocument(const class XmlDOMDocument&); 
    static class XercesDOMParser* s_parser;
    static class ErrorHandler* s_errorHandler;    
    class DOMDocument* m_doc;
    void tryCreateParser()
        {
            if (!s_parser)
            {
                XMLPlatformUtils::Initialize();
                s_parser = new XercesDOMParser();
                s_errorHandler = (ErrorHandler*) new XmlDomErrorHandler();
                s_parser->setErrorHandler(s_errorHandler);
            }
        }

public:    
    XmlDomDocument(const unsigned char* buf, const size_t len)
            : m_doc(NULL)
        {
            tryCreateParser();            
            MemBufInputSource xml_buf(buf, len, "xml (in memory)");            
            s_parser->parse(xml_buf);
            m_doc = s_parser->getDocument();            
        }       
    ~XmlDomDocument()
        {       
            if (m_doc) m_doc->release();
        }
    class DOMDocument* getDocument() { return m_doc; }
};

class XercesDOMParser* XmlDomDocument::s_parser = NULL;
class ErrorHandler* XmlDomDocument::s_errorHandler = NULL;

