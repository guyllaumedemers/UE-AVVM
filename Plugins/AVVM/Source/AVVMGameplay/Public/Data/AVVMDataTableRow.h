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

#include "Engine/DataAsset.h"

#include "AVVMDataTableRow.generated.h"

/**
 *	Class description:
 *
 *	FAVVMDataTableRow define an empty type to be derived from and expose a list of FSoftObjectPath that should be asynchronously loaded to the Manager Component.
 *	Users are expected to encapsulate data into UDataAsset or UObject derived type and append the ResourcePath list so the Manager Component can handle the loading of resources.
 *	Note : Nested Registry Id should be handled at the Actor level, i.e derived implementer of IAVVMResourceProvider. Actor derived Classes are expected to handle the gathering of unloaded
 *	RegistryIds and defer additional loading via IAVVMResourceProvider::CheckIsDoneAcquiringResources argument delegate.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const PURE_VIRTUAL(GetResourcesPaths, return TArray<FSoftObjectPath>(););
};
