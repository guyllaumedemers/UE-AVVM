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

#include "Templates/SubclassOf.h"
#include "UObject/Object.h"

#include "AVVMPresenter.generated.h"

class UMVVMViewModelBase;

/**
 *	Class description:
 *
 *	UAVVMPresenter is a UObject type that bridge the "Gameplay code" and "UI code". It's an Abstract Class
 *	that define a Key-Value pair information required for caching an Actor to a "Manual" type ViewModel on
 *	the UAVVMSubsystem.
 */
UCLASS(Abstract, BlueprintType)
class AVVM_API UAVVMPresenter : public UObject
{
	GENERATED_BODY()

public:
	UAVVMPresenter();
	virtual void BeginDestroy() override;

protected:
	virtual TSubclassOf<UMVVMViewModelBase> GetViewModelClass() const PURE_VIRTUAL(GetViewModelClass, return nullptr;)
	virtual AActor* GetOuterKey() const PURE_VIRTUAL(GetOuterKey, return nullptr;)

	friend class UAVVMSubsystem;
};
