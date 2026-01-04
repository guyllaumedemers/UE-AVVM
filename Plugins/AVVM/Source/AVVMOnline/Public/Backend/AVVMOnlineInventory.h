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
// @gdemers item
#define GET_ITEM_ID_ENCODING_BIT_RANGE (10)
#define GET_ITEM_ID_ENCODING_RSHIFT (0)
// @gdemers item position
#define GET_ITEM_POSITION_ENCODING_BIT_RANGE (6)
#define GET_ITEM_POSITION_ENCODING_RSHIFT (10)
// @gdemers item count
#define GET_ITEM_COUNT_ENCODING_BIT_RANGE (5)
#define GET_ITEM_COUNT_ENCODING_RSHIFT (16)
// @gdemers storage
#define GET_STORAGE_ID_ENCODING_BIT_RANGE (3)
#define GET_STORAGE_ID_ENCODING_RSHIFT (21)
// @gdemers attachment
#define GET_ATTACHMENT_ID_ENCODING_BIT_RANGE (8)
#define GET_ATTACHMENT_ID_ENCODING_RSHIFT (24)
// @gdemers passive items that are direct child of character require their bits encoding to set the first 10 bits to the max value
// so to allow validation during the attachment process.
#define CHECK_CHARACTER_DEPENDENT_ENCODING (1 << 10)
#endif
