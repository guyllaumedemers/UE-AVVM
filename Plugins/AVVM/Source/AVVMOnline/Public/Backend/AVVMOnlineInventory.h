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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

// @gdemers Inventory Items are referenced by {FAVVMPlayerResource}. More advance encoding are put in place to parse information
// within the {FAVVMPlayerProfile::InventoryIds}. The preprocessors available below are symbols defining the constraints of the bits encoding used.
#ifdef AVVMONLINE_USE_DEFAULT_INVENTORY_ENCODING
// @gdemers RELATIONSHIP Bitmask (7)
// storage		000
// attachment	001
// character	010
// item			100
// ---------------
// attachment, storage dependent	001
// attachment, item dependent		101
// attachment, character dependent	011
// item, storage dependent			100
// item, character dependent		110
#define GET_ELEMENT_RELATIONSHIP_BIT_RANGE (2)
#define GET_ELEMENT_RELATIONSHIP_RSHIFT (0)
// @gdemers item or attachment global unique id (255) (based on RELATIONSHIP value, we offset an attachment global id)
#define GET_ELEMENT_VIRTUAL_GLOBAL_ID_BIT_RANGE (7)
#define GET_ELEMENT_VIRTUAL_GLOBAL_ID_RSHIFT (3)
// @gdemers item or attachment id to identify duplicated instances (63) (solve the problem of ownership, and attachment composition)
#define GET_ELEMENT_INSTANCED_ID_BIT_RANGE (5)
#define GET_ELEMENT_INSTANCED_ID_RSHIFT (11)
// @gdemers storage referencing (7)
#define GET_STORAGE_VIRTUAL_GLOBAL_ID_BIT_RANGE (2)
#define GET_STORAGE_VIRTUAL_GLOBAL_ID_RSHIFT (17)
// @gdemers entry position within storage (31)
#define GET_STORAGE_POSITION_BIT_RANGE (4)
#define GET_STORAGE_POSITION_RSHIFT (20)
// @gdemers entry stack count (127)
#define GET_ELEMENT_STACK_COUNT_BIT_RANGE (6)
#define GET_ELEMENT_STACK_COUNT_RSHIFT (25)
// @gdemers check relationship
#define FILTER_CHARACTER_RELATIONSHIP_BIT (2/*2^1*/)
#endif

#ifdef AVVMONLINE_USE_DEFAULT_INVENTORY_DATATABLE_BASE_ADDRESSING
// @gdemers addressing offset we expect implementers of the Data Table to use when defining global id.
#define GET_STORAGE_PHYSICAL_ADDRESSING_OFFSET (1000)
#define GET_ITEM_PHYSICAL_ADDRESSING_OFFSET (2000)
#define GET_ATTACHMENT_PHYSICAL_ADDRESSING_OFFSET (3000)
#endif

// @gdemers element lookup for supporting the socketing process -- identify dependencies between an attachment and an owner (which may have more than one instance)
#ifdef AVVMONLINE_USE_DEFAULT_INVENTORY_LOOKUP_ENCODING
// @gdemers physical id that represent the element we are evaluating dependencies for
#define GET_ELEMENT_LOOKUP_PHYSICAL_GLOBAL_ID_BIT_RANGE (7)
#define GET_ELEMENT_LOOKUP_PHYSICAL_GLOBAL_ID_RSHIFT (0)
// @gdemers the instance id that uniquely identify 'this' element
#define GET_ELEMENT_LOOKUP_INSTANCED_ID_BIT_RANGE (5)
#define GET_ELEMENT_LOOKUP_INSTANCED_ID_RSHIFT (8)
// @gdemers the virtual id of a dependency that reference our element we evaluate
#define GET_ELEMENT_LOOKUP_OWNER_VIRTUAL_GLOBAL_ID_BIT_RANGE (7)
#define GET_ELEMENT_LOOKUP_OWNER_VIRTUAL_GLOBAL_ID_RSHIFT (14)
// @gdemers the instance id of a dependency that reference our element we evaluate
#define GET_ELEMENT_LOOKUP_OWNER_INSTANCED_ID_BIT_RANGE (5)
#define GET_ELEMENT_LOOKUP_OWNER_INSTANCED_ID_RSHIFT (22)
#endif
