//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "IECore/CompoundDataBase.h"
#include "IECore/TypedData.inl"

#include <iostream>
using namespace std;
using namespace IECore;

namespace IECore
{

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( CompoundDataBase, CompoundDataBaseTypeId );

template<>
void CompoundDataBase::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );
	const CompoundDataMap &data = readable();
	accumulator.accumulate( data.size() * sizeof( CompoundDataMap::value_type ) );
	
	CompoundDataMap::const_iterator iter = data.begin();
	while (iter != data.end())
	{
		accumulator.accumulate( iter->second );		
		iter++;
	}	
}

template<>
void CompoundDataBase::copyFrom( const Object *other, CopyContext *context )
{
	Data::copyFrom( other, context );
	const CompoundDataBase *tOther = static_cast<const CompoundDataBase *>( other );
	CompoundDataMap &data = writable();
	data.clear();
	const CompoundDataMap &otherData = tOther->readable();
	for( CompoundDataMap::const_iterator it = otherData.begin(); it!=otherData.end(); it++ )
	{
		data[it->first] = context->copy<Data>( it->second );
	}
}

template<>
bool CompoundDataBase::isEqualTo( const Object *other ) const
{
	if( !Data::isEqualTo( other ) )
	{
		return false;
	}
	const CompoundDataBase *tOther = static_cast<const CompoundDataBase *>( other );
	const CompoundDataMap &m1 = readable();
	const CompoundDataMap &m2 = tOther->readable();
	if( m1.size()!=m2.size() )
	{
		return false;
	}
	CompoundDataMap::const_iterator it1 = m1.begin();
	CompoundDataMap::const_iterator it2 = m2.begin();
	while( it1!=m1.end() )
	{
		if( it1->first!=it2->first )
		{
			return false;
		}
		if( ! it1->second->isEqualTo( it2->second ) )
		{
			return false;
		}
		it1++;
		it2++;
	}
	return true;
}

template<>
void CompoundDataBase::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );
	container->mkdir( "members" );
	container->chdir( "members" );
		const CompoundDataMap &m = readable();
		CompoundDataMap::const_iterator it;
		for( it=m.begin(); it!=m.end(); it++ )
		{
			context->save( it->second, container, it->first );
		}
	container->chdir( ".." );
}

template<>
void CompoundDataBase::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = 0;
	IndexedIOInterfacePtr container = 0;
	try
	{
		container = context->container( staticTypeName(), v );
	}
	catch( const IOException &e )
	{
		// probably a file with CORTEX_MAJOR_VERSION < 5, get the
		// data from CompoundData container instead.
		container = context->container( "CompoundData", v );
	}
	
	CompoundDataMap &m = writable();
	m.clear();
	container->chdir( "members" );
		IndexedIO::EntryList members = container->ls();
		IndexedIO::EntryList::const_iterator it;
		for( it=members.begin(); it!=members.end(); it++ )
		{
			m[it->id()] = context->load<Data>( container, it->id() );
		}
	container->chdir( ".." );
}

IE_CORE_DEFINETYPEDDATANOBASESIZE( CompoundDataBase )

}

// Instantiate that bad boy.
template class TypedData<CompoundDataMap>;