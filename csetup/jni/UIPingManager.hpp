/*
 * Copyright (c) 2012, Sergey Parshin, qrck@mail.ru
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __UI_PING_MANAGER_H__
#define __UI_PING_MANAGER_H__ 

#include <sys/time.h>


class CUIPingManager
{
public:
	class IUIPingReceiver
	{
	public:
		virtual void onPing() = 0;
	};

private:
	int 	m_nInterval;

	time_t m_lastPing;

	IUIPingReceiver* m_receiver;

	long long tm()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);

		return ((long long)tv.tv_sec) + ((long long)tv.tv_usec) * 1000000LL;
	}
public:
	CUIPingManager()
		: m_nInterval(10)
		, m_receiver(NULL)
		, m_lastPing(0)
	{
	}

	void setInterval(int pTimeout)
	{
		m_nInterval = pTimeout;
	}

	int getInterval() const
	{
		return m_nInterval;
	}

	void setReceiver(IUIPingReceiver* recv) 
	{
		m_receiver = recv;
	}

	bool isActive() const
	{
		return m_receiver != NULL;
	}

	void onIter()
	{
		if ( m_receiver )
		{
			time_t now = time(NULL);

			if ( now - m_lastPing >= m_nInterval )
			{
				m_lastPing = now;
				m_receiver->onPing();
			}
		}
	}
};


#endif
