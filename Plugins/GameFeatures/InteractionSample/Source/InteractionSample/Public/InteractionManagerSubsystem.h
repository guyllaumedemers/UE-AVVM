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

#include "Subsystems/WorldSubsystem.h"

#include "InteractionManagerSubsystem.generated.h"

/**
 *	Class description:
 *	
 *	UInteractionManagerSubsystem is a subsystem that builds up a graph of UActorInteractionComponent clusters based on proximity
 *	between elements at Runtime, and provide user feedback when interacting with a given entity in World in the form of a list.
 *	
 *	Since actor overlaps are common during gameplay, clustered elements become hard to pick selectively. A solution to this issue
 *	reside in creating cluster of elements, and allowing interaction based on priority.
 *	
 *	By graphing our interactable, we can allow creation of this list (ordered by priority),
 *	and allow quick action on the user part.
 */
UCLASS()
class INTERACTIONSAMPLE_API UInteractionManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	/*
	 *	TODO @gdemers Make all Component register with this subsystem. Create a graph of clusters
	 *  by proximity, and have use attempting an interaction with a given elements display the owning custer
	 *  in a list on the UI.
	 */
};
