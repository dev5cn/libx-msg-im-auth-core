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

#include "XmsgImAuthCfg.h"

shared_ptr<XmsgImAuthCfg> XmsgImAuthCfg::cfg; 

XmsgImAuthCfg::XmsgImAuthCfg()
{

}

shared_ptr<XmsgImAuthCfg> XmsgImAuthCfg::instance()
{
	return XmsgImAuthCfg::cfg;
}

shared_ptr<XmsgImAuthCfg> XmsgImAuthCfg::load(const char* path)
{
	XMLDocument doc;
	if (doc.LoadFile(path) != 0)
	{
		LOG_ERROR("load config file failed, path: %s", path)
		return nullptr;
	}
	XMLElement* root = doc.RootElement();
	if (root == NULL)
	{
		LOG_ERROR("it a empty xml file? path: %s", path)
		return nullptr;
	}
	shared_ptr<XmsgImAuthCfg> cfg(new XmsgImAuthCfg());
	cfg->cfgPb.reset(new XmsgImAuthCfgPb());
	Misc::strAtt(root, "cgt", cfg->cfgPb->mutable_cgt());
	cfg->cgt = ChannelGlobalTitle::parse(cfg->cfgPb->cgt());
	if (cfg->cgt == nullptr)
	{
		LOG_ERROR("channel global title format error: %s", cfg->cfgPb->cgt().c_str())
		return nullptr;
	}
	Misc::strAtt(root, "cfgType", cfg->cfgPb->mutable_cfgtype());
	if ("mongodb" == cfg->cfgPb->cfgtype())
	{
		XMLElement* dbUri = root->FirstChildElement("mongodb");
		auto mongodb = cfg->cfgPb->mutable_mongodb();
		Misc::strAtt(dbUri, "uri", mongodb->mutable_uri());
		XmsgImAuthCfg::setCfg(cfg); 
		return cfg;
	}
	if ("mysql" == cfg->cfgPb->cfgtype())
	{
		LOG_ERROR("not supported mysql.")
		return nullptr;
	}
	if ("mongodb" == Misc::strAtt(root, "db") && !cfg->loadMongodbCfg(root))
		return nullptr;
	if ("mysql" == Misc::strAtt(root, "db") && !cfg->loadMysqlCfg(root))
		return nullptr;
	if (!cfg->loadLogCfg(root))
		return nullptr;
	if (!cfg->loadXscServerCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeH2nCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeN2hCfg(root))
		return nullptr;
	if (!cfg->loadMiscCfg(root))
		return nullptr;
	LOG_INFO("load config file successful, cfg: %s", cfg->toString().c_str())
	XmsgImAuthCfg::setCfg(cfg);
	return cfg;
}

void XmsgImAuthCfg::setCfg(shared_ptr<XmsgImAuthCfg> cfg)
{
	XmsgImAuthCfg::cfg = cfg;
}

bool XmsgImAuthCfg::loadLogCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("log");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <log>")
		return false;
	}
	XmsgImAuthCfgLog* log = this->cfgPb->mutable_log();
	log->set_level(Misc::toUpercase(Misc::strAtt(node, "level")));
	log->set_output(Misc::toUpercase(Misc::strAtt(node, "output")));
	return true;
}

bool XmsgImAuthCfg::loadXscServerCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("xsc-server"); 
	while (node != NULL)
	{
		string name;
		Misc::strAtt(node, "name", &name);
		if ("pub-tcp" == name)
		{
			auto pub = this->loadXscTcpCfg(node);
			if (pub != nullptr)
				this->cfgPb->mutable_pubtcp()->CopyFrom(*pub);
			node = node->NextSiblingElement("xsc-server");
			continue;
		}
		if ("pub-http" == name)
		{
			auto pub = this->loadXscHttpCfg(node);
			if (pub != nullptr)
				this->cfgPb->mutable_pubhttp()->CopyFrom(*pub);
			node = node->NextSiblingElement("xsc-server");
			continue;
		}
		if ("pub-websocket" == name)
		{
			auto pub = this->loadXscWebSocketCfg(node);
			if (pub != nullptr)
				this->cfgPb->mutable_pubwebsocket()->CopyFrom(*pub);
			node = node->NextSiblingElement("xsc-server");
			continue;
		}
		if ("pri-tcp" == name)
		{
			auto pri = this->loadXscTcpCfg(node);
			if (pri == nullptr)
				return false;
			this->cfgPb->mutable_pritcp()->CopyFrom(*pri);
			node = node->NextSiblingElement("xsc-server");
			continue;
		}
		LOG_ERROR("unexpected xsc server name: %s, node: <xsc-server>", name.c_str())
		return false;
	}
	if (!this->cfgPb->has_pubtcp() && !this->cfgPb->has_pubhttp() && !this->cfgPb->has_pubwebsocket() && !this->cfgPb->has_pubudp() && !this->cfgPb->has_pubrudp())
	{
		LOG_ERROR("load x-msg-im-ap config failed, node: <xsc-server>, no public server")
		return false;
	}
	if (!this->cfgPb->has_pritcp())
	{
		LOG_ERROR("load x-msg-im-ap config failed, node: <xsc-server>, no private server")
		return false;
	}
	return true;
}

bool XmsgImAuthCfg::loadXmsgNeH2nCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("ne-group-h2n");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <ne-group-h2n>")
		return false;
	}
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		auto ne = this->cfgPb->add_h2n();
		Misc::strAtt(node, "neg", ne->mutable_neg());
		Misc::strAtt(node, "addr", ne->mutable_addr());
		Misc::strAtt(node, "pwd", ne->mutable_pwd());
		Misc::strAtt(node, "alg", ne->mutable_alg());
		if (ne->neg().empty() || ne->addr().empty() || ne->alg().empty())
		{
			LOG_ERROR("load config failed, node: <ne-group-h2n><ne>, ne: %s", ne->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("ne");
	}
	if (this->cfgPb->h2n().empty())
	{
		LOG_ERROR("load config failed, node: <ne-group-h2n><ne>")
		return false;
	}
	return true;
}

bool XmsgImAuthCfg::loadXmsgNeN2hCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("ne-group-n2h");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: ne-group-n2h")
		return false;
	}
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		auto ne = this->cfgPb->add_n2h();
		Misc::strAtt(node, "neg", ne->mutable_neg());
		Misc::strAtt(node, "cgt", ne->mutable_cgt());
		Misc::strAtt(node, "pwd", ne->mutable_pwd());
		Misc::strAtt(node, "addr", ne->mutable_addr());
		SptrCgt cgt = ChannelGlobalTitle::parse(ne->cgt());
		if (ne->cgt().empty() || ne->pwd().empty() || cgt == nullptr)
		{
			LOG_ERROR("load config failed, node: <ne-group-n2h><ne>, ne: %s", ne->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("ne");
	}
	if (this->cfgPb->n2h().empty())
	{
		LOG_ERROR("load config failed, node: <ne-group-n2h><ne>")
		return false;
	}
	return true;
}

bool XmsgImAuthCfg::loadMysqlCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("mysql");
	if (node == nullptr)
	{
		LOG_ERROR("load config failed, node: <mysql>")
		return false;
	}
	string host;
	auto mysql = this->cfgPb->mutable_mysql();
	Misc::strAtt(node, "host", &host);
	Misc::strAtt(node, "db", mysql->mutable_db());
	Misc::strAtt(node, "usr", mysql->mutable_usr());
	Misc::strAtt(node, "password", mysql->mutable_password());
	mysql->set_poolsize(Misc::hexOrInt(node, "poolSize"));
	int port;
	if (!Net::str2ipAndPort(host.c_str(), mysql->mutable_host(), &port))
	{
		LOG_ERROR("load config failed, node: <mysql>, host format error: %s", host.c_str())
		return false;
	}
	mysql->set_port(port);
	if (mysql->db().empty() || mysql->usr().empty() || mysql->password().empty())
	{
		LOG_ERROR("load config failed, node: <mysql>")
		return false;
	}
	return true;
}

bool XmsgImAuthCfg::loadMongodbCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("mongodb");
	if (node == nullptr)
	{
		LOG_ERROR("load config failed, node: <mongodb>")
		return false;
	}
	auto mongodb = this->cfgPb->mutable_mongodb();
	Misc::strAtt(node, "uri", mongodb->mutable_uri());
	if (mongodb->uri().empty())
	{
		LOG_ERROR("load config failed, node: <mongodb>")
		return false;
	}
	return true;
}

bool XmsgImAuthCfg::loadMiscCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("misc");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <misc>")
		return false;
	}
	auto misc = this->cfgPb->mutable_misc();
	misc->set_xmsgapserviceaddr(Misc::strAtt(node, "xmsgApServiceAddr"));
	if (misc->xmsgapserviceaddr().empty())
	{
		LOG_ERROR("load config failed, node: <misc><xmsgApServiceAddr>")
		return false;
	}
	misc->set_xmsgossserviceaddr(Misc::strAtt(node, "xmsgOssServiceAddr"));
	if (misc->xmsgossserviceaddr().empty())
	{
		LOG_ERROR("load config failed, node: <misc><xmsgOssServiceAddr>")
		return false;
	}
	misc->set_registeenable(Misc::strAtt(node, "registeEnable") == "true");
	misc->set_tokenexpiredseconds(Misc::intAtt(node, "tokenExpiredSeconds"));
	return true;
}

shared_ptr<XscTcpCfg> XmsgImAuthCfg::pubXscTcpServerCfg()
{
	return this->xmsgImAuthCfgXscTcpServer2xscTcpCfg(&this->cfgPb->pubtcp());
}

shared_ptr<XscHttpCfg> XmsgImAuthCfg::pubXscHttpServerCfg()
{
	if (!this->cfgPb->has_pubhttp())
		return nullptr;
	shared_ptr<XscHttpCfg> httpCfg(new XscHttpCfg());
	httpCfg->addr = this->cfgPb->pubhttp().tcp().addr();
	httpCfg->worker = this->cfgPb->pubhttp().tcp().worker();
	httpCfg->peerLimit = this->cfgPb->pubhttp().tcp().peerlimit();
	httpCfg->peerMtu = this->cfgPb->pubhttp().tcp().peermtu();
	httpCfg->peerRcvBuf = this->cfgPb->pubhttp().tcp().peerrcvbuf();
	httpCfg->peerSndBuf = this->cfgPb->pubhttp().tcp().peersndbuf();
	httpCfg->lazyClose = this->cfgPb->pubhttp().tcp().lazyclose();
	httpCfg->tracing = this->cfgPb->pubhttp().tcp().tracing();
	httpCfg->heartbeat = this->cfgPb->pubhttp().tcp().heartbeat();
	httpCfg->n2hZombie = this->cfgPb->pubhttp().tcp().n2hzombie();
	httpCfg->n2hTracing = this->cfgPb->pubhttp().tcp().n2htracing();
	httpCfg->h2nReConn = this->cfgPb->pubhttp().tcp().h2nreconn();
	httpCfg->h2nTransTimeout = this->cfgPb->pubhttp().tcp().h2ntranstimeout();
	httpCfg->headerLimit = this->cfgPb->pubhttp().headerlimit();
	httpCfg->bodyLimit = this->cfgPb->pubhttp().bodylimit();
	httpCfg->closeWait = this->cfgPb->pubhttp().closewait();
	for (auto& it : this->cfgPb->pubhttp().requiredheader())
		httpCfg->requiredHeader.insert(it);
	return httpCfg;
}

shared_ptr<XscWebSocketCfg> XmsgImAuthCfg::pubXscWebSocketServerCfg()
{
	if (!this->cfgPb->has_pubwebsocket())
		return nullptr;
	shared_ptr<XscWebSocketCfg> webSocketCfg(new XscWebSocketCfg());
	webSocketCfg->addr = this->cfgPb->pubwebsocket().tcp().addr();
	webSocketCfg->worker = this->cfgPb->pubwebsocket().tcp().worker();
	webSocketCfg->peerLimit = this->cfgPb->pubwebsocket().tcp().peerlimit();
	webSocketCfg->peerMtu = this->cfgPb->pubwebsocket().tcp().peermtu();
	webSocketCfg->peerRcvBuf = this->cfgPb->pubwebsocket().tcp().peerrcvbuf();
	webSocketCfg->peerSndBuf = this->cfgPb->pubwebsocket().tcp().peersndbuf();
	webSocketCfg->lazyClose = this->cfgPb->pubwebsocket().tcp().lazyclose();
	webSocketCfg->tracing = this->cfgPb->pubwebsocket().tcp().tracing();
	webSocketCfg->heartbeat = this->cfgPb->pubwebsocket().tcp().heartbeat();
	webSocketCfg->n2hZombie = this->cfgPb->pubwebsocket().tcp().n2hzombie();
	webSocketCfg->n2hTracing = this->cfgPb->pubwebsocket().tcp().n2htracing();
	webSocketCfg->h2nReConn = this->cfgPb->pubwebsocket().tcp().h2nreconn();
	webSocketCfg->h2nTransTimeout = this->cfgPb->pubwebsocket().tcp().h2ntranstimeout();
	return webSocketCfg;
}

shared_ptr<XscTcpCfg> XmsgImAuthCfg::priXscTcpServerCfg()
{
	return this->xmsgImAuthCfgXscTcpServer2xscTcpCfg(&this->cfgPb->pritcp());
}

shared_ptr<XmsgImAuthCfgXscTcpServer> XmsgImAuthCfg::loadXscTcpCfg(XMLElement* node)
{
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <xsc-server>");
		return nullptr;
	}
	shared_ptr<XmsgImAuthCfgXscTcpServer> tcpCfg(new XmsgImAuthCfgXscTcpServer());
	tcpCfg->set_addr(Misc::strAtt(node, "addr"));
	tcpCfg->set_worker(Misc::hexOrInt(node, "worker"));
	tcpCfg->set_peerlimit(Misc::hexOrInt(node, "peerLimit"));
	tcpCfg->set_peermtu(Misc::hexOrInt(node, "peerMtu"));
	tcpCfg->set_peerrcvbuf(Misc::hexOrInt(node, "peerRcvBuf"));
	tcpCfg->set_peersndbuf(Misc::hexOrInt(node, "peerSndBuf"));
	tcpCfg->set_lazyclose(Misc::hexOrInt(node, "lazyClose"));
	tcpCfg->set_tracing("true" == Misc::strAtt(node, "tracing"));
	tcpCfg->set_heartbeat(Misc::hexOrInt(node, "heartbeat"));
	tcpCfg->set_n2hzombie(Misc::hexOrInt(node, "n2hZombie"));
	tcpCfg->set_n2htranstimeout(Misc::hexOrInt(node, "n2hTransTimeout"));
	tcpCfg->set_n2htracing("true" == Misc::strAtt(node, "n2hTracing"));
	tcpCfg->set_h2nreconn(Misc::hexOrInt(node, "h2nReConn"));
	tcpCfg->set_h2ntranstimeout(Misc::hexOrInt(node, "h2nTransTimeout"));
	return tcpCfg;
}

shared_ptr<XmsgImAuthCfgXscHttpServer> XmsgImAuthCfg::loadXscHttpCfg(XMLElement* node)
{
	if (node == NULL)
		return nullptr;
	shared_ptr<XmsgImAuthCfgXscHttpServer> httpServer(new XmsgImAuthCfgXscHttpServer());
	httpServer->mutable_tcp()->CopyFrom(*this->loadXscTcpCfg(node));
	httpServer->set_headerlimit(Misc::hexOrInt(node, "headerLimit"));
	httpServer->set_bodylimit(Misc::hexOrInt(node, "bodyLimit"));
	httpServer->set_closewait(Misc::hexOrInt(node, "closeWait"));
	vector<string> header;
	Misc::split(Misc::strAtt(node, "requiredHeader"), ",", header);
	for (auto& it : header)
		httpServer->add_requiredheader(it);
	return httpServer;
}

shared_ptr<XmsgImAuthCfgXscWebSocketServer> XmsgImAuthCfg::loadXscWebSocketCfg(XMLElement* node)
{
	if (node == NULL)
		return nullptr;
	shared_ptr<XmsgImAuthCfgXscWebSocketServer> webSocketServer(new XmsgImAuthCfgXscWebSocketServer());
	webSocketServer->mutable_tcp()->CopyFrom(*this->loadXscTcpCfg(node));
	return webSocketServer;
}

shared_ptr<XscTcpCfg> XmsgImAuthCfg::xmsgImAuthCfgXscTcpServer2xscTcpCfg(const XmsgImAuthCfgXscTcpServer* pb)
{
	shared_ptr<XscTcpCfg> tcpCfg(new XscTcpCfg());
	tcpCfg->addr = pb->addr();
	tcpCfg->worker = pb->worker();
	tcpCfg->peerLimit = pb->peerlimit();
	tcpCfg->peerMtu = pb->peermtu();
	tcpCfg->peerRcvBuf = pb->peerrcvbuf();
	tcpCfg->peerSndBuf = pb->peersndbuf();
	tcpCfg->lazyClose = pb->lazyclose();
	tcpCfg->tracing = pb->tracing();
	tcpCfg->heartbeat = pb->heartbeat();
	tcpCfg->n2hZombie = pb->n2hzombie();
	tcpCfg->n2hTransTimeout = pb->n2htranstimeout();
	tcpCfg->n2hTracing = pb->n2htracing();
	tcpCfg->h2nReConn = pb->h2nreconn();
	tcpCfg->h2nTransTimeout = pb->h2ntranstimeout();
	return tcpCfg;
}

string XmsgImAuthCfg::toString()
{
	return this->cfgPb->ShortDebugString();
}

XmsgImAuthCfg::~XmsgImAuthCfg()
{

}

