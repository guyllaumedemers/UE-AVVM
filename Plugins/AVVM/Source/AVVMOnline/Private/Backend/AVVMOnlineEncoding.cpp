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
#include "Backend/AVVMOnlineEncoding.h"

int32 UAVVMOnlineEncodingUtils::DecodeInt32(const int32 Input, const int32 BitRange, const int32 RShift)
{
	TFunction<int32(const int32 NewInput)> Recurse;
	Recurse = [&](const int32 NewInput)
	{
		if (NewInput == 0)
		{
			return 1;
		}
		else
		{
			return (1 << NewInput) + Recurse(NewInput - 1);
		}
	};

	const int32 Result = ((Input >> RShift) & Recurse(BitRange));
	return Result;
}

// @gdemers ignore this
void Test()
{
	// @gdemers first thing to do is validate bounds of the input.
	// so according to what data we are trying to inject, we need to validate
	// if we are within bounds, otherwise assert.
	int32 ElementId = 10;
	
	// @gdemers has to also check position bounds.
	int32 ElementPositionInStorage = 0;
	
	// @gdemers has to check count.
	int32 ElementCount = 1;
	
	// @gdemers we also need to validate the bounds of this input based on our storage encoding.
	int32 StorageId = 01;
	int32 StorageIdBounds = (1 << 3);
	int32 StorageSize = (1 << 6);

	int32 Combined = ElementId
			+ (StorageId << 12)
			+ (ElementPositionInStorage << 15)
			+ (ElementCount << 21);
}
