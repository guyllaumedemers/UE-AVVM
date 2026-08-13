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

#include "UObject/Object.h"
#include "UObject/StrongObjectPtrTemplates.h"

#include "AVVMFileHelper.generated.h"

class UAVVMSaveGame;

/**
 *	Class description:
 *	
 *	UAVVMFileHelper is a singleton helper object that allow global access to content serialized to disk.
 */
UCLASS(NotBlueprintType, NotBlueprintable)
class AVVMTOOLKIT_API UAVVMFileHelper final : public UObject
{
	GENERATED_BODY()

public:
	static void Static_SetSaveGameSlot(const FName SaveGameSlot);
	static UAVVMSaveGame* Static_GetSetSaveGame();

protected:
	static UAVVMFileHelper* Get();
	FName GetSetSaveGameSlot();
	void SetSaveGameSlot(const FName SaveGameSlot);
	UAVVMSaveGame* GetSetSaveGame();

	UPROPERTY(Transient, BlueprintReadOnly)
	FName ActiveSaveGameSlot = NAME_None;

	static TStrongObjectPtr<UAVVMFileHelper> gFileHelper;
	TStrongObjectPtr<UAVVMSaveGame> SaveGameObject = nullptr;
};
