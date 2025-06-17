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

#include "Modules/ModuleManager.h"

/**
 *	Plugin Description :
 *
 *	HandshakeSample is a validation system that take advantage of the 'strategy pattern' to compare, validate and execute behaviour,
 *	based on the provided 'Handshake' context. A handshake can represent any interaction between two components that are trying to communicate
 *	with each other and execute an action. During gameplay, the GFP will push a component on the AGameStateBase and await a user request.
 *
 *	Example :
 *
 *		* Trading Items between players
 *		* Attempting to raise a player
 *		* Buying an Item from a Store
 *
 *	This system should be extended specific to your project needs and validation extended through the UHandshakeValidatorImpl so microservice in place
 *	can execute proper security check. 
 */
