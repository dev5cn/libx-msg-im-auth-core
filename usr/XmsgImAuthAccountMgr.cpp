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

#include "XmsgImAuthAccountMgr.h"

XmsgImAuthAccountMgr* XmsgImAuthAccountMgr::inst = new XmsgImAuthAccountMgr();

XmsgImAuthAccountMgr::XmsgImAuthAccountMgr()
{

}

XmsgImAuthAccountMgr* XmsgImAuthAccountMgr::instance()
{
	return XmsgImAuthAccountMgr::inst;
}

bool XmsgImAuthAccountMgr::add(shared_ptr<XmsgImAuthAccountColl> dat)
{
	unique_lock<mutex> lock(this->lock4account);
	auto it = this->account4usr.find(dat->usr);
	if (it != this->account4usr.end())
		return false;
	this->account4usr[dat->usr] = dat;
	return true;
}

shared_ptr<XmsgImAuthAccountColl> XmsgImAuthAccountMgr::findByUsr(const string& usr)
{
	unique_lock<mutex> lock(this->lock4account);
	auto it = this->account4usr.find(usr);
	return it == this->account4usr.end() ? nullptr : it->second;
}

size_t XmsgImAuthAccountMgr::size()
{
	unique_lock<mutex> lock(this->lock4account);
	return this->account4usr.size();
}

void XmsgImAuthAccountMgr::loadCb(shared_ptr<XmsgImAuthAccountColl> dat)
{
	if (XmsgImAuthAccountMgr::instance()->add(dat))
		return;
	LOG_FAULT("it`s a bug, account already existed, dat: %s", dat->toString().c_str())
}

XmsgImAuthAccountMgr::~XmsgImAuthAccountMgr()
{

}

