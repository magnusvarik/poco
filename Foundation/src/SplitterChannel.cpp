//
// SplitterChannel.cpp
//
// $Id: //poco/1.3/Foundation/src/SplitterChannel.cpp#1 $
//
// Library: Foundation
// Package: Logging
// Module:  SplitterChannel
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/SplitterChannel.h"
#include "Poco/LoggingRegistry.h"
#include "Poco/StringTokenizer.h"


namespace Poco {


SplitterChannel::SplitterChannel()
{
}


SplitterChannel::~SplitterChannel()
{
	close();
}


void SplitterChannel::addChannel(Channel* pChannel)
{
	poco_check_ptr (pChannel);

	FastMutex::ScopedLock lock(_mutex);
	
	pChannel->duplicate();
	_channels.push_back(pChannel);
}


void SplitterChannel::removeChannel(Channel* pChannel)
{
	FastMutex::ScopedLock lock(_mutex);

	for (ChannelVec::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (*it == pChannel)
		{
			pChannel->release();
			_channels.erase(it);
			break;
		}
	}
}


void SplitterChannel::setProperty(const std::string& name, const std::string& value)
{
	if (name.compare(0, 7, "channel") == 0)
	{
		StringTokenizer tokenizer(value, ",;", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
		for (StringTokenizer::Iterator it = tokenizer.begin(); it != tokenizer.end(); ++it)
		{
			addChannel(LoggingRegistry::defaultRegistry().channelForName(*it));
		}
	}
	else Channel::setProperty(name, value);
}


void SplitterChannel::log(const Message& msg)
{
	FastMutex::ScopedLock lock(_mutex);

	for (ChannelVec::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		(*it)->log(msg);
	}
}


void SplitterChannel::close()
{
	FastMutex::ScopedLock lock(_mutex);

	for (ChannelVec::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		(*it)->release();
	}
	_channels.clear();
}


int SplitterChannel::count() const
{
	FastMutex::ScopedLock lock(_mutex);
	
	return (int) _channels.size();
}


} // namespace Poco
