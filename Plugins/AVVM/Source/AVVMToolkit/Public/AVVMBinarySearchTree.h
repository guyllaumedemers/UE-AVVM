//Copyright(c) 2025 gdemers
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
template<typename InElementType> // convert to using concepts later
struct FAVVMBinaryTreeNode
{
	using TBinaryTreeNodeType = FAVVMBinaryTreeNode<InElementType>;
	
	TBinaryTreeNodeType* Lhs = nullptr;
	TBinaryTreeNodeType* Rhs = nullptr;
	InElementType Value{};
};

/**
 * 
 */
template <typename InElementType, typename InAllocatorType>
struct FAVVMBinaryTree
{
	using TBinaryTreeNodeType = FAVVMBinaryTreeNode<InElementType>;
	using TAllocatorType = typename InAllocatorType::template ForElementType<TBinaryTreeNodeType>;
	
	template<typename TSelectNodePredicate, typename TGoLeftPredicate>
	TBinaryTreeNodeType* Search(TSelectNodePredicate Select, TGoLeftPredicate GoLeft);
	
	template<typename TSumFunc>
	void Sum(TSumFunc Functor) const;
	
	bool IsEmpty() const;

private:
	TAllocatorType MemBlock{};
	int32 FreeIndex{INDEX_NONE};
	TBinaryTreeNodeType* Root = nullptr;
};

template <typename InElementType, typename InAllocatorType>
template <typename TSelectNodePredicate, typename TGoLeftPredicate>
typename FAVVMBinaryTree<InElementType, InAllocatorType>::TBinaryTreeNodeType* FAVVMBinaryTree<InElementType, InAllocatorType>::Search(TSelectNodePredicate Select,
                                                                                                                                       TGoLeftPredicate GoLeft)
{
	TFunction<TBinaryTreeNodeType*(TBinaryTreeNodeType* CurrNode,
	                               TSelectNodePredicate NewSelect,
	                               TGoLeftPredicate NewGoLeft)> BST_Recurse{};

	BST_Recurse = [&](TBinaryTreeNodeType* CurrNode,
	                  TSelectNodePredicate NewSelect,
	                  TGoLeftPredicate NewGoLeft)
	{
		if (CurrNode == nullptr)
		{
			void* Ptr = (char*)MemBlock.GetAllocation() + (sizeof(TBinaryTreeNodeType) * FreeIndex);
			void* LhsPtr = (char*)MemBlock.GetAllocation() + (sizeof(TBinaryTreeNodeType) * (FreeIndex + 1));
			void* RhsPtr = (char*)MemBlock.GetAllocation() + (sizeof(TBinaryTreeNodeType) * (FreeIndex + 2));
			CurrNode = new(Ptr) TBinaryTreeNodeType;
			CurrNode->Lhs = new(LhsPtr) TBinaryTreeNodeType;
			CurrNode->Rhs = new(RhsPtr) TBinaryTreeNodeType;
			FreeIndex += 3;
			return CurrNode;
		}
		else if (!CurrNode->Value.IsValid()/*TODO we need a requirements to a function that validate the value type*/ || NewSelect(CurrNode->Value))
		{
			return CurrNode;
		}

		const bool bResult = NewGoLeft(CurrNode->Value);
		if (bResult)
		{
			return BST_Recurse(CurrNode->Lhs,
			                   NewSelect,
			                   NewGoLeft);
		}
		else
		{
			return BST_Recurse(CurrNode->Rhs,
			                   NewSelect,
			                   NewGoLeft);
		}
	};

	FreeIndex = FMath::Clamp(FreeIndex, 0, INT32_MAX);
	auto* SearchResult = BST_Recurse(Root, Select, GoLeft);
	if (Root == nullptr)
	{
		Root = SearchResult;
	}

	return SearchResult;
}

template <typename InElementType, typename InAllocatorType>
template <typename TSumFunc>
void FAVVMBinaryTree<InElementType, InAllocatorType>::Sum(TSumFunc Functor) const
{
	TFunction<void(TBinaryTreeNodeType* CurrNode)> BST_Recurse{};
	BST_Recurse = [&](TBinaryTreeNodeType* CurrNode)
	{
		if (CurrNode == nullptr)
		{
			return;
		}
		
		Functor(CurrNode->Value);
		BST_Recurse(CurrNode->Lhs);
		BST_Recurse(CurrNode->Rhs);
	};
	
	BST_Recurse(Root);
}

template <typename InElementType, typename InAllocatorType>
bool FAVVMBinaryTree<InElementType, InAllocatorType>::IsEmpty() const
{
	return (Root == nullptr);
}
