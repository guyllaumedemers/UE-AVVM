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
#include "Ability/AVVMAttributeSet.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UAVVMAttributeSet::Init()
{
	const auto OnAsyncRequestComplete = [](const TWeakObjectPtr<UAVVMAttributeSet>& Caller)
	{
		UAVVMAttributeSet* AttributeSet = Caller.Get();
		if (!IsValid(AttributeSet))
		{
			return;
		}

		TSharedPtr<FStreamableHandle> StreamableHandle = AttributeSet->AttributeMetaDataTableHandle;
		if (!StreamableHandle.IsValid())
		{
			return;
		}

		TArray<UObject*> OutResources;
		StreamableHandle->GetLoadedAssets(OutResources);

		for (UObject* Resource : OutResources)
		{
			auto* AttributeMetaDataTable = Cast<UDataTable>(Resource);
			if (IsValid(AttributeMetaDataTable))
			{
				AttributeSet->InitFromMetaDataTable(AttributeMetaDataTable);
			}
		}
	};
	
	FStreamableDelegate Callback;
	Callback.BindWeakLambda(this, OnAsyncRequestComplete, TWeakObjectPtr(this));
	AttributeMetaDataTableHandle = UAssetManager::Get().LoadAssetList({AttributeMetaDataTable.ToSoftObjectPath()}, Callback);
}

void IAVVMDoesOwnAttributeSet::SetAttributeSet(const UAttributeSet* NewAttributeSet)
{
	OwnedAttributeSet = NewAttributeSet;
}

const UAttributeSet* IAVVMDoesOwnAttributeSet::GetAttributeSet() const
{
	return OwnedAttributeSet.Get();
}
