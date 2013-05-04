#include "engine/animation/SceneTree.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

SceneTree::SceneTree()
: m_uCurrentCount(0)
{
	for(u32 i = 0; i < SceneTree::MaxNodeCount; ++i)
	{
		m_aIndex[i].id = i;
		m_aIndex[i].index = (u32)-1;
		m_aIndex[i].next = i+1;
	}

	m_uFirstFreeIndex = 0;
	m_uLastFreeIndex = SceneTree::MaxNodeCount - 1;
}

//---------------------------------------------------------------------------------------

SceneTree::~SceneTree()
{
	// TODO: call destructors here.
}

//---------------------------------------------------------------------------------------

Handle SceneTree::Add()
{
	ENGINE_ASSERT( m_uCurrentCount < MaxNodeCount, "Array Full" );
	
	// grab first free index data.
	Index &in = m_aIndex[m_uFirstFreeIndex];

	// remove index from free list.
	m_uFirstFreeIndex = in.next;

	//in.id += NEW_OBJECT_ID_ADD;

	// update object count and set index data.
	in.index = m_uCurrentCount++;

	// grab the object.
	Node& node = m_aNode[in.index];
	node.m_parent = Node::InvalidIndex;
	node.m_child = Node::InvalidIndex;
	node.m_next = Node::InvalidIndex;

	// trigger constructor.
	//new (&node) T();

	// and set its handle.
	node.SetHandle(in.id);

	// return handle.
	return in.id;
}

//---------------------------------------------------------------------------------------

bool SceneTree::HasParent( Handle node )
{
	Node& data = Lookup( node );
	return data.m_parent != Node::InvalidIndex;
}

//---------------------------------------------------------------------------------------

bool SceneTree::HasChildren( Handle node )
{
	Node& data = Lookup( node );
	return data.m_child != Node::InvalidIndex;
}

//---------------------------------------------------------------------------------------

bool SceneTree::HasNext( Handle node )
{
	Node& data = Lookup( node );
	return data.m_next != Node::InvalidIndex;
}

//---------------------------------------------------------------------------------------

Handle SceneTree::GetParent( Handle node ) 
{
	ENGINE_ASSERT( HasParent( node ), "Node Has No Parent" );
	
	Node& data = Lookup( node );
	return data.m_parent;
}

//---------------------------------------------------------------------------------------

u32 SceneTree::GetChildCount( Handle node ) const
{
	u32 uChildCount = 0;

	u32 ParentIndex = m_aIndex[node].index;

	const Node& parentNode = Lookup( node );
	u32 LastChildIndex = parentNode.m_child;

	while( LastChildIndex != Node::InvalidIndex )
	{
		uChildCount++;

		LastChildIndex = m_aNode[LastChildIndex].m_next;
	}

	return uChildCount;
}

//---------------------------------------------------------------------------------------

u32 SceneTree::GetOffspringCount( Handle hNode ) const 
{
	u32 uParentIndex = m_aIndex[hNode].index;

	const Node& parentNode = Lookup( hNode );

	if( parentNode.m_child == Node::InvalidIndex )
		return 0;

	u32 uLastChildIndex = parentNode.m_child;

	while( m_aNode[uLastChildIndex].m_next != Node::InvalidIndex )
	{
		uLastChildIndex = m_aNode[uLastChildIndex].m_next;
	}
	
	return uLastChildIndex - uParentIndex + GetOffspringCount( m_aNode[uLastChildIndex].GetHandle() );
}

//---------------------------------------------------------------------------------------

Handle SceneTree::AddChild( Handle hParent )
{
	ENGINE_ASSERT( m_aIndex[hParent].index < m_uCurrentCount, "Invalid Handle" );

	if( m_uCurrentCount >= SceneTree::MaxNodeCount - 1 )
	{
		ENGINE_LOG( "Scene Tree Full: Cannot Add Child Node\n" );
		return Node::InvalidIndex;
	}

	// find parent node index.
	u32 ParentNodeIndex = m_aIndex[hParent].index;

	// find place to add child node.
	u32 InsertIndex = ParentNodeIndex + 1;

	// go past last child.
	while( m_aNode[InsertIndex].m_parent == ParentNodeIndex )
		InsertIndex++;
	
	// increment all index refferences (parent, child, next).
	for( u32 i = 0; i < m_uCurrentCount; ++i )
	{
		// update next refferences.
		if( m_aNode[i].m_next != Node::InvalidIndex && m_aNode[i].m_next >= InsertIndex )
			m_aNode[i].m_next++;

		// update parent refferences.
		if( m_aNode[i].m_parent != Node::InvalidIndex && m_aNode[i].m_parent >= InsertIndex )
			m_aNode[i].m_parent++;

		// update child refferences.
		if( m_aNode[i].m_child != Node::InvalidIndex && m_aNode[i].m_child >= InsertIndex )
			m_aNode[i].m_child++;
	}

	// move all proceeding nodes forward.
	for( u32 i = m_uCurrentCount; i > InsertIndex; --i )
		m_aNode[i] = m_aNode[i-1];

	// update index array.
	for( u32 i = 0; i < SceneTree::MaxNodeCount; ++i )
	{
		if( m_aIndex[i].index >= InsertIndex && m_aIndex[i].index != (u32)-1 )
			m_aIndex[i].index++;
	}

	// increment node count.
	m_uCurrentCount++;

	// update parent node refferences (parent, child, next)
	if( m_aNode[ParentNodeIndex].m_child != Node::InvalidIndex )
	{
		u32 LastChildIndex = m_aNode[ParentNodeIndex].m_child;
		while( m_aNode[LastChildIndex].m_next != Node::InvalidIndex )
			LastChildIndex++;

		m_aNode[LastChildIndex].m_next = InsertIndex;
	}
	else
	{
		m_aNode[ParentNodeIndex].m_child = InsertIndex;
	}

	// set new node refferences (parent, child, next )
	m_aNode[InsertIndex].m_parent = ParentNodeIndex;
	m_aNode[InsertIndex].m_child = Node::InvalidIndex;
	m_aNode[InsertIndex].m_next = Node::InvalidIndex;

	Handle ResultHandle = (u32)-1;

	// figure out the handle.
	{
		// grab first free index data.
		Index &in = m_aIndex[m_uFirstFreeIndex];

		// remove index from free list.
		m_uFirstFreeIndex = in.next;

		//in.id += NEW_OBJECT_ID_ADD;

		// set index data.
		in.index = InsertIndex;

		// grab the object.
		Node& node = m_aNode[in.index];

		// trigger constructor.
		//new (&node) T();

		// and set its handle.
		node.SetHandle(in.id);

		ResultHandle = in.id;
	}

	print();

	// return handle.
	return ResultHandle;
}

//---------------------------------------------------------------------------------------

SceneTree::Node& SceneTree::Lookup( Handle handle )
{
	ENGINE_ASSERT( m_aIndex[handle].index < m_uCurrentCount, "Invalid Handle" );

	return m_aNode[ m_aIndex[handle].index ];
}

//---------------------------------------------------------------------------------------

const SceneTree::Node& SceneTree::Lookup( Handle handle ) const
{
	ENGINE_ASSERT( m_aIndex[handle].index < m_uCurrentCount, "Invalid Handle" );

	return m_aNode[ m_aIndex[handle].index ];
}

//---------------------------------------------------------------------------------------

void SceneTree::CalculateLocalTransformations()
{
	for( u32 index = 0; index < m_uCurrentCount; ++index )
	{
		Node& node = m_aNode[index];
			
		node.m_local.MakeScale(node.m_scale, node.m_scale, node.m_scale);
			
		Matrix4x4 rot;
		node.m_rotation.toMatrix(rot);
			
		node.m_local *= rot;
			
		node.m_local.matrix[03] = node.m_translation.x;
		node.m_local.matrix[07] = node.m_translation.y;
		node.m_local.matrix[11] = node.m_translation.z;
	}
}

//---------------------------------------------------------------------------------------

void SceneTree::CalculateWorldTransformations()
{
	for( u32 index = 0; index < m_uCurrentCount; ++index )
	{
		Node& node = m_aNode[index];
			
		if( node.m_parent != Node::InvalidIndex )
		{
			Node& parent = Lookup( node.m_parent );
				
			node.m_world = parent.m_world;
			node.m_world *= node.m_local;
		}
		else
		{
			node.m_world = node.m_local;
		}
	}
}

//---------------------------------------------------------------------------------------

void SceneTree::print() const
{
	ENGINE_LOG("\nINDEX : |");
	for( u32 i = 0; i < GetNodeCount(); ++i )
		ENGINE_LOG("%2d|", i);
	ENGINE_LOG("\nPARENT: |");
	for( u32 i = 0; i < GetNodeCount(); ++i )
		ENGINE_LOG("%2d|", m_aNode[i].m_parent);
	ENGINE_LOG("\nNEXT  : |");
	for( u32 i = 0; i < GetNodeCount(); ++i )
		ENGINE_LOG("%2d|", m_aNode[i].m_next);
	ENGINE_LOG("\nCHILD : |");
	for( u32 i = 0; i < GetNodeCount(); ++i )
		ENGINE_LOG("%2d|", m_aNode[i].m_child);
	ENGINE_LOG("\n");
}

//---------------------------------------------------------------------------------------

const Vec3& SceneTree::GetLocalTranslation( Handle hNode ) const
{
	const Node& object = Lookup( hNode );

	return object.GetLocalTranslation();
}

//---------------------------------------------------------------------------------------

float SceneTree::GetLocalScale( Handle hNode ) const
{
	const Node& object = Lookup( hNode );

	return object.GetLocalScale();
}

//---------------------------------------------------------------------------------------

const Quaternion& SceneTree::GetLocalRotation( Handle hNode ) const
{
	const Node& object = Lookup( hNode );

	return object.GetLocalRotation();
}

//---------------------------------------------------------------------------------------

SceneTree::Node::Node()
	: m_translation( Vec3::Identity)
	, m_scale(1.f)
	, m_rotation( 0.f, 0.f, 0.f, 0.f )
	, m_local( Matrix4x4::Identity )
	, m_world( Matrix4x4::Identity )
	, m_parent( Node::InvalidIndex )
	, m_next( Node::InvalidIndex )
	, m_child( Node::InvalidIndex )
{

}

//---------------------------------------------------------------------------------------

SceneTree::Node::~Node()
{

}

//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------
