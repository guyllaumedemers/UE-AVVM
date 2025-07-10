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

#include "AVVMWidgetPickerDataAsset.generated.h"

class UCommonUserWidget;

/**
 *	Class description:
 *
 *	UAVVMWidgetPickerDataAsset is a Data Asset that expose a mapping of Object Class to Widget Class. User is expected
 *	to provide ViewModel derived Class as Key and Widget derived class as Value.
 *
 *	Doing so will allow generating Widget Class specific to the provided UObject type when populating dynamic list widgets.
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMWidgetPickerDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	TSubclassOf<UCommonUserWidget> GetWidgetClass(const UClass* ObjectClass);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<TObjectPtr<UClass>, TSubclassOf<UCommonUserWidget>> ObjectClassToWidgetClass;
};
