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
#include "Backend/AVVMOnlineEncodingUtils.h"

int32 UAVVMOnlineEncodingUtils::DecodeInt32(const int32 Input,
                                            const int32 BitRange,
                                            const int32 RShift)
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

	const int32 BitRange_Clamped = FMath::Clamp(BitRange, 0, 32);
	const int32 Result = ((Input >> RShift) & Recurse(BitRange_Clamped));
	return Result;
}

int32 UAVVMOnlineEncodingUtils::EncodeInt32(const int32 Input,
                                            const int32 BitRange,
                                            const int32 LShift)
{
	if (!ensureAlwaysMsgf(Input >= 0 && (Input < (1 << BitRange)),
	                      TEXT("Invalid Input. Outside provided Range.")))
	{
		return INDEX_NONE;
	}

	const int32 Result = (Input << LShift);
	return Result;
}

TArray<int32> UAVVMOnlineEncodingUtils::SearchValues(const TArray<int32>& Inputs,
                                                     const int32 BitRange,
                                                     const int32 RShift,
                                                     const int32 SearchValue)
{
	const int32 BitRange_Clamped = FMath::Clamp(BitRange, 0, 32);

	TArray<int32> OutResults;
	for (const int32 i : Inputs)
	{
		const int32 Result = UAVVMOnlineEncodingUtils::DecodeInt32(i, BitRange_Clamped, RShift);
		if (Result == SearchValue)
		{
			OutResults.Add(Result);
		}
	}

	return OutResults;
}
