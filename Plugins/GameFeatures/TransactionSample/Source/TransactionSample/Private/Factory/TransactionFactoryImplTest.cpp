﻿//Copyright(c) 2025 gdemers
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
#include "Factory/TransactionFactoryImplTest.h"

#include "AVVMTokenizer.h"

FTransactionPayloadTest::FTransactionPayloadTest(const int32 NewDummyProperty)
	: DummyProperty(NewDummyProperty)
{
}

TInstancedStruct<FTransactionPayload> FTransactionPayloadTest::Init(const FString& NewPayload)
{
	const int32 Property = UAVVMTokenizer::GetTokenValue<int32>(TEXT("DummyProperty"), NewPayload);
	return FTransactionPayload::Make<FTransactionPayloadTest>(Property);
}

FString FTransactionPayloadTest::ToString() const
{
	FStringFormatNamedArguments Args;
	Args.Add(TEXT("DummyProperty"), FStringFormatArg{DummyProperty});
	return FString::Format(TEXT("DummyProperty:{DummyProperty}"), Args);
}

TInstancedStruct<FTransactionPayload> UTransactionFactoryImplTest::CreatePayload(const FString& NewPayload) const
{
	auto Instanced = FTransactionPayloadTest();
	return Instanced.Init(NewPayload);
}
