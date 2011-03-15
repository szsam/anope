/*
 * Copyright (C) 2008-2011 Robin Burchell <w00t@inspircd.org>
 * Copyright (C) 2008-2011 Anope Team <team@anope.org>
 *
 * Please read COPYING and README for further details.
 */

#include "services.h"
#include "modules.h"
#include "commands.h"

Anope::insensitive_map<BotInfo *> BotListByNick;
Anope::map<BotInfo *> BotListByUID;

BotInfo *BotServ = NULL;
BotInfo *ChanServ = NULL;
BotInfo *Global = NULL;
BotInfo *HostServ = NULL;
BotInfo *MemoServ = NULL;
BotInfo *NickServ = NULL;
BotInfo *OperServ = NULL;

BotInfo::BotInfo(const Anope::string &nnick, const Anope::string &nuser, const Anope::string &nhost, const Anope::string &nreal) : User(nnick, nuser, nhost, ts6_uid_retrieve()), Flags<BotFlag, BI_END>(BotFlagString)
{
	this->realname = nreal;
	this->server = Me;

	this->chancount = 0;
	this->lastmsg = this->created = Anope::CurTime;

	this->SetFlag(BI_CORE);
	if (!Config->s_ChanServ.empty() && nnick.equals_ci(Config->s_ChanServ))
		ChanServ = this;
	else if (!Config->s_BotServ.empty() && nnick.equals_ci(Config->s_BotServ))
		BotServ = this;
	else if (!Config->s_HostServ.empty() && nnick.equals_ci(Config->s_HostServ))
		HostServ = this;
	else if (!Config->s_OperServ.empty() && nnick.equals_ci(Config->s_OperServ))
		OperServ = this;
	else if (!Config->s_MemoServ.empty() && nnick.equals_ci(Config->s_MemoServ))
		MemoServ = this;
	else if (!Config->s_NickServ.empty() && nnick.equals_ci(Config->s_NickServ))
		NickServ = this;
	else if (!Config->s_GlobalNoticer.empty() && nnick.equals_ci(Config->s_GlobalNoticer))
		Global = this;
	else
		this->UnsetFlag(BI_CORE);

	BotListByNick[this->nick] = this;
	if (!this->uid.empty())
		BotListByUID[this->uid] = this;

	// If we're synchronised with the uplink already, send the bot.
	if (Me && Me->IsSynced())
	{
		ircdproto->SendClientIntroduction(this, ircd->pseudoclient_mode);
		XLine x(this->nick, "Reserved for services");
		ircdproto->SendSQLine(NULL, &x);
	}

	this->SetModeInternal(ModeManager::FindUserModeByName(UMODE_PROTECTED));
	this->SetModeInternal(ModeManager::FindUserModeByName(UMODE_GOD));
}

BotInfo::~BotInfo()
{
	// If we're synchronised with the uplink already, send the bot.
	if (Me && Me->IsSynced())
	{
		ircdproto->SendQuit(this, NULL);
		XLine x(this->nick);
		ircdproto->SendSQLineDel(&x);
	}

	for (registered_channel_map::const_iterator it = RegisteredChannelList.begin(), it_end = RegisteredChannelList.end(); it != it_end; ++it)
	{
		ChannelInfo *ci = it->second;

		if (ci->bi == this)
			ci->bi = NULL;
	}

	for (CommandMap::const_iterator it = this->Commands.begin(), it_end = this->Commands.end(); it != it_end; ++it)
	{
		Command *c = it->second;

		if (c->module)
			c->module->DelCommand(this, c);
	}

	BotListByNick.erase(this->nick);
	if (!this->uid.empty())
		BotListByUID.erase(this->uid);
	
	if (this->HasFlag(BI_CORE))
	{
		if (this == ChanServ)
			ChanServ = NULL;
		else if (this == BotServ)
			BotServ = NULL;
		else if (this == HostServ)
			HostServ = NULL;
		else if (this == OperServ)
			OperServ = NULL;
		else if (this == MemoServ)
			MemoServ = NULL;
		else if (this == NickServ)
			NickServ = NULL;
		else if (this == Global)
			Global = NULL;
	}
}

void BotInfo::SetNewNick(const Anope::string &newnick)
{
	UserListByNick.erase(this->nick);
	BotListByNick.erase(this->nick);

	this->nick = newnick;

	UserListByNick[this->nick] = this;
	BotListByNick[this->nick] = this;
}

void BotInfo::RejoinAll()
{
	for (registered_channel_map::const_iterator it = RegisteredChannelList.begin(), it_end = RegisteredChannelList.end(); it != it_end; ++it)
	{
		ChannelInfo *ci = it->second;

		if (ci->bi == this && ci->c && ci->c->users.size() >= Config->BSMinUsers)
			this->Join(ci->c);
	}
}

void BotInfo::Assign(User *u, ChannelInfo *ci)
{
	EventReturn MOD_RESULT = EVENT_CONTINUE;
	FOREACH_RESULT(I_OnBotAssign, OnBotAssign(u, ci, this));
	if (MOD_RESULT == EVENT_STOP)
		return;

	if (ci->bi)
		ci->bi->UnAssign(u, ci);
	
	++this->chancount;

	ci->bi = this;
	if (ci->c && ci->c->users.size() >= Config->BSMinUsers)
		this->Join(ci->c, &Config->BotModeList);
}

void BotInfo::UnAssign(User *u, ChannelInfo *ci)
{
	EventReturn MOD_RESULT = EVENT_CONTINUE;
	FOREACH_RESULT(I_OnBotUnAssign, OnBotUnAssign(u, ci));
	if (MOD_RESULT == EVENT_STOP)
		return;

	if (ci->c && ci->c->FindUser(ci->bi))
	{
		if (u)
			ci->bi->Part(ci->c, "UNASSIGN from " + u->nick);
		else
			ci->bi->Part(ci->c);
	}

	--this->chancount;

	ci->bi = NULL;
}

void BotInfo::Join(Channel *c, ChannelStatus *status)
{
	if (Config->BSSmartJoin)
	{
		std::pair<Channel::ModeList::iterator, Channel::ModeList::iterator> bans = c->GetModeList(CMODE_BAN);

		/* We check for bans */
		for (; bans.first != bans.second; ++bans.first)
		{
			Entry ban(CMODE_BAN, bans.first->second);
			if (ban.Matches(this))
				c->RemoveMode(NULL, CMODE_BAN, ban.GetMask());
		}

		Anope::string Limit;
		unsigned limit = 0;
		if (c->GetParam(CMODE_LIMIT, Limit) && Limit.is_pos_number_only())
			limit = convertTo<unsigned>(Limit);

		/* Should we be invited? */
		if (c->HasMode(CMODE_INVITE) || (limit && c->users.size() >= limit))
			ircdproto->SendNoticeChanops(this, c, "%s invited %s into the channel.", this->nick.c_str(), this->nick.c_str());

		ModeManager::ProcessModes();
	}

	c->JoinUser(this);
	ircdproto->SendJoin(this, c, status);

	FOREACH_MOD(I_OnBotJoin, OnBotJoin(c, this));
}

void BotInfo::Join(const Anope::string &chname, ChannelStatus *status)
{
	Channel *c = findchan(chname);
	return this->Join(c ? c : new Channel(chname), status);
}

void BotInfo::Part(Channel *c, const Anope::string &reason)
{
	ircdproto->SendPart(this, c, "%s", !reason.empty() ? reason.c_str() : "");
	c->DeleteUser(this);
}
