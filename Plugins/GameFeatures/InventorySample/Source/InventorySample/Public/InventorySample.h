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

INVENTORYSAMPLE_API DECLARE_LOG_CATEGORY_EXTERN(LogInventorySample, Log, All);

/**
 *	Plugin Description :
 *
 *	InventorySample is a system that define specific details about what an Inventory would look like in a game. Traditional inventory system
 *	seems to be generally answering a single need, define what the UPlayer holds on them. But, as I'm thinking about it, the general use of this
 *	system can be broader.
 *
 *		Here's how...
 *
 *		This plugin doesn't restrict itself to the standard inventory system we usually think about, any AActor could own the UActorComponent supporting
 *		the inventory system, which means any Actor could hold a collection of Items i.e AShop, AMysteryChess, AForge, etc...
 *		and they could all have data driven collection of items (or dynamic if provided from backend services).
 *
 *	This system has the benefit of allowing two components of similar type to exchange content, allow actions like trading, buying, etc... It unify
 *	the api under a single roof, removing the need for systems like Shop/Forge/etc... and simplify how the UI system would request the relevant data for multi-context display.
 *
 *		Example :
 *
 *			* Displaying side-by-side AShop items and UPlayer 1 inventory.
 *			* Displaying side-by-side UPlayer 1 inventory and UPlayer 1 Backup Storage.
 *			* Displaying side-by-side UPlayer 1 inventory and UPlayer 2 inventory.
 *			* or just displaying UPlayer 1 inventory in a singular window context.
 *
 *	Note : This system expect to make direct calls to the HandshakeSample GFP for running validations!
 *	
 *			IMPORTANT : DO NOT IMPLEMENT AN ITEM PROGRESSION SYSTEM! (This case is handled for you.)
 *	
 *	Note : Item Progression are managed via the Item AttributeSet, and the reference Data Table it's initialized from!
 */
