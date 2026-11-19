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
#include "FastTransactionObject.h"

#include "AVVMLogger.h"
#include "AVVMToolkitModule.h"
#include "Engine/Engine.h"

bool FLightweightTransactionObject::operator==(const FLightweightTransactionObject& Rhs) const
{
	return (PlayerTransactions.GetGameplayTagStackObjects() == Rhs.PlayerTransactions.GetGameplayTagStackObjects());
}

int32 FLightweightTransactionObjectFastArray::Modify(const FUniqueNetIdRepl& PlayerUniqueId,
                                                     const FGameplayTag& Tag,
                                                     const int32 DeltaValue,
                                                     const bool bHasAuthority)
{
	FLightweightTransactionObject** SearchResult = FastLookup.Find(PlayerUniqueId);
	if ((SearchResult == nullptr) && bHasAuthority)
	{
		MarkArrayDirty();

		auto& Value = PerPlayerTransactions.Add_GetRef(FLightweightTransactionObject{});
		Value.PlayerTransactions.Modify(Tag, DeltaValue, bHasAuthority);
		FastLookup.Add(PlayerUniqueId, &Value);
	}
	else if (SearchResult == nullptr)
	{
		AVVM_LOGGER_LOG(LogToolkit,
		                GEngine,
		                GEngine,
		                TEXT("Attempting to access %s from Client before initial FLightweightTransactionObject replication."),
		                *Tag.ToString());
	}
	else if ((*SearchResult) != nullptr)
	{
		if (bHasAuthority)
		{
			MarkItemDirty(**SearchResult);
		}

		return (*SearchResult)->PlayerTransactions.Modify(Tag, DeltaValue, bHasAuthority);
	}

	return DeltaValue;
}
