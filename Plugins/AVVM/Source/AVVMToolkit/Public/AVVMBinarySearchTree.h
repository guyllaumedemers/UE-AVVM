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
 *	@gdemers constraint against storing container types within the Binary Tree.
 */
template <typename T>
constexpr bool TIsContainer = TIsTArray<T>::Value || TIsTMap<T>::Value || TIsTSet<T>::Value;

template <typename T>
constexpr bool TIsNotContainer = !TIsContainer<T>;

/**
 *	@gdemers compile time evaluation of WeakOrStrong Object usage for compile time branching.
 */
template <typename T>
concept TIsWeakOrStrongObjectPtrType = requires(TIsDerivedFrom<T, UObject> Obj)
{
	Obj.IsValid();
};

/**
 *  @gdemers constraint against untracked TObjectPtr usage.
 */
template <typename T>
concept TIsObjectPtrType = requires(TIsDerivedFrom<T, UObject> Obj)
{
	Obj.GetPtrTypeHash();
};

template <typename T>
concept TIsNotObjectPtrType = !TIsObjectPtrType<T>;

/**
 *	Class description:
 *	
 *	FAVVMBinaryTreeNode is a template class that encapsulate generic type for binary traversal.
 */
template <typename InElementType> requires(TIsNotObjectPtrType<InElementType> && TIsNotContainer<InElementType>)
struct FAVVMBinaryTreeNode
{
	using TBinaryTreeNodeType = FAVVMBinaryTreeNode<InElementType>;

	TBinaryTreeNodeType* Lhs = nullptr;
	TBinaryTreeNodeType* Rhs = nullptr;
	InElementType Value{};
};

/**
 *	Class description:
 *	
 *	FAVVMBinaryTree is a template class that manage a binary search tree.
 */
template <typename InElementType, typename InAllocatorType>
struct FAVVMBinaryTree
{
	using TBinaryTreeNodeType = FAVVMBinaryTreeNode<InElementType>;
	using TAllocatorType = typename InAllocatorType::template ForElementType<TBinaryTreeNodeType>;

	template <typename TOnSelectElement,
	          typename TIsElementLessThan,
	          typename TOnPushNewElement>
	TBinaryTreeNodeType* Push(const TOnSelectElement& OnSelectElement,
	                          const TIsElementLessThan& IsElementLessThan,
	                          const TOnPushNewElement& OnPushNewElement);

	template <typename TOnSelectElement,
	          typename TIsElementLessThan>
	bool Pop(const TOnSelectElement& OnSelectElement,
	         const TIsElementLessThan& IsElementLessThan);

	template <typename TOnSelectElement,
	          typename TIsElementLessThan>
	TBinaryTreeNodeType** SearchV2(const TOnSelectElement& OnSelectElement,
	                               const TIsElementLessThan& IsElementLessThan) const;

	template <typename TSumFunc>
	void SumV2(const TSumFunc& Functor) const;

	bool IsEmpty() const;

private:
	template <typename TOnSelectElement,
	          typename TIsElementLessThan>
	TBinaryTreeNodeType** SearchV1(TBinaryTreeNodeType*& CurrNode,
	                               const TOnSelectElement& OnSelectElement,
	                               const TIsElementLessThan& IsElementLessThan) const;

	template <typename TSumFunc>
	void SumV1(TBinaryTreeNodeType* CurrNode,
	           const TSumFunc& Functor) const;
	
	TAllocatorType MemBlock{};
	int32 FreeIndex{INDEX_NONE};
	TBinaryTreeNodeType* Root = nullptr;
};

template <typename InElementType, typename InAllocatorType>
template <typename TOnSelectElement, typename TIsElementLessThan, typename TOnPushNewElement>
typename FAVVMBinaryTree<InElementType, InAllocatorType>::TBinaryTreeNodeType* FAVVMBinaryTree<InElementType, InAllocatorType>::Push(const TOnSelectElement& OnSelectElement,
                                                                                                                                     const TIsElementLessThan& IsElementLessThan,
                                                                                                                                     const TOnPushNewElement& OnPushNewElement)
{
	FreeIndex = FMath::Clamp(FreeIndex, 0, INT32_MAX);
	auto** SearchResult = SearchV1(Root, OnSelectElement, IsElementLessThan);
	if (ensureAlwaysMsgf(SearchResult != nullptr, TEXT("Failed to find valid address.")) && (*SearchResult == nullptr))
	{
		void* Ptr = (char*)MemBlock.GetAllocation() + (sizeof(TBinaryTreeNodeType) * FreeIndex);
		(*SearchResult) = new(Ptr) TBinaryTreeNodeType;
		(*SearchResult)->Value = OnPushNewElement();
		FreeIndex += 1;
		return (*SearchResult);
	}
	else if (SearchResult != nullptr)
	{
		return *SearchResult;
	}
	else
	{
		return nullptr;
	}
}

template <typename InElementType, typename InAllocatorType>
template <typename TOnSelectElement, typename TIsElementLessThan>
bool FAVVMBinaryTree<InElementType, InAllocatorType>::Pop(const TOnSelectElement& OnSelectElement,
                                                          const TIsElementLessThan& IsElementLessThan)
{
	auto** SearchResult = SearchV1(Root, OnSelectElement, IsElementLessThan);
	if (!ensureAlwaysMsgf(SearchResult != nullptr, TEXT("Failed to find valid address.")))
	{
		return false;
	}

	if (((*SearchResult)->Lhs == nullptr) && ((*SearchResult)->Rhs == nullptr))
	{
		FMemory::Memzero((*SearchResult), sizeof(TBinaryTreeNodeType));
		return true;
	}

	if ((*SearchResult)->Lhs != nullptr)
	{
		// @gdemers we have found the node to pop, but have to relocate its lhs since the target node
		// will be overwritten by the rhs child.
		auto** Temp = &((*SearchResult)->Rhs);
		while ((Temp != nullptr) && ((*Temp) != nullptr))
		{
			Temp = &((*Temp)->Lhs);
		}

		// @gdemers relocate lhs, & swap curr with rhs.
		void* Ptr = (char*)MemBlock.GetAllocation() + (sizeof(TBinaryTreeNodeType) * FreeIndex);
		(*Temp) = new(Ptr) TBinaryTreeNodeType;
		// TODO @gdemers Theres a memory issue here, and problem with cache locality. When swapping memory, existing allocation
		// still persist, and cannot be reclaimed due to the index based tracking accessing the Allocated type used.
		// -> Make require api changes to support cache affinity, and reclaim zeroed out memory.
		FMemory::Memswap((*SearchResult)->Lhs, (*Temp), sizeof(TBinaryTreeNodeType));
		++FreeIndex;
	}

	auto* RequireZeroed = (*SearchResult)->Rhs;
	FMemory::Memswap((*SearchResult), (*SearchResult)->Rhs, sizeof(TBinaryTreeNodeType));
	FMemory::Memzero(RequireZeroed, sizeof(TBinaryTreeNodeType));
	return true;
}

template <typename InElementType, typename InAllocatorType>
template <typename TOnSelectElement, typename TIsElementLessThan>
typename FAVVMBinaryTree<InElementType, InAllocatorType>::TBinaryTreeNodeType** FAVVMBinaryTree<InElementType, InAllocatorType>::SearchV2(const TOnSelectElement& OnSelectElement,
                                                                                                                                          const TIsElementLessThan& IsElementLessThan) const
{
	auto* NonConstPtr = Root;
	return SearchV1(NonConstPtr/*Cant pass Root in a const api call here*/, OnSelectElement, IsElementLessThan);
}

template <typename InElementType, typename InAllocatorType>
template <typename TSumFunc>
void FAVVMBinaryTree<InElementType, InAllocatorType>::SumV2(const TSumFunc& Functor) const
{
	SumV1(Root, Functor);
}

template <typename InElementType, typename InAllocatorType>
template <typename TOnSelectElement, typename TIsElementLessThan>
typename FAVVMBinaryTree<InElementType, InAllocatorType>::TBinaryTreeNodeType** FAVVMBinaryTree<InElementType, InAllocatorType>::SearchV1(TBinaryTreeNodeType*& CurrNode,
                                                                                                                                          const TOnSelectElement& OnSelectElement,
                                                                                                                                          const TIsElementLessThan& IsElementLessThan) const
{
	if constexpr (TIsWeakOrStrongObjectPtrType<InElementType>)
	{
		if ((CurrNode == nullptr) || !CurrNode->Value.IsValid() || OnSelectElement(CurrNode))
		{
			return &CurrNode;
		}
	}
	else
	{
		if ((CurrNode == nullptr) || OnSelectElement(CurrNode))
		{
			return &CurrNode;
		}
	}

	const bool bResult = IsElementLessThan(CurrNode->Value);
	if (bResult)
	{
		return SearchV1(CurrNode->Lhs,
		                OnSelectElement,
		                IsElementLessThan);
	}
	else
	{
		return SearchV1(CurrNode->Rhs,
		                OnSelectElement,
		                IsElementLessThan);
	}
}

template <typename InElementType, typename InAllocatorType>
template <typename TSumFunc>
void FAVVMBinaryTree<InElementType, InAllocatorType>::SumV1(TBinaryTreeNodeType* CurrNode,
                                                            const TSumFunc& Functor) const
{
	if (CurrNode != nullptr)
	{
		Functor(CurrNode->Value);
		SumV1(CurrNode->Lhs, Functor);
		SumV1(CurrNode->Rhs, Functor);
	}
}

template <typename InElementType, typename InAllocatorType>
bool FAVVMBinaryTree<InElementType, InAllocatorType>::IsEmpty() const
{
	return (Root == nullptr);
}
