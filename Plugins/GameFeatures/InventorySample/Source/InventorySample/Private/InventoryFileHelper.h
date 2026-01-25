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

#include "InventoryFileHelper.generated.h"

/**
 *	Class description:
 *	
 *	UInventoryFileHelper is a singleton helper object that allow global access to inventory content serialized to disk.
 */
UCLASS()
class INVENTORYSAMPLE_API UInventoryFileHelper : public UObject
{
	GENERATED_BODY()
	
public:
	static FStringView Static_GetSetFileContent();
	static void Static_Serialize(const FString& NewFileContent);
	
protected:
	static UInventoryFileHelper* Get();
	FStringView GetSetFileContent(const FStringView NewFilePath);
	// @gdemers _v2 prevent function name shadowing in base UObject class.
	void Serialize_v2(const FString& NewFileContent);
	void MarkFileDirty();
	
	UPROPERTY(Transient,  BlueprintReadOnly)
	bool bIsMarkedDirty = false;
	
	UPROPERTY(Transient,  BlueprintReadOnly)
	FString FileContent = FString();
	
	static TStrongObjectPtr<UInventoryFileHelper> gInventoryFileHelper;
};
