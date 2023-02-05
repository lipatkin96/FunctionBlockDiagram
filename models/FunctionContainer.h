#ifndef FUNCTIONCONTAINER_H
#define FUNCTIONCONTAINER_H

#include "GraphStruct.h"

#include <QHash>
#include <QList>

class FunctionContainer
{
public:
    int add( SFunctionNode node )
    {
        node.id = m_availableId++;
        m_funcMap.insert( node.id, node );
        return node.id;
    }

    QList< SFunctionNode > toList() const
    {
        return m_funcMap.values();
    }

    size_t size() const { return m_funcMap.size(); }

    SFunctionNode & operator[]( int i ) { return *( m_funcMap.find( i ) ); }
    SFunctionNode operator[]( int i ) const { return *( m_funcMap.find( i ) ); }

private:
    QHash< int, SFunctionNode >     m_funcMap;
    int                             m_availableId = 0;
};


#endif // FUNCTIONCONTAINER_H
