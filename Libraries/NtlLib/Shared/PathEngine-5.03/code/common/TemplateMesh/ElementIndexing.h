//**********************************************************************
//
// Copyright (c) 2005
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef ENCODED_ELEMENT_INDEX_INCLUDED
#define ENCODED_ELEMENT_INDEX_INCLUDED

#include "common/TemplateMesh/MeshElement.h"
#include "common/interface/Assert.h"

template <class tMesh> long 
IndexForElement(typename tMesh::face f)
{
    return f.index() * 3;
}
template <class tMesh> long
IndexForElement(typename tMesh::edge e)
{
    return e.index() * 3 + 1;
}
template <class tMesh> long
IndexForElement(typename tMesh::vertex v)
{
    return v.index() * 3 + 2;
}
template <class tMesh> long
IndexForElement(const cMeshElement<tMesh> element)
{
    switch(element.type())
    {
    default:
        invalid();
    case bMeshElement::FACE:
        return IndexForElement<tMesh>(element.asFace());
    case bMeshElement::EDGE:
        return IndexForElement<tMesh>(element.asEdge());
    case bMeshElement::VERTEX:
        return IndexForElement<tMesh>(element.asVertex());
    }
}

template <class tMesh> cMeshElement<tMesh>
ElementAtIndex(tMesh& mesh, long index)
{
    long type = index % 3;
    index = index / 3;
    switch(type)
    {
    default:
        invalid();
    case 0:
        return mesh.faceAt(index);
    case 1:
        return mesh.edgeAt(index);
    case 2:
        return mesh.vertAt(index);
    }
}

inline long
ElementIndexForTypeAndSubIndex(bMeshElement::eType type, long subIndex)
{
    switch(type)
    {
    default:
        invalid();
    case bMeshElement::FACE:
        return subIndex * 3;
    case bMeshElement::EDGE:
        return subIndex * 3 + 1;
    case bMeshElement::VERTEX:
        return subIndex * 3 + 2;
    }
}

inline void
TypeAndSubIndexForElementIndex(long index, bMeshElement::eType& type, long& subIndex)
{
    long typeIndex = index % 3;
    subIndex = index / 3;
    switch(typeIndex)
    {
    default:
        invalid();
    case 0:
        type = bMeshElement::FACE;
        break;
    case 1:
        type = bMeshElement::EDGE;
        break;
    case 2:
        type = bMeshElement::VERTEX;
        break;
    }
}

template <class tMesh> long
TopElement(tMesh& mesh)
{
    long result = -1;
    if(mesh.vertSize())
    {
        result = (mesh.vertSize() - 1) * 3 + 2;
    }
    if(mesh.edgeSize())
    {
        long candidate = (mesh.edgeSize() - 1) * 3 + 1;
        if(candidate > result)
        {
            result = candidate;
        }
    }
    if(mesh.faceSize())
    {
        long candidate = (mesh.faceSize() - 1) * 3;
        if(candidate > result)
        {
            result = candidate;
        }
    }
    return result;
}

#endif
