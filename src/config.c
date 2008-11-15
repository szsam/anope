/* Configuration file handling.
 *
 * (C) 2003-2008 Anope Team
 * Contact us at info@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 * $Id$
 *
 */

#include "services.h"
#include "configreader.h"
#include "hashcomp.h" // If this gets added to services.h or someplace else later, remove it from here -- CyberBotX

/*************************************************************************/

ServerConfig serverConfig;

// This is temporary while directives are transitioned from the old config to the new one -- CyberBotX
static const char *SERVICES_CONF_NEW = "services_new.conf";

/* Configurable variables: */

char *IRCDModule;
char *EncModule;
char *RemoteServer;
int RemotePort;
char *RemotePassword;

char *RemoteServer2;
int RemotePort2;
char *RemotePassword2;

char *RemoteServer3;
int RemotePort3;
char *RemotePassword3;

char *LocalHost;
int LocalPort;

char *ServerName;
char *ServerDesc;
char *ServiceUser;
char *ServiceHost;

char *HelpChannel;
char *LogChannel;
char *NetworkName;
int NickLen;

char *s_NickServ;
char *s_ChanServ;
char *s_MemoServ;
char *s_BotServ;
char *s_HelpServ;
char *s_OperServ;
char *s_GlobalNoticer;
char *desc_NickServ;
char *desc_ChanServ;
char *desc_MemoServ;
char *desc_BotServ;
char *desc_HelpServ;
char *desc_OperServ;
char *desc_GlobalNoticer;

char *HostDBName;			   /* Name of HostServ DB File */
char *s_HostServ;			   /* HostServ Name */
char *desc_HostServ;			/* HostServ Description */

char *PIDFilename;
char *MOTDFilename;
char *NickDBName;
char *PreNickDBName;
char *ChanDBName;
char *BotDBName;
char *OperDBName;
char *AutokillDBName;
char *NewsDBName;

static char *HostSetter;
char **HostSetters;
int HostNumber = 0;			 /* needs to be set to 0 */

int NoBackupOkay;
int StrictPasswords;
int BadPassLimit;
int BadPassTimeout;
int UpdateTimeout;
int ExpireTimeout;
int ReadTimeout;
int WarningTimeout;
int TimeoutCheck;
int KeepLogs;
int KeepBackups;
int ForceForbidReason;
int UsePrivmsg;
int UseStrictPrivMsg;
int DumpCore;
int LogUsers;
int NickRegDelay;
int UseSVSHOLD;
int NewsCount;

bool UseMail;
char *SendMailPath;
char *SendFrom;
bool RestrictMail;
time_t MailDelay;
bool DontQuoteAddresses;

static std::string NSDefaults;
char *NSGuestNickPrefix;
bool NSAllowKillImmed;
bool NSNoGroupChange;
int NSDefFlags;
int NSDefLanguage;
time_t NSRegDelay;
time_t NSResendDelay;
time_t NSExpire;
time_t NSRExpire;
bool NSForceEmail;
int NSMaxAliases;
int NSAccessMax;
char *NSEnforcerUser;
char *NSEnforcerHost;
static char *temp_nsuserhost;
time_t NSReleaseTimeout;
bool NSListOpersOnly;
int NSListMax;
bool NSSecureAdmins;
bool NSStrictPrivileges;
bool NSEmailReg;
bool NSModeOnID;
bool NSRestrictGetPass;
bool NSNickTracking;
bool NSAddAccessOnReg;

static std::string CSDefaults;
int CSDefFlags;
int CSMaxReg;
time_t CSExpire;
int CSDefBantype;
int CSAccessMax;
int CSAutokickMax;
char *CSAutokickReason;
time_t CSInhabit;
bool CSListOpersOnly;
int CSListMax;
bool CSRestrictGetPass;
bool CSOpersOnly;

int MSMaxMemos;
time_t MSSendDelay;
bool MSNotifyAll;
int MSMemoReceipt;

static std::string BSDefaults;
int BSDefFlags;
time_t BSKeepData;
int BSMinUsers;
int BSBadWordsMax;
bool BSSmartJoin;
bool BSGentleBWReason;
bool BSCaseSensitive;
char *BSFantasyCharacter;

int HideStatsO;
int GlobalOnCycle;
int AnonymousGlobal;
int RestrictOperNicks;
char *GlobalOnCycleMessage;
char *GlobalOnCycleUP;
static char *ServicesRoot;
char **ServicesRoots;
int RootNumber;
bool SuperAdmin;
bool LogBot;
bool LogMaxUsers;
bool DisableRaw;
time_t AutokillExpiry;
time_t ChankillExpiry;
time_t SGLineExpiry;
time_t SQLineExpiry;
time_t SZLineExpiry;
bool AkillOnAdd;
bool KillonSGline;
bool KillonSQline;
static std::string OSNotifications;
bool WallOper;
bool WallBadOS;
bool WallOSGlobal;
bool WallOSMode;
bool WallOSClearmodes;
bool WallOSKick;
bool WallOSAkill;
bool WallOSSGLine;
bool WallOSSQLine;
bool WallOSSZLine;
bool WallOSNoOp;
bool WallOSJupe;
bool WallOSRaw;
bool WallAkillExpire;
bool WallSGLineExpire;
bool WallSQLineExpire;
bool WallSZLineExpire;
bool WallExceptionExpire;
bool WallDrop;
bool WallForbid;
bool WallGetpass;
bool WallSetpass;
bool AddAkiller;

bool LimitSessions;
int DefSessionLimit;
time_t ExceptionExpiry;
int MaxSessionKill;
int MaxSessionLimit;
time_t SessionAutoKillExpiry;
char *ExceptionDBName;
char *SessionLimitExceeded;
char *SessionLimitDetailsLoc;

bool OSOpersOnly;

char *Modules;
char **ModulesAutoload;
int ModulesNumber;

/**
 * Core Module Stuff
 **/
static char *HostCoreModules;
char **HostServCoreModules;
int HostServCoreNumber;

static char *MemoCoreModules;
char **MemoServCoreModules;
int MemoServCoreNumber;

static char *HelpCoreModules;
char **HelpServCoreModules;
int HelpServCoreNumber;

static char *BotCoreModules;
char **BotServCoreModules;
int BotServCoreNumber;

static char *OperCoreModules;
char **OperServCoreModules;
int OperServCoreNumber;

static char *NickCoreModules;
char **NickServCoreModules;
int NickServCoreNumber;

static char *ChanCoreModules;
char **ChanServCoreModules;
int ChanServCoreNumber;


char *MysqlHost;
char *MysqlUser;
char *MysqlPass;
char *MysqlName;
int MysqlPort;
char *MysqlSecure;
char *MysqlSock;
int MysqlRetries = 0;
int MysqlRetryGap = 0;

int DefConLevel;
static std::string DefCon1;
static std::string DefCon2;
static std::string DefCon3;
static std::string DefCon4;
int DefCon[6];
time_t DefConTimeOut;
int DefConSessionLimit;
time_t DefConAKILL;
char *DefConChanModes;
bool GlobalOnDefcon;
bool GlobalOnDefconMore;
char *DefConOffMessage;
char *DefconMessage;
char *DefConAkillReason;

long unsigned int UserKey1;
long unsigned int UserKey2;
long unsigned int UserKey3;

char *Numeric;

int UnRestrictSAdmin;

char *UlineServers;
char **Ulines;
int NumUlines;

int UseTS6;


/*************************************************************************/

ServerConfig::ServerConfig() : include_stack(), errstr(""), newconfig(), config_data()
{
	this->ClearStack();
}

void ServerConfig::ClearStack()
{
	include_stack.clear();
}

bool ServerConfig::CheckOnce(const char *tag)
{
	int count = ConfValueEnum(config_data, tag);
	if (count > 1) {
		throw ConfigException(static_cast<std::string>("You have more than one <") + tag + "> tag, this is not permitted.");
	}
	if (count < 1) {
		throw ConfigException(static_cast<std::string>("You have not defined a <") + tag + "> tag, this is required.");
	}
	return true;
}

bool NoValidation(ServerConfig *, const char *, const char *, ValueItem &)
{
	return true;
}

bool DoneConfItem(ServerConfig *, const char *)
{
	return true;
}

void ServerConfig::ValidateNoSpaces(const char *p, const std::string &tag, const std::string &val)
{
	for (const char *ptr = p; *ptr; ++ptr) if (*ptr == ' ') throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val +
		"> cannot contain spaces");
}

/* NOTE: Before anyone asks why we're not using inet_pton for this, it is because inet_pton and friends do not return so much detail,
 * even in strerror(errno). They just return 'yes' or 'no' to an address without such detail as to whats WRONG with the address.
 * Because ircd users arent as technical as they used to be (;)) we are going to give more of a useful error message.
 */
void ServerConfig::ValidateIP(const char *p, const std::string &tag, const std::string &val, bool wild)
{
	int num_dots = 0, num_seps = 0;
	bool not_numbers = false, not_hex = false;
	if (*p) {
		if (*p == '.') throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val + "> is not an IP address");
		for (const char *ptr = p; *ptr; ++ptr) {
			if (wild && (*ptr == '*' || *ptr == '?' || *ptr == '/')) continue;
			if (*ptr != ':' && *ptr != '.') {
				if (*ptr < '0' || *ptr > '9') {
					not_numbers = true;
					if (toupper(*ptr) < 'A' || toupper(*ptr) > 'F') not_hex = true;
				}
			}
			switch (*ptr) {
				case ' ':
					throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val + "> is not an IP address");
				case '.':
					++num_dots;
					break;
				case ':':
					++num_seps;
			}
		}
		if (num_dots > 3) throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val +
			"> is an IPv4 address with too many fields!");
		if (num_seps > 8) throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val +
			"> is an IPv6 address with too many fields!");
		if (!num_seps && num_dots < 3 && !wild) throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val +
			"> looks to be a malformed IPv4 address");
		if (!num_seps && num_dots == 3 && not_numbers) throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val +
			"> contains non-numeric characters in an IPv4 address");
		if (num_seps && not_hex) throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val +
			"> contains non-hexdecimal characters in an IPv6 address");
		if (num_seps && num_dots != 3 && num_dots && !wild) throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val +
			"> is a malformed IPv6 4in6 address");
	}
}

void ServerConfig::ValidateHostname(const char *p, const std::string &tag, const std::string &val)
{
	if (!strcasecmp(p, "localhost")) return;
	int num_dots = 0;
	if (*p) {
		if (*p == '.') throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val + "> is not a valid hostname");
		for (const char *ptr = p; *ptr; ++ptr) {
			switch (*ptr) {
				case ' ':
					throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val + "> is not a valid hostname");
				case '.':
					++num_dots;
			}
		}
		if (!num_dots) throw ConfigException(static_cast<std::string>("The value of <") + tag + ":" + val + "> is not a valid hostname");
	}
}

bool ValidateMaxTargets(ServerConfig *, const char *, const char *, ValueItem &data)
{
	if (data.GetInteger() < 0 || data.GetInteger() > 31) {
		alog("WARNING: <options:maxtargets> value is greater than 31 or less than 0, set to 20.");
		data.Set(20);
	}
	return true;
}

bool ValidateNotEmpty(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	if (!*data.GetString()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> cannot be empty!");
	return true;
}

bool ValidateNotZero(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	if (!data.GetInteger()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> must be non-zero!");
	return true;
}

bool ValidateEmailReg(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	if (NSEmailReg) {
		if (static_cast<std::string>(value) == "prenickdatabase") {
			if (!*data.GetString()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> cannot be empty when e-mail registrations are enabled!");
		}
		else if (static_cast<std::string>(value) == "preregexpire") {
			if (!data.GetInteger()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> must be non-zero when e-mail registration are enabled!");
		}
		else {
			if (!data.GetBool()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> must be set to yes when e-mail registrations are enabled!");
		}
	}
	return true;
}

bool ValidatePort(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	int port = data.GetInteger();
	if (!port) return true;
	if (port < 1 || port > 65535) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value +
		"> is not a value port, it must be between 1 and 65535!");
	return true;
}

bool ValidateLanguage(ServerConfig *, const char *, const char *, ValueItem &data)
{
	int language = data.GetInteger();
	char maxlang[3];
	snprintf(maxlang, 3, "%d", USED_LANGS);
	if (language < 1 || language > USED_LANGS) throw ConfigException(static_cast<std::string>("The value for <nickserv:defaultlanguage> must be between 1 and ") + maxlang + "!");
	data.Set(--language);
	return true;
}

bool ValidateGuestPrefix(ServerConfig *conf, const char *tag, const char *value, ValueItem &data)
{
	ValidateNotEmpty(conf, tag, value, data);
	if (strlen(data.GetString()) > 21) throw ConfigException("The value for <nickserv:guestnickprefix> cannot exceed 21 characters in length!");
	return true;
}

bool ValidateBantype(ServerConfig *, const char *, const char *, ValueItem &data)
{
	int bantype = data.GetInteger();
	if (bantype < 0 || bantype > 3) throw ConfigException("The value for <chanserv:defbantype> must be between 0 and 3!");
	return true;
}

bool ValidateBotServ(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	if (s_BotServ) {
		if (static_cast<std::string>(value) == "description" || static_cast<std::string>(value) == "database") {
			if (!*data.GetString()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> cannot be empty when BotServ is enabled!");
		}
		else if (static_cast<std::string>(value) == "minusers" || static_cast<std::string>(value) == "badwordsmax" || static_cast<std::string>(value) == "keepdata") {
			if (!data.GetInteger()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> must be non-zero when BotServ is enabled!");
		}
	}
	return true;
}

bool ValidateHostServ(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	if (s_HostServ) {
		if (static_cast<std::string>(value) == "description" || static_cast<std::string>(value) == "database") {
			if (!*data.GetString()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> cannot be empty when HostServ is enabled!");
		}
	}
	return true;
}

bool ValidateLimitSessions(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	if (LimitSessions) {
		if (static_cast<std::string>(value) == "maxsessionlimit" || static_cast<std::string>(value) == "exceptionexpiry") {
			if (!data.GetInteger()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> must be non-zero when session limiting is enabled!");
		}
	}
	return true;
}

bool ValidateDefCon(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	if (static_cast<std::string>(value) == "defaultlevel") {
		int level = data.GetInteger();
		if (!level) return true;
		if (level > 5) throw ConfigException("The value for <defcon:defaultlevel> must be between 1 through 5 if you wish to use DefCon or 0 if you wish to disable it!");
	}
	else if (DefConLevel) {
		if ((static_cast<std::string>(value).substr(0, 5) == "level" && isdigit(value[5])) || static_cast<std::string>(value) == "chanmodes" || static_cast<std::string>(value) == "akillreason") {
			if (!*data.GetString()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> cannot be empty when DefCon is enabled!");
		}
		else if (static_cast<std::string>(value) == "message" && GlobalOnDefconMore) {
			if (!*data.GetString()) throw ConfigException("The value for <defcon:message> cannot be empty when globalondefconmore is enabled!");
		}
		else if (static_cast<std::string>(value) == "sessionlimit" || static_cast<std::string>(value) == "akillexpire") {
			if (!data.GetInteger()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> must be non-zero when DefCon is enabled!");
		}
	}
	return true;
}

bool ValidateNickLen(ServerConfig *, const char *, const char *, ValueItem &data)
{
	int nicklen = data.GetInteger();
	if (!nicklen) {
		alog("You have not defined the <networkinfo:nicklen> directive. It is strongly");
		alog("adviced that you do configure this correctly in your services.conf");
		data.Set(NICKMAX - 1);
	}
	else if (nicklen < 1 || nicklen >= NICKMAX) {
		alog("<networkinfo:nicklen> has an invalid value; setting to %d", NICKMAX - 1);
		data.Set(NICKMAX - 1);
	}
	return true;
}

bool ValidateMail(ServerConfig *, const char *tag, const char *value, ValueItem &data)
{
	if (UseMail) {
		if (static_cast<std::string>(value) == "sendmailpath" || static_cast<std::string>(value) == "sendfrom") {
			if (!*data.GetString()) throw ConfigException(static_cast<std::string>("The value for <") + tag + ":" + value + "> cannot be empty when e-mail is enabled!");
		}
	}
	return true;
}

void ServerConfig::ReportConfigError(const std::string &errormessage, bool bail)
{
	alog("There were errors in your configuration file: %s", errormessage.c_str());
	if (bail) {
		// TODO -- Need a way to stop loading in a safe way -- CyberBotX
		//ServerInstance->Exit(EXIT_STATUS_CONFIG);
	}
}

int ServerConfig::Read(bool bail)
{
	errstr.clear();
	// These tags MUST occur and must ONLY occur once in the config file
	static const char *Once[] = {"serverinfo", "networkinfo", "options", "nickserv", "chanserv", "memoserv", "helpserv", "operserv", NULL};
	// These tags can occur ONCE or not at all
	InitialConfig Values[] = {
		/* The following comments are from CyberBotX to w00t as examples to use:
		 *
		 * The last argument, for validation, must use one of the functions with the following signature:
		 * bool <function>(ServerConfig *, const char *, const char *, ValueItem &)
		 * Examples are: NoValidation, ValidateNotEmpty, etc.
		 *
		 * If you want to create a directive using an integer:
		 * int blarg;
		 * {"tag", "value", "0", new ValueContainerInt(&blarg), DT_INTEGER, <validation>},
		 *
		 * If you want to create a directive using an unsigned integer:
		 * unsigned blarg;
		 * {"tag", "value", "0", new ValueContainerUInt(&blarg), DT_UINTEGER, <validation>},
		 *
		 * If you want to create a directive using a character pointer without additional validation (see below for hostnames, fields with no spaces, and IP addresses):
		 * char *blarg;
		 * {"tag", "value", "", new ValueContainerChar(&blarg), DT_CHARPTR, <validation>},
		 *
		 * If you want to create a directive using a string:
		 * std::string blarg;
		 * {"tag", "value", "", new ValueContainerString(&blarg), DT_STRING, <validation>},
		 *
		 * If you want to create a directive using a boolean:
		 * bool blarg;
		 * {"tag", "value", "no", new ValueContainerBool(&blarg), DT_BOOLEAN, <validation>},
		 *
		 * If you want to create a directive using a character pointer specifically to hold a hostname (this will call ValidateHostname automatically):
		 * char *blarg;
		 * {"tag", "value", "", new ValueContainerChar(blarg), DT_HOSTNAME, <validation>},
		 *
		 * If you want to create a directive using a character pointer that specifically can not have spaces in it (this will call ValidateNoSpaces automatically):
		 * char *blarg;
		 * {"tag", "value", "", new ValueContainerChar(blarg), DT_NOSPACES, <validation>},
		 *
		 * If you want to create a directive using a character pointer specifically to hold an IP address (this will call ValidateIP automatically):
		 * char *blarg;
		 * {"tag", "value", "", new ValueContainerChar(blarg), DT_IPADDRESS, <validation>},
		 *
		 * If you want to create a directive using a time (a time_t variable converted from a string):
		 * time_t blarg;
		 * {"tag", "value", "", new ValueContainterTime(&blarg), DT_TIME, <validation>},
		 *
		 * For the second-to-last argument, you can or (|) in the following values:
		 * DT_NORELOAD - The variable can't be changed on a reload of the configuration (CURRENTLY NOT SET UP TO WORK YET)
		 * DT_ALLOW_WILD - Allows wildcards/CIDR in DT_IPADDRESS
		 * DT_ALLOW_NEWLINE - Allows new line characters in DT_CHARPTR and DT_STRING
		 *
		 * We may need to add some other validation functions to handle certain things, we can handle that later.
		 * Any questions about these, w00t, feel free to ask. */
		{"uplink", "type", "", new ValueContainerChar(&IRCDModule), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"uplink", "host", "", new ValueContainerChar(&RemoteServer), DT_HOSTNAME | DT_NORELOAD, ValidateNotEmpty},
		{"uplink", "port", "0", new ValueContainerInt(&RemotePort), DT_INTEGER | DT_NORELOAD, ValidatePort},
		{"uplink", "password", "", new ValueContainerChar(&RemotePassword), DT_NOSPACES | DT_NORELOAD, ValidateNotEmpty},
		{"uplink", "id", "", new ValueContainerChar(&Numeric), DT_NOSPACES | DT_NORELOAD, NoValidation},
		{"serverinfo", "name", "", new ValueContainerChar(&ServerName), DT_HOSTNAME | DT_NORELOAD, ValidateNotEmpty},
		{"serverinfo", "description", "", new ValueContainerChar(&ServerDesc), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"serverinfo", "ident", "", new ValueContainerChar(&ServiceUser), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"serverinfo", "hostname", "", new ValueContainerChar(&ServiceHost), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"serverinfo", "pid", "services.pid", new ValueContainerChar(&PIDFilename), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"serverinfo", "motd", "services.motd", new ValueContainerChar(&MOTDFilename), DT_CHARPTR, ValidateNotEmpty},
		{"networkinfo", "helpchannel", "", new ValueContainerChar(&HelpChannel), DT_CHARPTR, NoValidation},
		{"networkinfo", "logchannel", "", new ValueContainerChar(&LogChannel), DT_CHARPTR, NoValidation},
		{"networkinfo", "logbot", "no", new ValueContainerBool(&LogBot), DT_BOOLEAN, NoValidation},
		{"networkinfo", "networkname", "", new ValueContainerChar(&NetworkName), DT_CHARPTR, ValidateNotEmpty},
		{"networkinfo", "nicklen", "0", new ValueContainerInt(&NickLen), DT_INTEGER | DT_NORELOAD, ValidateNickLen},
		{"options", "encryption", "", new ValueContainerChar(&EncModule), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"options", "userkey1", "0", new ValueContainerLUInt(&UserKey1), DT_LUINTEGER, NoValidation},
		{"options", "userkey2", "0", new ValueContainerLUInt(&UserKey2), DT_LUINTEGER, NoValidation},
		{"options", "userkey3", "0", new ValueContainerLUInt(&UserKey3), DT_LUINTEGER, NoValidation},
		{"nickserv", "nick", "NickServ", new ValueContainerChar(&s_NickServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"nickserv", "description", "Nickname Registration Service", new ValueContainerChar(&desc_NickServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"nickserv", "database", "nick.db", new ValueContainerChar(&NickDBName), DT_CHARPTR, ValidateNotEmpty},
		{"nickserv", "emailregistration", "no", new ValueContainerBool(&NSEmailReg), DT_BOOLEAN, NoValidation},
		{"nickserv", "prenickdatabase", "", new ValueContainerChar(&PreNickDBName), DT_CHARPTR, ValidateEmailReg},
		{"nickserv", "modules", "", new ValueContainerChar(&NickCoreModules), DT_CHARPTR, NoValidation},
		{"nickserv", "forceemail", "no", new ValueContainerBool(&NSForceEmail), DT_BOOLEAN, ValidateEmailReg},
		{"nickserv", "defaults", "secure memosignon memoreceive", new ValueContainerString(&NSDefaults), DT_STRING, NoValidation},
		{"nickserv", "defaultlanguage", "0", new ValueContainerInt(&NSDefLanguage), DT_INTEGER, ValidateLanguage},
		{"nickserv", "regdelay", "0", new ValueContainerTime(&NSRegDelay), DT_TIME, NoValidation},
		{"nickserv", "resenddelay", "0", new ValueContainerTime(&NSResendDelay), DT_TIME, NoValidation},
		{"nickserv", "expire", "21d", new ValueContainerTime(&NSExpire), DT_TIME, NoValidation},
		{"nickserv", "preregexpire", "0", new ValueContainerTime(&NSRExpire), DT_TIME, ValidateEmailReg},
		{"nickserv", "maxaliases", "0", new ValueContainerInt(&NSMaxAliases), DT_INTEGER, NoValidation},
		{"nickserv", "accessmax", "0", new ValueContainerInt(&NSAccessMax), DT_INTEGER, ValidateNotZero},
		{"nickserv", "enforceruser", "", new ValueContainerChar(&temp_nsuserhost), DT_CHARPTR, ValidateNotEmpty},
		{"nickserv", "releasetimeout", "0", new ValueContainerTime(&NSReleaseTimeout), DT_TIME, ValidateNotZero},
		{"nickserv", "allowkillimmed", "no", new ValueContainerBool(&NSAllowKillImmed), DT_BOOLEAN | DT_NORELOAD, NoValidation},
		{"nickserv", "nogroupchange", "no", new ValueContainerBool(&NSNoGroupChange), DT_BOOLEAN, NoValidation},
		{"nickserv", "listopersonly", "no", new ValueContainerBool(&NSListOpersOnly), DT_BOOLEAN, NoValidation},
		{"nickserv", "listmax", "0", new ValueContainerInt(&NSListMax), DT_INTEGER, ValidateNotZero},
		{"nickserv", "guestnickprefix", "", new ValueContainerChar(&NSGuestNickPrefix), DT_CHARPTR, ValidateGuestPrefix},
		{"nickserv", "secureadmins", "no", new ValueContainerBool(&NSSecureAdmins), DT_BOOLEAN, NoValidation},
		{"nickserv", "strictprivileges", "no", new ValueContainerBool(&NSStrictPrivileges), DT_BOOLEAN, NoValidation},
		{"nickserv", "modeonid", "no", new ValueContainerBool(&NSModeOnID), DT_BOOLEAN, NoValidation},
		{"nickserv", "restrictgetpass", "no", new ValueContainerBool(&NSRestrictGetPass), DT_BOOLEAN, NoValidation},
		{"nickserv", "nicktracking", "no", new ValueContainerBool(&NSNickTracking), DT_BOOLEAN, NoValidation},
		{"nickserv", "addaccessonreg", "no", new ValueContainerBool(&NSAddAccessOnReg), DT_BOOLEAN, NoValidation},
		{"mail", "usemail", "no", new ValueContainerBool(&UseMail), DT_BOOLEAN, ValidateEmailReg},
		{"mail", "sendmailpath", "", new ValueContainerChar(&SendMailPath), DT_CHARPTR, ValidateMail},
		{"mail", "sendfrom", "", new ValueContainerChar(&SendFrom), DT_CHARPTR, ValidateMail},
		{"mail", "restrict", "no", new ValueContainerBool(&RestrictMail), DT_BOOLEAN, NoValidation},
		{"mail", "delay", "0", new ValueContainerTime(&MailDelay), DT_TIME, NoValidation},
		{"mail", "dontquoteaddresses", "no", new ValueContainerBool(&DontQuoteAddresses), DT_BOOLEAN, NoValidation},
		{"chanserv", "nick", "ChanServ", new ValueContainerChar(&s_ChanServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"chanserv", "description", "Channel Registration Service", new ValueContainerChar(&desc_ChanServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"chanserv", "database", "chan.db", new ValueContainerChar(&ChanDBName), DT_CHARPTR, ValidateNotEmpty},
		{"chanserv", "modules", "", new ValueContainerChar(&ChanCoreModules), DT_CHARPTR, NoValidation},
		{"chanserv", "defaults", "keetopic secure securefounder signkick", new ValueContainerString(&CSDefaults), DT_STRING, NoValidation},
		{"chanserv", "maxregistered", "0", new ValueContainerInt(&CSMaxReg), DT_INTEGER, NoValidation},
		{"chanserv", "expire", "14d", new ValueContainerTime(&CSExpire), DT_TIME, NoValidation},
		{"chanserv", "defbantype", "2", new ValueContainerInt(&CSDefBantype), DT_INTEGER, ValidateBantype},
		{"chanserv", "accessmax", "0", new ValueContainerInt(&CSAccessMax), DT_INTEGER, ValidateNotZero},
		{"chanserv", "autokickmax", "0", new ValueContainerInt(&CSAutokickMax), DT_INTEGER, ValidateNotZero},
		{"chanserv", "autokickreason", "User has been banned from the channel", new ValueContainerChar(&CSAutokickReason), DT_CHARPTR, ValidateNotEmpty},
		{"chanserv", "inhabit", "0", new ValueContainerTime(&CSInhabit), DT_TIME, ValidateNotZero},
		{"chanserv", "listopersonly", "no", new ValueContainerBool(&CSListOpersOnly), DT_BOOLEAN, NoValidation},
		{"chanserv", "listmax", "0", new ValueContainerInt(&CSListMax), DT_INTEGER, ValidateNotZero},
		{"chanserv", "restrictgetpass", "no", new ValueContainerBool(&CSRestrictGetPass), DT_BOOLEAN, NoValidation},
		{"chanserv", "opersonly", "no", new ValueContainerBool(&CSOpersOnly), DT_BOOLEAN, NoValidation},
		{"memoserv", "nick", "MemoServ", new ValueContainerChar(&s_MemoServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"memoserv", "description", "Memo Service", new ValueContainerChar(&desc_MemoServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"memoserv", "modules", "", new ValueContainerChar(&MemoCoreModules), DT_CHARPTR, NoValidation},
		{"memoserv", "maxmemos", "0", new ValueContainerInt(&MSMaxMemos), DT_INTEGER, NoValidation},
		{"memoserv", "senddelay", "0", new ValueContainerTime(&MSSendDelay), DT_TIME, NoValidation},
		{"memoserv", "notifyall", "no", new ValueContainerBool(&MSNotifyAll), DT_BOOLEAN, NoValidation},
		{"memoserv", "memoreceipt", "0", new ValueContainerInt(&MSMemoReceipt), DT_INTEGER, NoValidation},
		{"botserv", "nick", "", new ValueContainerChar(&s_BotServ), DT_CHARPTR | DT_NORELOAD, NoValidation},
		{"botserv", "description", "Bot Service", new ValueContainerChar(&desc_BotServ), DT_CHARPTR | DT_NORELOAD, ValidateBotServ},
		{"botserv", "database", "bot.db", new ValueContainerChar(&BotDBName), DT_CHARPTR, ValidateBotServ},
		{"botserv", "modules", "", new ValueContainerChar(&BotCoreModules), DT_CHARPTR, NoValidation},
		{"botserv", "defaults", "", new ValueContainerString(&BSDefaults), DT_STRING, NoValidation},
		{"botserv", "minusers", "0", new ValueContainerInt(&BSMinUsers), DT_INTEGER, ValidateBotServ},
		{"botserv", "badwordsmax", "0", new ValueContainerInt(&BSBadWordsMax), DT_INTEGER, ValidateBotServ},
		{"botserv", "keepdata", "0", new ValueContainerTime(&BSKeepData), DT_TIME, ValidateBotServ},
		{"botserv", "smartjoin", "no", new ValueContainerBool(&BSSmartJoin), DT_BOOLEAN, NoValidation},
		{"botserv", "gentlebadwordreason", "no", new ValueContainerBool(&BSGentleBWReason), DT_BOOLEAN, NoValidation},
		{"botserv", "casesensitive", "no", new ValueContainerBool(&BSCaseSensitive), DT_BOOLEAN, NoValidation},
		{"botserv", "fantasycharacter", "!", new ValueContainerChar(&BSFantasyCharacter), DT_CHARPTR, NoValidation},
		{"hostserv", "nick", "", new ValueContainerChar(&s_HostServ), DT_CHARPTR | DT_NORELOAD, NoValidation},
		{"hostserv", "description", "vHost Service", new ValueContainerChar(&desc_HostServ), DT_CHARPTR | DT_NORELOAD, ValidateHostServ},
		{"hostserv", "database", "hosts.db", new ValueContainerChar(&HostDBName), DT_CHARPTR, ValidateHostServ},
		{"hostserv", "modules", "", new ValueContainerChar(&HostCoreModules), DT_CHARPTR, NoValidation},
		{"hostserv", "hostsetters", "", new ValueContainerChar(&HostSetter), DT_CHARPTR, NoValidation},
		{"helpserv", "nick", "HelpServ", new ValueContainerChar(&s_HelpServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"helpserv", "description", "Help Service", new ValueContainerChar(&desc_HelpServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"helpserv", "modules", "", new ValueContainerChar(&HelpCoreModules), DT_CHARPTR, NoValidation},
		{"operserv", "nick", "OperServ", new ValueContainerChar(&s_OperServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"operserv", "description", "Operator Service", new ValueContainerChar(&desc_OperServ), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"operserv", "globalnick", "Global", new ValueContainerChar(&s_GlobalNoticer), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"operserv", "globaldescription", "Global Noticer", new ValueContainerChar(&desc_GlobalNoticer), DT_CHARPTR | DT_NORELOAD, ValidateNotEmpty},
		{"operserv", "database", "oper.db", new ValueContainerChar(&OperDBName), DT_CHARPTR, ValidateNotEmpty},
		{"operserv", "newsdatabase", "news.db", new ValueContainerChar(&NewsDBName), DT_CHARPTR, ValidateNotEmpty},
		{"operserv", "exceptiondatabase", "exception.db", new ValueContainerChar(&ExceptionDBName), DT_CHARPTR, ValidateNotEmpty},
		{"operserv", "autokilldatabase", "akill.db", new ValueContainerChar(&AutokillDBName), DT_CHARPTR, ValidateNotEmpty},
		{"operserv", "modules", "", new ValueContainerChar(&OperCoreModules), DT_CHARPTR, NoValidation},
		{"operserv", "servicesroot", "", new ValueContainerChar(&ServicesRoot), DT_CHARPTR, ValidateNotEmpty},
		{"operserv", "superadmin", "no", new ValueContainerBool(&SuperAdmin), DT_BOOLEAN, NoValidation},
		{"operserv", "logmaxusers", "no", new ValueContainerBool(&LogMaxUsers), DT_BOOLEAN, NoValidation},
		{"operserv", "autokillexpiry", "0", new ValueContainerTime(&AutokillExpiry), DT_TIME, ValidateNotZero},
		{"operserv", "chankillexpiry", "0", new ValueContainerTime(&ChankillExpiry), DT_TIME, ValidateNotZero},
		{"operserv", "sglineexpiry", "0", new ValueContainerTime(&SGLineExpiry), DT_TIME, ValidateNotZero},
		{"operserv", "sqlineexpiry", "0", new ValueContainerTime(&SQLineExpiry), DT_TIME, ValidateNotZero},
		{"operserv", "szlineexpiry", "0", new ValueContainerTime(&SZLineExpiry), DT_TIME, ValidateNotZero},
		{"operserv", "akillonadd", "no", new ValueContainerBool(&AkillOnAdd), DT_BOOLEAN, NoValidation},
		{"operserv", "killonsgline", "no", new ValueContainerBool(&KillonSGline), DT_BOOLEAN, NoValidation},
		{"operserv", "killonsqline", "no", new ValueContainerBool(&KillonSQline), DT_BOOLEAN, NoValidation},
		{"operserv", "disableraw", "no", new ValueContainerBool(&DisableRaw), DT_BOOLEAN, NoValidation},
		{"operserv", "notifications", "", new ValueContainerString(&OSNotifications), DT_STRING, NoValidation},
		{"operserv", "limitsessions", "no", new ValueContainerBool(&LimitSessions), DT_BOOLEAN, NoValidation},
		{"operserv", "defaultsessionlimit", "0", new ValueContainerInt(&DefSessionLimit), DT_INTEGER, NoValidation},
		{"operserv", "maxsessionlimit", "0", new ValueContainerInt(&MaxSessionLimit), DT_INTEGER, ValidateLimitSessions},
		{"operserv", "exceptionexpiry", "0", new ValueContainerTime(&ExceptionExpiry), DT_TIME, ValidateLimitSessions},
		{"operserv", "sessionlimitexceeded", "", new ValueContainerChar(&SessionLimitExceeded), DT_CHARPTR, NoValidation},
		{"operserv", "sessionlimitdetailsloc", "", new ValueContainerChar(&SessionLimitDetailsLoc), DT_CHARPTR, NoValidation},
		{"operserv", "maxsessionkill", "0", new ValueContainerInt(&MaxSessionKill), DT_INTEGER, NoValidation},
		{"operserv", "sessionautokillexpiry", "0", new ValueContainerTime(&SessionAutoKillExpiry), DT_TIME, NoValidation},
		{"operserv", "addakiller", "no", new ValueContainerBool(&AddAkiller), DT_BOOLEAN, NoValidation},
		{"opserver", "opersonly", "no", new ValueContainerBool(&OSOpersOnly), DT_BOOLEAN, NoValidation},
		{"defcon", "defaultlevel", "0", new ValueContainerInt(&DefConLevel), DT_INTEGER, ValidateDefCon},
		{"defcon", "level4", "", new ValueContainerString(&DefCon4), DT_STRING, ValidateDefCon},
		{"defcon", "level3", "", new ValueContainerString(&DefCon3), DT_STRING, ValidateDefCon},
		{"defcon", "level2", "", new ValueContainerString(&DefCon2), DT_STRING, ValidateDefCon},
		{"defcon", "level1", "", new ValueContainerString(&DefCon1), DT_STRING, ValidateDefCon},
		{"defcon", "sessionlimit", "0", new ValueContainerInt(&DefConSessionLimit), DT_INTEGER, ValidateDefCon},
		{"defcon", "akillexpire", "0", new ValueContainerTime(&DefConAKILL), DT_TIME, ValidateDefCon},
		{"defcon", "chanmodes", "", new ValueContainerChar(&DefConChanModes), DT_CHARPTR, ValidateDefCon},
		{"defcon", "timeout", "0", new ValueContainerTime(&DefConTimeOut), DT_TIME, NoValidation},
		{"defcon", "globalondefcon", "no", new ValueContainerBool(&GlobalOnDefcon), DT_BOOLEAN, NoValidation},
		{"defcon", "globalondefconmore", "no", new ValueContainerBool(&GlobalOnDefconMore), DT_BOOLEAN, NoValidation},
		{"defcon", "message", "", new ValueContainerChar(&DefconMessage), DT_CHARPTR, ValidateDefCon},
		{"defcon", "offmessage", "", new ValueContainerChar(&DefConOffMessage), DT_CHARPTR, NoValidation},
		{"defcon", "akillreason", "", new ValueContainerChar(&DefConAkillReason), DT_CHARPTR, ValidateDefCon},
		{NULL, NULL, NULL, NULL, DT_NOTHING, NoValidation}
	};
	/* These tags can occur multiple times, and therefore they have special code to read them
	 * which is different to the code for reading the singular tags listed above. */
	MultiConfig MultiValues[] = {
		{NULL,
			{NULL},
			{NULL},
			{0},
			NULL, NULL, NULL}
	};
	// Load and parse the config file, if there are any errors then explode
	// Make a copy here so if it fails then we can carry on running with an unaffected config
	newconfig.clear();
	if (LoadConf(newconfig, SERVICES_CONF_NEW, errstr)) {
		// If we succeeded, set the ircd config to the new one
		config_data = newconfig;
	}
	else {
		ReportConfigError(errstr.str(), bail);
		return 0;
	}
	// The stuff in here may throw CoreException, be sure we're in a position to catch it.
	try {
		// Read the values of all the tags which occur once or not at all, and call their callbacks.
		for (int Index = 0; Values[Index].tag; ++Index) {
			char item[BUFSIZE];
			int dt = Values[Index].datatype;
			bool allow_newlines = dt & DT_ALLOW_NEWLINE, allow_wild = dt & DT_ALLOW_WILD, noreload = dt & DT_NORELOAD;
			dt &= ~DT_ALLOW_NEWLINE;
			dt &= ~DT_ALLOW_WILD;
			dt &= ~DT_NORELOAD;
			// If the value is set to not allow reloading and we are reloading (bail will be true), skip the item
			if (noreload && bail)
				continue;
			ConfValue(config_data, Values[Index].tag, Values[Index].value, Values[Index].default_value, 0, item, BUFSIZE, allow_newlines);
			ValueItem vi(item);
			if (!Values[Index].validation_function(this, Values[Index].tag, Values[Index].value, vi))
				throw ConfigException("One or more values in your configuration file failed to validate. Please see your ircd.log for more information.");
			switch (dt) {
				case DT_NOSPACES: {
					ValueContainerChar *vcc = dynamic_cast<ValueContainerChar *>(Values[Index].val);
					ValidateNoSpaces(vi.GetString(), Values[Index].tag, Values[Index].value);
					vcc->Set(vi.GetString(), strlen(vi.GetString()) + 1);
				}
				break;
				case DT_HOSTNAME: {
					ValueContainerChar *vcc = dynamic_cast<ValueContainerChar *>(Values[Index].val);
					ValidateHostname(vi.GetString(), Values[Index].tag, Values[Index].value);
					vcc->Set(vi.GetString(), strlen(vi.GetString()) + 1);
				}
				break;
				case DT_IPADDRESS: {
					ValueContainerChar *vcc = dynamic_cast<ValueContainerChar *>(Values[Index].val);
					ValidateIP(vi.GetString(), Values[Index].tag, Values[Index].value, allow_wild);
					vcc->Set(vi.GetString(), strlen(vi.GetString()) + 1);
				}
				break;
				case DT_CHARPTR: {
					ValueContainerChar *vcc = dynamic_cast<ValueContainerChar *>(Values[Index].val);
					// Make sure we also copy the null terminator
					vcc->Set(vi.GetString(), strlen(vi.GetString()) + 1);
				}
				break;
				case DT_STRING: {
					ValueContainerString *vcs = dynamic_cast<ValueContainerString *>(Values[Index].val);
					vcs->Set(vi.GetString());
				}
				break;
				case DT_INTEGER: {
					int val = vi.GetInteger();
					ValueContainerInt *vci = dynamic_cast<ValueContainerInt *>(Values[Index].val);
					vci->Set(&val, sizeof(int));
				}
				break;
				case DT_UINTEGER: {
					unsigned val = vi.GetInteger();
					ValueContainerUInt *vci = dynamic_cast<ValueContainerUInt *>(Values[Index].val);
					vci->Set(&val, sizeof(unsigned));
				}
				break;
				case DT_LUINTEGER: {
					long unsigned val = vi.GetInteger();
					ValueContainerLUInt *vci = dynamic_cast<ValueContainerLUInt *>(Values[Index].val);
					vci->Set(&val, sizeof(long unsigned));
				}
				break;
				case DT_TIME: {
					time_t time = dotime(vi.GetString());
					ValueContainerTime *vci = dynamic_cast<ValueContainerTime *>(Values[Index].val);
					vci->Set(&time, sizeof(time_t));
				}
				break;
				case DT_BOOLEAN: {
					bool val = vi.GetBool();
					ValueContainerBool *vcb = dynamic_cast<ValueContainerBool *>(Values[Index].val);
					vcb->Set(&val, sizeof(bool));
				}
				break;
				default:
					break;
			}
			// We're done with this now
			delete Values[Index].val;
		}
		/* Read the multiple-tag items (class tags, connect tags, etc)
		 * and call the callbacks associated with them. We have three
		 * callbacks for these, a 'start', 'item' and 'end' callback. */
		for (int Index = 0; MultiValues[Index].tag; ++Index) {
			MultiValues[Index].init_function(this, MultiValues[Index].tag);
			int number_of_tags = ConfValueEnum(config_data, MultiValues[Index].tag);
			for (int tagnum = 0; tagnum < number_of_tags; ++tagnum) {
				ValueList vl;
				for (int valuenum = 0; MultiValues[Index].items[valuenum]; ++valuenum) {
					int dt = MultiValues[Index].datatype[valuenum];
					bool allow_newlines =  dt & DT_ALLOW_NEWLINE, allow_wild = dt & DT_ALLOW_WILD, noreload = dt & DT_NORELOAD;
					dt &= ~DT_ALLOW_NEWLINE;
					dt &= ~DT_ALLOW_WILD;
					dt &= ~DT_NORELOAD;
					// If the value is set to not allow reloading and we are reloading (bail will be true), skip the item
					if (noreload && bail)
						continue;
					switch (dt) {
						case DT_NOSPACES: {
							char item[BUFSIZE];
							if (ConfValue(config_data, MultiValues[Index].tag, MultiValues[Index].items[valuenum],
								MultiValues[Index].items_default[valuenum], tagnum, item, BUFSIZE, allow_newlines)) {
								vl.push_back(ValueItem(item));
							}
							else vl.push_back(ValueItem(""));
							ValidateNoSpaces(vl[vl.size() - 1].GetString(), MultiValues[Index].tag, MultiValues[Index].items[valuenum]);
						}
						break;
						case DT_HOSTNAME: {
							char item[BUFSIZE];
							if (ConfValue(config_data, MultiValues[Index].tag, MultiValues[Index].items[valuenum],
								MultiValues[Index].items_default[valuenum], tagnum, item, BUFSIZE, allow_newlines)) {
								vl.push_back(ValueItem(item));
							}
							else vl.push_back(ValueItem(""));
							ValidateHostname(vl[vl.size() - 1].GetString(), MultiValues[Index].tag, MultiValues[Index].items[valuenum]);
						}
						break;
						case DT_IPADDRESS: {
							char item[BUFSIZE];
							if (ConfValue(config_data, MultiValues[Index].tag, MultiValues[Index].items[valuenum],
								MultiValues[Index].items_default[valuenum], tagnum, item, BUFSIZE, allow_newlines)) {
								vl.push_back(ValueItem(item));
							}
							else vl.push_back(ValueItem(""));
							ValidateIP(vl[vl.size() - 1].GetString(), MultiValues[Index].tag, MultiValues[Index].items[valuenum], allow_wild);
						}
						break;
						case DT_CHARPTR: {
							char item[BUFSIZE];
							if (ConfValue(config_data, MultiValues[Index].tag, MultiValues[Index].items[valuenum],
								MultiValues[Index].items_default[valuenum], tagnum, item, BUFSIZE, allow_newlines)) {
								vl.push_back(ValueItem(item));
							}
							else vl.push_back(ValueItem(""));
						}
						break;
						case DT_STRING: {
							std::string item;
							if (ConfValue(config_data, static_cast<std::string>(MultiValues[Index].tag),
								static_cast<std::string>(MultiValues[Index].items[valuenum]),
								static_cast<std::string>(MultiValues[Index].items_default[valuenum]), tagnum, item, allow_newlines)) {
								vl.push_back(ValueItem(item));
							}
							else vl.push_back(ValueItem(""));
						}
						break;
						case DT_INTEGER:
						case DT_UINTEGER: {
							int item = 0;
							if (ConfValueInteger(config_data, MultiValues[Index].tag, MultiValues[Index].items[valuenum],
								MultiValues[Index].items_default[valuenum], tagnum, item)) vl.push_back(ValueItem(item));
							else vl.push_back(ValueItem(0));
						}
						break;
						case DT_TIME: {
							std::string item;
							if (ConfValue(config_data, static_cast<std::string>(MultiValues[Index].tag),
								static_cast<std::string>(MultiValues[Index].items[valuenum]),
								static_cast<std::string>(MultiValues[Index].items_default[valuenum]), tagnum, item, allow_newlines)) {
								int time = dotime(item.c_str());
								vl.push_back(ValueItem(time));
							}
							else vl.push_back(ValueItem(0));
						}
						break;
						case DT_BOOLEAN: {
							bool item = ConfValueBool(config_data, MultiValues[Index].tag, MultiValues[Index].items[valuenum],
								MultiValues[Index].items_default[valuenum], tagnum);
							vl.push_back(ValueItem(item));
						}
					}
				}
				MultiValues[Index].validation_function(this, MultiValues[Index].tag, static_cast<const char **>(MultiValues[Index].items), vl,
					MultiValues[Index].datatype);
			}
			MultiValues[Index].finish_function(this, MultiValues[Index].tag);
		}
	}
	catch (ConfigException &ce) {
		ReportConfigError(ce.GetReason(), bail);
		return 0;
	}
	if (debug) alog("End config");
	for (int Index = 0; Once[Index]; ++Index) if (!CheckOnce(Once[Index])) return 0;
	alog("Done reading configuration file.");
	return 1;
}

bool ServerConfig::LoadConf(ConfigDataHash &target, const char *filename, std::ostringstream &errorstream)
{
	std::string line, wordbuffer, section, itemname;
	std::ifstream conf(filename);
	int linenumber = 0;
	bool in_word = false, in_quote = false, in_ml_comment = false;
	KeyValList sectiondata;
	if (conf.fail()) {
		errorstream << "File " << filename << " could not be opened." << std::endl;
		return false;
	}
	if (debug) alog("Start to read conf %s", filename);
	// Start reading characters...
	while (getline(conf, line)) {
		++linenumber;
		unsigned c = 0, len = line.size();
		for (; c < len; ++c) {
			char ch = line[c];
			if (in_quote) {
				if (ch == '"') {
					in_quote = in_word = false;
					continue;
				}
				wordbuffer += ch;
				continue;
			}
			if (in_ml_comment) {
				if (ch == '*' && c + 1 < len && line[c + 1] == '/') {
					in_ml_comment = false;
					++c;
				}
				continue;
			}
			if (ch == '#' || (ch == '/' && c + 1 < len && line[c + 1] == '/')) break; // Line comment, ignore the rest of the line (much like this one!)
			else if (ch == '/' && c + 1 < len && line[c + 1] == '*') {
				// Multiline (or less than one line) comment
				in_ml_comment = true;
				++c;
				continue;
			}
			else if (ch == '"') {
				// Quotes are valid only in the value position
				if (section.empty() || itemname.empty()) {
					errorstream << "Unexpected quoted string: " << filename << ":" << linenumber << std::endl;
					return false;
				}
				if (in_word || !wordbuffer.empty()) {
					errorstream << "Unexpected quoted string (prior unhandled words): " << filename << ":" << linenumber << std::endl;
					return false;
				}
				in_quote = in_word = true;
				continue;
			}
			else if (ch == '=') {
				if (section.empty()) {
					errorstream << "Config item outside of section (or stray '='): " << filename << ":" << linenumber << std::endl;
					return false;
				}
				if (!itemname.empty()) {
					errorstream << "Stray '=' sign or item without value: " << filename << ":" << linenumber << std::endl;
					return false;
				}
				if (in_word) in_word = false;
				itemname = wordbuffer;
				wordbuffer.clear();
			}
			else if (ch == '{') {
				if (!section.empty()) {
					errorstream << "Section inside another section: " << filename << ":" << linenumber << std::endl;
					return false;
				}
				if (wordbuffer.empty()) {
					errorstream << "Section without a name or unexpected '{': " << filename << ":" << linenumber << std::endl;
					return false;
				}
				if (in_word) in_word = false;
				section = wordbuffer;
				wordbuffer.clear();
			}
			else if (ch == '}') {
				if (section.empty()) {
					errorstream << "Stray '}': " << filename << ":" << linenumber << std::endl;
					return false;
				}
				if (!wordbuffer.empty() || !itemname.empty()) {
					// this will allow for the construct:  section { key = value }
					// but will not allow for anything else, such as:  section { key = value; key = value }
					if (!sectiondata.empty()) {
						errorstream << "Unexpected end of section: " << filename << ":" << linenumber << std::endl;
						return false;
					}
					// this is the same as the below section for testing if itemname is non-empty after the loop, but done inside it to allow the above construct
					if (debug) alog("ln %d EOL: s='%s' '%s' set to '%s'", linenumber, section.c_str(), itemname.c_str(), wordbuffer.c_str());
					sectiondata.push_back(KeyVal(itemname, wordbuffer));
					wordbuffer.clear();
					itemname.clear();
				}
				target.insert(std::pair<std::string, KeyValList>(section, sectiondata));
				section.clear();
				sectiondata.clear();
			}
			else if (ch == ';' || ch == '\r') continue; // Ignore
			else if (ch == ' ' || ch == '\t') {
				// Terminate word
				if (in_word) in_word = false;
			}
			else {
				if (!in_word && !wordbuffer.empty()) {
					errorstream << "Unexpected word: " << filename << ":" << linenumber << std::endl;
					return false;
				}
				wordbuffer += ch;
				in_word = true;
			}
		}
		if (in_quote) {
			// Quotes can span multiple lines; all we need to do is go to the next line without clearing things
			wordbuffer += "\n";
			continue;
		}
		in_word = false;
		if (!itemname.empty()) {
			if (wordbuffer.empty()) {
				errorstream << "Item without value: " << filename << ":" << linenumber << std::endl;
				return false;
			}
			if (debug) alog("ln %d EOL: s='%s' '%s' set to '%s'", linenumber, section.c_str(), itemname.c_str(), wordbuffer.c_str());
			sectiondata.push_back(KeyVal(itemname, wordbuffer));
			wordbuffer.clear();
			itemname.clear();
		}
	}
	if (in_ml_comment) {
		errorstream << "Unterminated multiline comment at end of file: " << filename << std::endl;
		return false;
	}
	if (in_quote) {
		errorstream << "Unterminated quote at end of file: " << filename << std::endl;
		return false;
	}
	if (!itemname.empty() || !wordbuffer.empty()) {
		errorstream << "Unexpected garbage at end of file: " << filename << std::endl;
		return false;
	}
	if (!section.empty()) {
		errorstream << "Unterminated section at end of file: " << filename << std::endl;
		return false;
	}
	return true;
}

bool ServerConfig::LoadConf(ConfigDataHash &target, const std::string &filename, std::ostringstream &errorstream)
{
	return LoadConf(target, filename.c_str(), errorstream);
}

bool ServerConfig::ConfValue(ConfigDataHash &target, const char *tag, const char *var, int index, char *result, int length, bool allow_linefeeds)
{
	return ConfValue(target, tag, var, "", index, result, length, allow_linefeeds);
}

bool ServerConfig::ConfValue(ConfigDataHash &target, const char *tag, const char *var, const char *default_value, int index, char *result,
	int length, bool allow_linefeeds)
{
	std::string value;
	bool r = ConfValue(target, static_cast<std::string>(tag), static_cast<std::string>(var), static_cast<std::string>(default_value), index, value,
		allow_linefeeds);
	strlcpy(result, value.c_str(), length);
	return r;
}

bool ServerConfig::ConfValue(ConfigDataHash &target, const std::string &tag, const std::string &var, int index, std::string &result,
	bool allow_linefeeds)
{
	return ConfValue(target, tag, var, "", index, result, allow_linefeeds);
}

bool ServerConfig::ConfValue(ConfigDataHash &target, const std::string &tag, const std::string &var, const std::string &default_value, int index,
	std::string &result, bool allow_linefeeds)
{
	ConfigDataHash::size_type pos = index;
	if (pos < target.count(tag)) {
		ConfigDataHash::iterator iter = target.find(tag);
		for (int i = 0; i < index; ++i) ++iter;
		KeyValList::iterator j = iter->second.begin(), jend = iter->second.end();
		for (; j != jend; ++j) {
			if (j->first == var) {
				if (!allow_linefeeds && j->second.find('\n') != std::string::npos) {
					alog("Value of <%s:%s> contains a linefeed, and linefeeds in this value are not permitted -- stripped to spaces.", tag.c_str(), var.c_str());
					std::string::iterator n = j->second.begin(), nend = j->second.end();
					for (; n != nend; ++n) if (*n == '\n') *n = ' ';
				}
				else {
					result = j->second;
					return true;
				}
			}
		}
		if (!default_value.empty()) {
			result = default_value;
			return true;
		}
	}
	else if (!pos) {
		if (!default_value.empty()) {
			result = default_value;
			return true;
		}
	}
	return false;
}

bool ServerConfig::ConfValueInteger(ConfigDataHash &target, const char *tag, const char *var, int index, int &result)
{
	return ConfValueInteger(target, static_cast<std::string>(tag), static_cast<std::string>(var), "", index, result);
}

bool ServerConfig::ConfValueInteger(ConfigDataHash &target, const char *tag, const char *var, const char *default_value, int index, int &result)
{
	return ConfValueInteger(target, static_cast<std::string>(tag), static_cast<std::string>(var), static_cast<std::string>(default_value), index,
		result);
}

bool ServerConfig::ConfValueInteger(ConfigDataHash &target, const std::string &tag, const std::string &var, int index, int &result)
{
	return ConfValueInteger(target, tag, var, "", index, result);
}

bool ServerConfig::ConfValueInteger(ConfigDataHash &target, const std::string &tag, const std::string &var, const std::string &default_value, int index, int &result)
{
	std::string value;
	std::istringstream stream;
	bool r = ConfValue(target, tag, var, default_value, index, value);
	stream.str(value);
	if (!(stream >> result)) return false;
	else {
		if (!value.empty()) {
			if (value.substr(0, 2) == "0x") {
				char *endptr;
				value.erase(0, 2);
				result = strtol(value.c_str(), &endptr, 16);
				/* No digits found */
				if (endptr == value.c_str()) return false;
			}
			else {
				char denominator = *(value.end() - 1);
				switch (toupper(denominator)) {
					case 'K':
						// Kilobytes -> bytes
						result = result * 1024;
						break;
					case 'M':
						// Megabytes -> bytes
						result = result * 1048576;
						break;
					case 'G':
						// Gigabytes -> bytes
						result = result * 1073741824;
						break;
				}
			}
		}
	}
	return r;
}

bool ServerConfig::ConfValueBool(ConfigDataHash &target, const char *tag, const char *var, int index)
{
	return ConfValueBool(target, static_cast<std::string>(tag), static_cast<std::string>(var), "", index);
}

bool ServerConfig::ConfValueBool(ConfigDataHash &target, const char *tag, const char *var, const char *default_value, int index)
{
	return ConfValueBool(target, static_cast<std::string>(tag), static_cast<std::string>(var), static_cast<std::string>(default_value), index);
}

bool ServerConfig::ConfValueBool(ConfigDataHash &target, const std::string &tag, const std::string &var, int index)
{
	return ConfValueBool(target, tag, var, "", index);
}

bool ServerConfig::ConfValueBool(ConfigDataHash &target, const std::string &tag, const std::string &var, const std::string &default_value, int index)
{
	std::string result;
	if (!ConfValue(target, tag, var, default_value, index, result)) return false;
	return result == "yes" || result == "true" || result == "1";
}

int ServerConfig::ConfValueEnum(ConfigDataHash &target, const char *tag)
{
	return target.count(tag);
}

int ServerConfig::ConfValueEnum(ConfigDataHash &target, const std::string &tag)
{
	return target.count(tag);
}

int ServerConfig::ConfVarEnum(ConfigDataHash &target, const char *tag, int index)
{
	return ConfVarEnum(target, static_cast<std::string>(tag), index);
}

int ServerConfig::ConfVarEnum(ConfigDataHash &target, const std::string &tag, int index)
{
	ConfigDataHash::size_type pos = index;
	if (pos < target.count(tag)) {
		ConfigDataHash::const_iterator iter = target.find(tag);
		for (int i = 0; i < index; ++i) ++iter;
		return iter->second.size();
	}
	return 0;
}

ValueItem::ValueItem(int value) : v("")
{
	std::stringstream n;
	n << value;
	v = n.str();
}

ValueItem::ValueItem(bool value) : v("")
{
	std::stringstream n;
	n << value;
	v = n.str();
}

ValueItem::ValueItem(const char *value) : v(value) { }

ValueItem::ValueItem(const std::string &value) : v(value) { }

void ValueItem::Set(const char *value)
{
	v = value;
}

void ValueItem::Set(const std::string &value)
{
	v = value;
}

void ValueItem::Set(int value)
{
	std::stringstream n;
	n << value;
	v = n.str();
}

int ValueItem::GetInteger()
{
	if (v.empty()) return 0;
	return atoi(v.c_str());
}

char *ValueItem::GetString()
{
	return const_cast<char *>(v.c_str());
}

bool ValueItem::GetBool()
{
	return GetInteger() || v == "yes" || v == "true";
}

/*************************************************************************/

/* Deprecated directive (dep_) and value checking (chk_) functions: */

/* Hey, there are no left! -GD */

/*************************************************************************/

Directive directives[] = {
	{"BadPassLimit", {{PARAM_POSINT, PARAM_RELOAD, &BadPassLimit}}},
	{"BadPassTimeout", {{PARAM_TIME, PARAM_RELOAD, &BadPassTimeout}}},
	{"DumpCore", {{PARAM_SET, 0, &DumpCore}}},
	{"ExpireTimeout", {{PARAM_TIME, PARAM_RELOAD, &ExpireTimeout}}},
	{"ForceForbidReason", {{PARAM_SET, PARAM_RELOAD, &ForceForbidReason}}},
	{"KeepBackups", {{PARAM_INT, PARAM_RELOAD, &KeepBackups}}},
	{"KeepLogs", {{PARAM_INT, PARAM_RELOAD, &KeepLogs}}},
	{"LocalAddress", {{PARAM_STRING, 0, &LocalHost},
					  {PARAM_PORT, PARAM_OPTIONAL, &LocalPort}}},
	{"LogUsers", {{PARAM_SET, PARAM_RELOAD, &LogUsers}}},
	{"MysqlHost", {{PARAM_STRING, PARAM_RELOAD, &MysqlHost}}},
	{"MysqlUser", {{PARAM_STRING, PARAM_RELOAD, &MysqlUser}}},
	{"MysqlPass", {{PARAM_STRING, PARAM_RELOAD, &MysqlPass}}},
	{"MysqlName", {{PARAM_STRING, PARAM_RELOAD, &MysqlName}}},
	{"MysqlPort", {{PARAM_PORT, PARAM_RELOAD, &MysqlPort}}},
	{"MysqlSecure", {{PARAM_STRING, PARAM_RELOAD, &MysqlSecure}}},
	{"MysqlSock", {{PARAM_STRING, PARAM_RELOAD, &MysqlSock}}},
	{"MysqlRetries", {{PARAM_POSINT, PARAM_RELOAD, &MysqlRetries}}},
	{"MysqlRetryGap", {{PARAM_POSINT, PARAM_RELOAD, &MysqlRetryGap}}},
	{"ModuleAutoload", {{PARAM_STRING, PARAM_RELOAD, &Modules}}},
	{"NewsCount", {{PARAM_POSINT, PARAM_RELOAD, &NewsCount}}},
	{"NickRegDelay", {{PARAM_POSINT, PARAM_RELOAD, &NickRegDelay}}},
	{"NoBackupOkay", {{PARAM_SET, PARAM_RELOAD, &NoBackupOkay}}},
	{"ReadTimeout", {{PARAM_TIME, PARAM_RELOAD, &ReadTimeout}}},
	{"RemoteServer2", {{PARAM_STRING, 0, &RemoteServer2},
					   {PARAM_PORT, 0, &RemotePort2},
					   {PARAM_STRING, 0, &RemotePassword2}}},
	{"RemoteServer3", {{PARAM_STRING, 0, &RemoteServer3},
					   {PARAM_PORT, 0, &RemotePort3},
					   {PARAM_STRING, 0, &RemotePassword3}}},
	{"RestrictOperNicks", {{PARAM_SET, PARAM_RELOAD, &RestrictOperNicks}}},
	{"HideStatsO", {{PARAM_SET, PARAM_RELOAD, &HideStatsO}}},
	{"GlobalOnCycle", {{PARAM_SET, PARAM_RELOAD, &GlobalOnCycle}}},
	{"AnonymousGlobal", {{PARAM_SET, PARAM_RELOAD, &AnonymousGlobal}}},
	{"GlobalOnCycleMessage",
	 {{PARAM_STRING, PARAM_RELOAD, &GlobalOnCycleMessage}}},
	{"GlobalOnCycleUP", {{PARAM_STRING, PARAM_RELOAD, &GlobalOnCycleUP}}},
	{"StrictPasswords", {{PARAM_SET, PARAM_RELOAD, &StrictPasswords}}},
	{"TimeoutCheck", {{PARAM_TIME, PARAM_RELOAD, &TimeoutCheck}}},
	{"UpdateTimeout", {{PARAM_TIME, PARAM_RELOAD, &UpdateTimeout}}},
	{"UsePrivmsg", {{PARAM_SET, PARAM_RELOAD, &UsePrivmsg}}},
	{"UseStrictPrivMsg", {{PARAM_SET, PARAM_RELOAD, &UseStrictPrivMsg}}},
	{"UseSVSHOLD", {{PARAM_SET, PARAM_RELOAD, &UseSVSHOLD}}},
	{"UseTS6", {{PARAM_SET, 0, &UseTS6}}},
	{"UnRestrictSAdmin", {{PARAM_SET, PARAM_RELOAD, &UnRestrictSAdmin}}},
	{"WarningTimeout", {{PARAM_TIME, PARAM_RELOAD, &WarningTimeout}}},
	{"UlineServers", {{PARAM_STRING, PARAM_RELOAD, &UlineServers}}},
};

/*************************************************************************/

/* Print an error message to the log (and the console, if open). */

void error(int linenum, const char *message, ...)
{
	char buf[4096];
	va_list args;

	va_start(args, message);
	vsnprintf(buf, sizeof(buf), message, args);
	va_end(args);

	if (linenum)
		alog("%s:%d: %s", SERVICES_CONF, linenum, buf);
	else
		alog("%s: %s", SERVICES_CONF, buf);

	if (!nofork && isatty(2)) {
		if (linenum)
			fprintf(stderr, "%s:%d: %s\n", SERVICES_CONF, linenum, buf);
		else
			fprintf(stderr, "%s: %s\n", SERVICES_CONF, buf);
	}
}

/*************************************************************************/

/* Parse a configuration line.  Return 1 on success; otherwise, print an
 * appropriate error message and return 0.  Destroys the buffer by side
 * effect.
 */

int parse_directive(Directive * d, char *dir, int ac, char *av[MAXPARAMS],
					int linenum, int reload, char *s)
{
	int retval = 1;
	int i;
	long val;
	int myoptind;

	if (stricmp(dir, d->name) != 0)
		return 1;
	myoptind = 0;
	for (i = 0; i < MAXPARAMS && d->params[i].type != PARAM_NONE; i++) {
		if (reload && !(d->params[i].flags & PARAM_RELOAD))
			continue;

		if (d->params[i].type == PARAM_SET) {
			*(int *) d->params[i].ptr = 1;
			continue;
		}

		/* Should we remove PARAM_DEPRECATED because it's
		 * useless right now? -GD */
		if (d->params[i].type == PARAM_DEPRECATED) {
			void (*func) (void);
			error(linenum, "Deprecated directive `%s' used", d->name);
			func = (void (*)(void)) (d->params[i].ptr);
			func();			 /* For clarity */
			continue;
		}
		if (myoptind >= ac) {
			if (!(d->params[i].flags & PARAM_OPTIONAL)) {
				error(linenum, "Not enough parameters for `%s'", d->name);
				retval = 0;
			}
			break;
		}
		switch (d->params[i].type) {
		case PARAM_INT:
			val = strtol(av[myoptind++], &s, 0);
			if (*s) {
				error(linenum,
					  "%s: Expected an integer for parameter %d",
					  d->name, myoptind);
				retval = 0;
				break;
			}
			*(int *) d->params[i].ptr = val;
			break;
		case PARAM_POSINT:
			val = strtol(av[myoptind++], &s, 0);
			if (*s || val <= 0) {
				error(linenum,
					  "%s: Expected a positive integer for parameter %d",
					  d->name, myoptind);
				retval = 0;
				break;
			}
			if (errno == ERANGE && val == LONG_MAX) {
				/* well the true top off is 2,147,483,647 but lets not give them the real top */
				error(linenum,
					  "%s: paramter %d is to large, reduce this value (0 to 2,147,483,646)",
					  d->name, myoptind);
			}
			*(int *) d->params[i].ptr = val;
			break;
		case PARAM_PORT:
			val = strtol(av[myoptind++], &s, 0);
			if (*s) {
				error(linenum,
					  "%s: Expected a port number for parameter %d",
					  d->name, myoptind);
				retval = 0;
				break;
			}
			if (val < 1 || val > 65535) {
				error(linenum,
					  "Port numbers must be in the range 1..65535");
				retval = 0;
				break;
			}
			*(int *) d->params[i].ptr = val;
			break;
		case PARAM_STRING:
/*		  if (reload && *(char **)d->params[i].ptr)
		  	free(*(char **)d->params[i].ptr); */
			*(char **) d->params[i].ptr = sstrdup(av[myoptind++]);
			if (!d->params[i].ptr) {
				error(linenum, "%s: Out of memory", d->name);
				return 0;
			}
			break;
		case PARAM_TIME:
			val = dotime(av[myoptind++]);
			if (val < 0) {
				error(linenum,
					  "%s: Expected a time value for parameter %d",
					  d->name, myoptind);
				retval = 0;
				break;
			}
			*(int *) d->params[i].ptr = val;
			break;
		default:
			error(linenum, "%s: Unknown type %d for param %d",
				  d->name, d->params[i].type, i + 1);
			retval = 0;		 /* don't bother continuing--something's bizarre */
			break;
		}
	}
	return retval;;
}


int parse(char *buf, int linenum, int reload)
{
	char *s, *t, *dir;
	unsigned int n;
	int retval = 1;
	int ac = 0;
	char *av[MAXPARAMS];

	dir = strtok(buf, " \t\r\n");
	s = strtok(NULL, "");
	if (s) {
		while (isspace(*s))
			s++;
		while (*s) {
			if (ac >= MAXPARAMS) {
				error(linenum, "Warning: too many parameters (%d max)",
					  MAXPARAMS);
				break;
			}
			t = s;
			if (*s == '"') {
				t++;
				s++;
				while (*s && *s != '"') {
					if (*s == '\\' && s[1] != 0)
						s++;
					s++;
				}
				if (!*s)
					error(linenum,
						  "Warning: unterminated double-quoted string");
				else
					*s++ = 0;
			} else {
				s += strcspn(s, " \t\r\n");
				if (*s)
					*s++ = 0;
			}
			av[ac++] = t;
			while (isspace(*s))
				s++;
		}
	}

	if (!dir)
		return 1;

	for (n = 0; n < lenof(directives); n++) {
		Directive *d = &directives[n];
		retval = parse_directive(d, dir, ac, av, linenum, reload, s);
		if (!retval) {
			break;
		}
	}

	return retval;
}


/*************************************************************************/

#define CHECK(v) do {			\
	if (!v) {				\
	error(0, #v " missing");	\
	retval = 0;			\
	}					\
} while (0)

#define CHEK2(v,n) do {			\
	if (!v) {				\
	error(0, #n " missing");	\
	retval = 0;			\
	}					\
} while (0)

/* Read the entire configuration file.  If an error occurs while reading
 * the file or a required directive is not found, print and log an
 * appropriate error message and return 0; otherwise, return 1.
 *
 * If reload is 1, will reload the configuration file.
 *		--lara
 *
 */

int read_config(int reload)
{
	FILE *config;
	int linenum = 0, retval = 1;
	char buf[1024], *s;
	int defconCount = 0;

	if (reload) {
		unsigned int i, n;

		/* Reset all the reloadable settings */

		for (n = 0; n < lenof(directives); n++) {
			Directive *d = &directives[n];

			for (i = 0; i < MAXPARAMS && d->params[i].type != PARAM_NONE;
				 i++) {
				if (!(d->params[i].flags & PARAM_RELOAD))
					continue;

				if (d->params[i].type == PARAM_SET
					|| d->params[i].type == PARAM_INT
					|| d->params[i].type == PARAM_POSINT
					|| d->params[i].type == PARAM_TIME) {
					*(int *) d->params[i].ptr = 0;
				} else if (d->params[i].type == PARAM_STRING) {
					if (*(char **) d->params[i].ptr)
						free(*(char **) d->params[i].ptr);
					(*(char **) d->params[i].ptr) = NULL;
				}
			}
		}
	}

	retval = serverConfig.Read(reload);
	if (!retval) return 0; // Temporary until most of the below is modified to use the new parser -- CyberBotX
	config = fopen(SERVICES_CONF, "r");
	if (!config) {
		log_perror("Can't open " SERVICES_CONF);
		if (!nofork && isatty(2)) {
			if (!reload)
				perror("Can't open " SERVICES_CONF);
			else
				alog("Can't open %s", SERVICES_CONF);
		}
		return 0;
	}
	while (fgets(buf, sizeof(buf), config)) {
		linenum++;
		if (*buf == '#' || *buf == '\r' || *buf == '\n')
			continue;
		if (!parse(buf, linenum, reload))
			retval = 0;
	}
	fclose(config);

	if (!reload) {
		if (RemoteServer3)
			CHECK(RemoteServer2);

		if (LocalHost && RemoteServer) {
			if ((!stricmp(LocalHost, RemoteServer))
				&& LocalPort == RemotePort) {
				printf
					("\n*** LocalAddress and RemoteServer are set to use the same IP address\n"
					 "*** (%s) and port (%d). This would have resulted in errors.\n"
					 "*** Change the LocalAddress to bind to another port.\n",
					 RemoteServer, LocalPort);
				retval = 0;
			}
		}
	}

	CHECK(UpdateTimeout);
	CHECK(ExpireTimeout);
	CHECK(ReadTimeout);
	CHECK(WarningTimeout);
	CHECK(TimeoutCheck);

	if (temp_nsuserhost) {
		if (!(s = strchr(temp_nsuserhost, '@'))) {
			NSEnforcerUser = temp_nsuserhost;
			NSEnforcerHost = ServiceHost;
		} else {
			*s++ = 0;
			NSEnforcerUser = temp_nsuserhost;
			NSEnforcerHost = s;
		}
	}

	NSDefFlags = 0;
	if (NSDefaults.empty()) NSDefFlags = NI_SECURE | NI_MEMO_SIGNON | NI_MEMO_RECEIVE;
	else if (NSDefaults != "none") {
		bool hadAutoop = false;
		spacesepstream options(NSDefaults);
		std::string option;
		while (options.GetToken(option)) {
			if (option == "kill") NSDefFlags |= NI_KILLPROTECT;
			else if (option == "killquick") NSDefFlags |= NI_KILL_QUICK;
			else if (option == "secure") NSDefFlags |= NI_SECURE;
			else if (option == "private") NSDefFlags |= NI_PRIVATE;
			else if (option == "msg") {
				if (!UsePrivmsg) alog("msg in <nickserv:defaults> can only be used when UsePrivmsg is set");
				else NSDefFlags |= NI_MSG;
			}
			else if (option == "hideemail") NSDefFlags |= NI_HIDE_EMAIL;
			else if (option == "hideusermask") NSDefFlags |= NI_HIDE_MASK;
			else if (option == "hidequit") NSDefFlags |= NI_HIDE_QUIT;
			else if (option == "memosignon") NSDefFlags |= NI_MEMO_SIGNON;
			else if (option == "memoreceive") NSDefFlags |= NI_MEMO_RECEIVE;
			else if (option == "autoop") hadAutoop = true;
		}
		if (!hadAutoop) NSDefFlags |= NI_AUTOOP;
	}

	if (!NewsCount) {
		NewsCount = 3;
	}

	if (reload) {
		if ((NSDefLanguage = langlist[NSDefLanguage]) < 0)
			NSDefLanguage = DEF_LANGUAGE;
	}

	if (CSDefBantype < 0 || CSDefBantype > 3) {
		error(0, "Value of CSDefBantype must be between 0 and 3 included");
		retval = 0;
	}

	if (!MysqlRetries || !MysqlRetryGap) {
		MysqlRetries = 5;
		MysqlRetryGap = 1;
	} else if (((MysqlRetries * MysqlRetryGap) > 60)
			   || ((MysqlRetries * MysqlRetryGap) < 1)) {
		error(0,
			  "MysqlRetries * MysqlRetryGap must be between 1 and 60, using standard values.");
		MysqlRetries = 5;
		MysqlRetryGap = 1;
	}

	CSDefFlags = 0;
	if (CSDefaults.empty()) CSDefFlags = CI_KEEPTOPIC | CI_SECURE | CI_SECUREFOUNDER | CI_SIGNKICK;
	else if (CSDefaults != "none") {
		spacesepstream options(CSDefaults);
		std::string option;
		while (options.GetToken(option)) {
			if (option == "keeptopic") CSDefFlags |= CI_KEEPTOPIC;
			else if (option == "topiclock") CSDefFlags |= CI_TOPICLOCK;
			else if (option == "private") CSDefFlags |= CI_PRIVATE;
			else if (option == "restricted") CSDefFlags |= CI_RESTRICTED;
			else if (option == "secure") CSDefFlags |= CI_SECURE;
			else if (option == "secureops") CSDefFlags |= CI_SECUREOPS;
			else if (option == "securefounder") CSDefFlags |= CI_SECUREFOUNDER;
			else if (option == "signkick") CSDefFlags |= CI_SIGNKICK;
			else if (option == "signkicklevel") CSDefFlags |= CI_SIGNKICK_LEVEL;
			else if (option == "opnotice") CSDefFlags |= CI_OPNOTICE;
			else if (option == "xop") CSDefFlags |= CI_XOP;
			else if (option == "peace") CSDefFlags |= CI_PEACE;
		}
	}

	BSDefFlags = 0;
	if (!BSDefaults.empty()) {
		spacesepstream options(BSDefaults);
		std::string option;
		while (options.GetToken(option)) {
			if (option == "dontkickops") BSDefFlags |= BS_DONTKICKOPS;
			else if (option == "dontkickvoices") BSDefFlags |= BS_DONTKICKVOICES;
			else if (option == "greet") BSDefFlags |= BS_GREET;
			else if (option == "fantasy") BSDefFlags |= BS_FANTASY;
			else if (option == "symbiosis") BSDefFlags |= BS_SYMBIOSIS;
		}
	}

	WallOper = WallBadOS = WallOSGlobal = WallOSMode = WallOSClearmodes = WallOSKick = WallOSAkill = WallOSSGLine = WallOSSQLine = WallOSSZLine = WallOSNoOp =
		WallOSJupe = WallOSRaw = WallAkillExpire = WallSGLineExpire = WallSQLineExpire = WallSZLineExpire = WallExceptionExpire = WallGetpass = WallSetpass =
		WallForbid = WallDrop = false;
	if (!OSNotifications.empty()) {
		spacesepstream notifications(OSNotifications);
		std::string notice;
		while (notifications.GetToken(notice)) {
			if (notice == "oper") WallOper = true;
			else if (notice == "bados") WallBadOS = true;
			else if (notice == "osglobal") WallOSGlobal = true;
			else if (notice == "osmode") WallOSMode = true;
			else if (notice == "osclearmodes") WallOSClearmodes = true;
			else if (notice == "oskick") WallOSKick = true;
			else if (notice == "osakill") WallOSAkill = true;
			else if (notice == "ossgline") WallOSSGLine = true;
			else if (notice == "ossqline") WallOSSQLine = true;
			else if (notice == "osszline") WallOSSZLine = true;
			else if (notice == "osnoop") WallOSNoOp = true;
			else if (notice == "osjupe") WallOSJupe = true;
			else if (notice == "osraw") WallOSRaw = true;
			else if (notice == "akillexpire") WallAkillExpire = true;
			else if (notice == "sglineexpire") WallSGLineExpire = true;
			else if (notice == "sqlineexpire") WallSQLineExpire = true;
			else if (notice == "szlineexpire") WallSZLineExpire = true;
			else if (notice == "exceptionexpire") WallExceptionExpire = true;
			else if (notice == "getpass") WallGetpass = true;
			else if (notice == "setpass") WallSetpass = true;
			else if (notice == "forbid") WallForbid = true;
			else if (notice == "drop") WallDrop = true;
		}
	}

	/* Services Root building */

	if (ServicesRoot && !reload) {	  /* Check to prevent segmentation fault if it's missing */
		RootNumber = 0;

		s = strtok(ServicesRoot, " ");
		do {
			if (s) {
				RootNumber++;
				ServicesRoots =
					(char **)realloc(ServicesRoots, sizeof(char *) * RootNumber);
				ServicesRoots[RootNumber - 1] = sstrdup(s);
			}
		} while ((s = strtok(NULL, " ")));
	}

	if (!RootNumber) {
		error(0, "No ServicesRoot defined");
		retval = 0;
	}

	/* Ulines */

	if (UlineServers) {
		NumUlines = 0;

		s = strtok(UlineServers, " ");
		do {
			if (s) {
				NumUlines++;
				Ulines = (char **)realloc(Ulines, sizeof(char *) * NumUlines);
				Ulines[NumUlines - 1] = sstrdup(s);
			}
		} while ((s = strtok(NULL, " ")));
	}

	/* Host Setters building... :P */
	HostSetters = buildStringList(HostSetter, &HostNumber);

	/* Modules Autoload building... :P */
	ModulesAutoload = buildStringList(Modules, &ModulesNumber);
	HostServCoreModules =
		buildStringList(HostCoreModules, &HostServCoreNumber);
	MemoServCoreModules =
		buildStringList(MemoCoreModules, &MemoServCoreNumber);
	HelpServCoreModules =
		buildStringList(HelpCoreModules, &HelpServCoreNumber);

	BotServCoreModules =
		buildStringList(BotCoreModules, &BotServCoreNumber);

	OperServCoreModules =
		buildStringList(OperCoreModules, &OperServCoreNumber);

	ChanServCoreModules =
		buildStringList(ChanCoreModules, &ChanServCoreNumber);

	NickServCoreModules =
		buildStringList(NickCoreModules, &NickServCoreNumber);


	if (LimitSessions) {
		if (MaxSessionKill && !SessionAutoKillExpiry)
			SessionAutoKillExpiry = 1800;	/* 30 minutes */
	}

	if (s_BotServ) {
		if (!BSFantasyCharacter || !*BSFantasyCharacter)
			BSFantasyCharacter = sstrdup("!");
		if (*BSFantasyCharacter && (strlen(BSFantasyCharacter) > 1)) {
			printf
				("*** BSFantasyCharacter is more than 1 character long. Only the first\n"
				 "*** character ('%c') will be used. The others will be ignored.\n",
				 *BSFantasyCharacter);
		}
	}

	if (GlobalOnCycle) {
		if (!GlobalOnCycleMessage && !GlobalOnCycleUP) {
			alog("GlobalOnCycleMessage and GlobalOnCycleUP are not defined; disabling GlobalOnCycle");
			GlobalOnCycle = 0;
		}
	}

	/* Check the user keys */
	if ((UserKey1 == UserKey2) || (UserKey1 == UserKey3)
		|| (UserKey3 == UserKey2))
		alog("Every UserKey must be different. It's for YOUR safety! Remember that!");

	/**
	 * Check all DEFCON dependiencies...
	 **/
	if (DefConLevel) {
		/* Build DefCon's */
		DefCon[0] = 0;
		for (int level = 1; level < 5; ++level) {
			DefCon[level] = 0;
			std::string *levelDefinition = NULL;
			switch (level) {
				case 1:
					levelDefinition = &DefCon1;
					break;
				case 2:
					levelDefinition = &DefCon2;
					break;
				case 3:
					levelDefinition = &DefCon3;
					break;
				case 4:
					levelDefinition = &DefCon4;
			}
			spacesepstream operations(*levelDefinition);
			std::string operation;
			while (operations.GetToken(operation)) {
				if (operation == "nonewchannels") DefCon[level] |= DEFCON_NO_NEW_CHANNELS;
				else if (operation == "nonewnicks") DefCon[level] |= DEFCON_NO_NEW_NICKS;
				else if (operation == "nomlockchanges") DefCon[level] |= DEFCON_NO_MLOCK_CHANGE;
				else if (operation == "forcechanmodes") DefCon[level] |= DEFCON_FORCE_CHAN_MODES;
				else if (operation == "reducedsessions") DefCon[level] |= DEFCON_REDUCE_SESSION;
				else if (operation == "nonewclients") DefCon[level] |= DEFCON_NO_NEW_CLIENTS;
				else if (operation == "operonly") DefCon[level] |= DEFCON_OPER_ONLY;
				else if (operation == "silentoperonly") DefCon[level] |= DEFCON_SILENT_OPER_ONLY;
				else if (operation == "akillnewclients") DefCon[level] |= DEFCON_AKILL_NEW_CLIENTS;
				else if (operation == "nonewmemos") DefCon[level] |= DEFCON_NO_NEW_MEMOS;
			}
		}
		DefCon[5] = 0; /* DefCon level 5 is always normal operation */
		for (defconCount = 1; defconCount <= 5; defconCount++) {		/* Check any defcon needed settings */
			if (DefCon[defconCount] & DEFCON_REDUCE_SESSION) {
				CHECK(DefConSessionLimit);
			}
			if (DefCon[defconCount] & DEFCON_AKILL_NEW_CLIENTS) {
				CHECK(DefConAKILL);
				CHECK(DefConAkillReason);
			}
			if (DefCon[defconCount] & DEFCON_FORCE_CHAN_MODES) {
				CHECK(DefConChanModes);
			}
		}
	}

	/**
	 * If they try to enable any email registration option,
	 * make sure they have everything else they need too...
	 *
	 * rob
	 **/
	if (!NSEmailReg) {
		delete [] PreNickDBName;
		PreNickDBName = NULL;
		NSRExpire = 0;
	}

	if (!retval) {
		printf
			("\n*** Support resources: Read through the services.conf self-contained \n*** documentation. Read the documentation files found in the 'docs' \n*** folder. Visit our portal located at http://www.anope.org/. Join \n*** our support channel on /server irc.anope.org channel #anope.\n\n");
	}

	return retval;
}


/*************************************************************************/
