/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "XmsgImAuthAccountTokenMgr.h"

XmsgImAuthAccountTokenMgr* XmsgImAuthAccountTokenMgr::inst = new XmsgImAuthAccountTokenMgr();

XmsgImAuthAccountTokenMgr::XmsgImAuthAccountTokenMgr()
{

}

XmsgImAuthAccountTokenMgr* XmsgImAuthAccountTokenMgr::instance()
{
	return XmsgImAuthAccountTokenMgr::inst;
}

void XmsgImAuthAccountTokenMgr::add(shared_ptr<XmsgImAuthTokenColl> dat)
{
	unique_lock<mutex> lock(this->lock4token);
	this->token[dat->token] = dat;
}

shared_ptr<XmsgImAuthTokenColl> XmsgImAuthAccountTokenMgr::find(const string& token)
{
	unique_lock<mutex> lock(this->lock4token);
	auto it = this->token.find(token);
	if (it == this->token.end())
		return nullptr;
	if (DateMisc::gotoGmt0(Xsc::clock) >= it->second->expired)
	{
		LOG_DEBUG("have a auth token expired: %s", it->second->toString().c_str())
		this->token.erase(it);
		return nullptr;
	}
	return it->second;
}

size_t XmsgImAuthAccountTokenMgr::size()
{
	unique_lock<mutex> lock(this->lock4token);
	return this->token.size();
}

void XmsgImAuthAccountTokenMgr::loadCb(shared_ptr<XmsgImAuthTokenColl> dat)
{
	XmsgImAuthAccountTokenMgr::instance()->add(dat);
}

XmsgImAuthAccountTokenMgr::~XmsgImAuthAccountTokenMgr()
{

}

