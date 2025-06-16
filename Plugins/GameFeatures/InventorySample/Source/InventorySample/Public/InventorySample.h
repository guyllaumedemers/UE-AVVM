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

#include "Modules/ModuleManager.h"

/**
 *	Plugin Description :
 *
 *	InventorySample handle collections of items owned by a given Actor. This plugin doesn't restrict itself to the standard inventory
 *	system we usually think about. Any actor could own the component supporting the inventory system and define a set of items to be act upon. A visual
 *	representation of the available set specific to the context at hand would be provided to the local client once interacted with.
 *
 *	Example : A Backpack, Shop or Mystery Chess could all be provided from various sources a collection set of items to store.
 *
 *	A) For the backpack, this would be specific to the local player (Backend account or local save file) and would represent what is currently held
 *	by the player.
 *
 *	B) For the shop, this would be a statically define collection set of items from which the user can buy, sell, trade from. I.e defined by design. In this
 *	case, interaction between the player and the shop would involve two components talking to each other and exchanging data.
 *
 *	C) Same for the Mystery chess.
 */