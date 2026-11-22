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

#include "AVVMNotificationSubsystem.h"
#include "Dom/JsonObject.h"

#include "AVVMComplexPayloads.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMStringPayload is an FAVVMNotificationPayload type carrying an FString property.
 */
USTRUCT()
struct AVVM_API FAVVMStringPayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FAVVMStringPayload() = default;
	FAVVMStringPayload(const FAVVMStringPayload&) = default;
	FAVVMStringPayload(FAVVMStringPayload&&) noexcept = default;
	FAVVMStringPayload& operator=(const FAVVMStringPayload&) = default;
	FAVVMStringPayload& operator=(FAVVMStringPayload&&) noexcept = default;
	bool operator==(const FAVVMStringPayload& Rhs) const;
	explicit FAVVMStringPayload(FString&& NewValue);

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Value{};
};

/**
 *	Class description:
 *	
 *	FAVVMJsonPayload is an FAVVMNotificationPayload type carrying an TSharedPtr<FJsonObject> property.
 */
USTRUCT()
struct AVVM_API FAVVMJsonPayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FAVVMJsonPayload() = default;
	FAVVMJsonPayload(const FAVVMJsonPayload&) = default;
	FAVVMJsonPayload(FAVVMJsonPayload&&) noexcept = default;
	FAVVMJsonPayload& operator=(const FAVVMJsonPayload&) = default;
	FAVVMJsonPayload& operator=(FAVVMJsonPayload&&) noexcept = default;
	bool operator==(const FAVVMJsonPayload& Rhs) const;
	explicit FAVVMJsonPayload(const FString& NewValue);
	explicit FAVVMJsonPayload(TSharedPtr<FJsonObject> NewValue);

	template <typename TJsonObject>
	static TJsonObject From(const FAVVMJsonPayload* Payload);

	template <typename TJsonObject>
	static TInstancedStruct<FAVVMNotificationPayload> To(TJsonObject NewValue);

	UPROPERTY(Transient, BlueprintReadWrite)
	FString JsonPayload{};

	TSharedPtr<FJsonObject> Value{};
};

template <>
FORCEINLINE TInstancedStruct<FAVVMNotificationPayload> FAVVMJsonPayload::To(bool NewValue)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetBoolField(TEXT("Value"), NewValue);
	return FAVVMNotificationPayload::Make<FAVVMJsonPayload>(JsonData);
}

template <>
FORCEINLINE TInstancedStruct<FAVVMNotificationPayload> FAVVMJsonPayload::To(int32 NewValue)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("Value"), NewValue);
	return FAVVMNotificationPayload::Make<FAVVMJsonPayload>(JsonData);
}

template <>
FORCEINLINE TInstancedStruct<FAVVMNotificationPayload> FAVVMJsonPayload::To(double NewValue)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetNumberField(TEXT("Value"), NewValue);
	return FAVVMNotificationPayload::Make<FAVVMJsonPayload>(JsonData);
}

template <>
FORCEINLINE TInstancedStruct<FAVVMNotificationPayload> FAVVMJsonPayload::To(FString NewValue)
{
	TSharedPtr<FJsonObject> JsonData = MakeShareable(new FJsonObject);
	JsonData->SetStringField(TEXT("Value"), NewValue);
	return FAVVMNotificationPayload::Make<FAVVMJsonPayload>(JsonData);
}

template <typename TJsonObject>
TInstancedStruct<FAVVMNotificationPayload> FAVVMJsonPayload::To(TJsonObject NewValue)
{
	return FAVVMNotificationPayload::Make<FAVVMJsonPayload>(NewValue.GetJsonObject());
}

template <>
FORCEINLINE bool FAVVMJsonPayload::From(const FAVVMJsonPayload* Payload)
{
	if (ensureAlwaysMsgf(Payload != nullptr, TEXT("Failed to cast retrieved FAVVMJsonPayload object.")))
	{
		return Payload->Value.IsValid() ? Payload->Value->GetBoolField(TEXT("Value")) : false;
	}
	else
	{
		return false;
	}
}

template <>
FORCEINLINE int32 FAVVMJsonPayload::From(const FAVVMJsonPayload* Payload)
{
	if (ensureAlwaysMsgf(Payload != nullptr, TEXT("Failed to cast retrieved FAVVMJsonPayload object.")))
	{
		return Payload->Value.IsValid() ? Payload->Value->GetIntegerField(TEXT("Value")) : INDEX_NONE;
	}
	else
	{
		return INDEX_NONE;
	}
}

template <>
FORCEINLINE double FAVVMJsonPayload::From(const FAVVMJsonPayload* Payload)
{
	if (ensureAlwaysMsgf(Payload != nullptr, TEXT("Failed to cast retrieved FAVVMJsonPayload object.")))
	{
		return Payload->Value.IsValid() ? Payload->Value->GetNumberField(TEXT("Value")) : INDEX_NONE;
	}
	else
	{
		return INDEX_NONE;
	}
}

template <>
FORCEINLINE FString FAVVMJsonPayload::From(const FAVVMJsonPayload* Payload)
{
	static FStringView EmptyString{TEXT("")};
	if (ensureAlwaysMsgf(Payload != nullptr, TEXT("Failed to cast retrieved FAVVMJsonPayload object.")))
	{
		return Payload->Value.IsValid() ? Payload->Value->GetStringField(TEXT("Value")) : EmptyString.GetData();
	}
	else
	{
		return EmptyString.GetData();
	}
}

template <typename TJsonObject>
TJsonObject FAVVMJsonPayload::From(const FAVVMJsonPayload* Payload)
{
	if (ensureAlwaysMsgf(Payload != nullptr, TEXT("Failed to cast retrieved FAVVMJsonPayload object.")))
	{
		return TJsonObject{Payload->Value};
	}
	else
	{
		return TJsonObject{};
	}
}
