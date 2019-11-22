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

#ifndef USR_XMSGIMAUTHACCOUNTMGR_H_
#define USR_XMSGIMAUTHACCOUNTMGR_H_

#include "../coll/XmsgImAuthAccountColl.h"

class XmsgImAuthAccountMgr
{
public:
	bool add(shared_ptr<XmsgImAuthAccountColl> dat); 
	shared_ptr<XmsgImAuthAccountColl> findByUsr(const string& usr); 
	static void loadCb(shared_ptr<XmsgImAuthAccountColl> dat); 
	size_t size();
	static XmsgImAuthAccountMgr* instance();
private:
	unordered_map<string , shared_ptr<XmsgImAuthAccountColl>> account4usr; 
	mutex lock4account; 
	static XmsgImAuthAccountMgr* inst;
	XmsgImAuthAccountMgr();
	virtual ~XmsgImAuthAccountMgr();
};

#endif 
