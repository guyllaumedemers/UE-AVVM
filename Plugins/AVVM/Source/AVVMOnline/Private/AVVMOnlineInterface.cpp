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
#include "AVVMOnlineInterface.h"

bool FAVVMPlayerWallet::operator==(const FAVVMPlayerWallet& Rhs) const
{
	return true;
}

bool FAVVMPlayerProfile::operator==(const FAVVMPlayerProfile& Rhs) const
{
	return true;
}

bool FAVVMHostConfiguration::operator==(const FAVVMHostConfiguration& Rhs) const
{
	return true;
}

bool FAVVMRuntimeResource::operator==(const FAVVMRuntimeResource& Rhs) const
{
	return true;
}

bool FAVVMRuntimeChallenge::operator==(const FAVVMRuntimeChallenge& Rhs) const
{
	return true;
}

bool FAVVMPlayerConnection::operator==(const FAVVMPlayerConnection& Rhs) const
{
	return true;
}

bool FAVVMParty::operator==(const FAVVMParty& Rhs) const
{
	return true;
}

FAVVMPlayerRequest::FAVVMPlayerRequest(const FString& NewSrcPlayerUniqueNetId,
                                       const FString& NewDestPlayerUniqueNetId,
                                       const EAVVMPlayerRequestType NewRequestType,
                                       const FString& NewPayload)
	: SrcPlayerUniqueNetId(NewSrcPlayerUniqueNetId)
	, DestPlayerUniqueNetId(NewDestPlayerUniqueNetId)
	, RequestType(NewRequestType)
	, Payload(NewPayload)
{
}

bool FAVVMPlayerRequest::operator==(const FAVVMPlayerRequest& Rhs) const
{
	return true;
}

bool FAVVMLoginContext::operator==(const FAVVMLoginContext& Rhs) const
{
	return true;
}
