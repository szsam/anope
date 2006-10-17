
/* MySQL functions.
 *
 * (C) 2003-2005 Anope Team
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

/*************************************************************************/

/* Database Global Variables */
MYSQL *mysql;                   /* MySQL Handler */
MYSQL_RES *mysql_res;           /* MySQL Result  */
MYSQL_FIELD *mysql_fields;      /* MySQL Fields  */
MYSQL_ROW mysql_row;            /* MySQL Row     */

int mysql_is_connected = 0;		/* Are we currently connected? */

/*************************************************************************/

void db_mysql_error(int severity, char *msg)
{
    static char buf[512];

    if (mysql_error(mysql)) {
        snprintf(buf, sizeof(buf), "MySQL %s %s: %s", msg,
                 severity == MYSQL_WARNING ? "warning" : "error",
                 mysql_error(mysql));
    } else {
        snprintf(buf, sizeof(buf), "MySQL %s %s", msg,
                 severity == MYSQL_WARNING ? "warning" : "error");
    }

    log_perror(buf);

    if (severity == MYSQL_ERROR) {
        log_perror("MySQL FATAL error... aborting.");
        exit(0);
    }

}

/*************************************************************************/

int db_mysql_init()
{

    /* If the host is not defined, assume we don't want MySQL */
    if (!MysqlHost) {
        do_mysql = 0;
        alog("MySQL: has been disabled.");
        return 0;
    } else {
        do_mysql = 1;
        alog("MySQL: has been enabled.");
        alog("MySQL: client version %s.", mysql_get_client_info());
    }

    /* The following configuration options are required.
     * If missing disable MySQL to avoid any problems.   
     */

    if ((do_mysql) && (!MysqlName || !MysqlUser)) {
        do_mysql = 0;
        alog("MySQL Error: Set all required configuration options.");
        return 0;
    }

    if (!db_mysql_open()) {
        do_mysql = 0;
        return 0;
    }

    return 1;
}

/*************************************************************************/

int db_mysql_open()
{
    /* If MySQL is disabled, return 0 */
    if (!do_mysql)
        return 0;
	
	/* If we're already connected, return 1 */
	if (mysql_is_connected)
		return 1;
	
    mysql = mysql_init(NULL);
    if (mysql == NULL)
        db_mysql_error(MYSQL_WARNING, "Unable to create mysql object");

    if (!MysqlPort)
        MysqlPort = MYSQL_DEFAULT_PORT;

    if (MysqlSock) {
        if ((!mysql_real_connect
             (mysql, MysqlHost, MysqlUser, MysqlPass, MysqlName, MysqlPort,
              MysqlSock, 0))) {
            log_perror("MySQL Error: Cant connect to MySQL: %s\n",
                       mysql_error(mysql));
            return 0;
        }
    } else {
        if ((!mysql_real_connect
             (mysql, MysqlHost, MysqlUser, MysqlPass, MysqlName, MysqlPort,
              NULL, 0))) {
            log_perror("MySQL Error: Cant connect to MySQL: %s\n",
                       mysql_error(mysql));
            return 0;
        }
    }
	
	mysql_is_connected = 1;
	
    return 1;

}

/*************************************************************************/

int db_mysql_query(char *sql)
{
    int lcv;

    if (!do_mysql) {
        /* Error is 1 */
        return 1;
    }

    if (debug) {
		alog("debug: MySQL: %s", sql);
    }

    /* Try as many times as configured in MysqlRetries */
    for (lcv = 0; lcv < MysqlRetries; lcv++) {

        if (db_mysql_open() && (!mysql_query(mysql, sql))) {
            
            /* Success is 0 */
            return 0;

        }

        /* If we get here, we could not run the query */
        log_perror("Unable to run query: %s\n", mysql_error(mysql));

        /* Wait for MysqlRetryGap seconds and try again */
        sleep(MysqlRetryGap);

    }

    /* Unable to run the query */
    db_mysql_error(MYSQL_ERROR, "query");

    /* Error is 1 */
    return 1;

}

/*************************************************************************/

char *db_mysql_quote(char *sql)
{
    int slen;
    char *quoted;


    if (!sql) {
        return sstrdup("");
    }

    slen = strlen(sql);
    quoted = malloc((1 + (slen * 2)) * sizeof(char));

    mysql_real_escape_string(mysql, quoted, sql, slen);
    return quoted;

}

/*************************************************************************/

/* I don't like using res here, maybe we can pass it as a param? */
int db_mysql_close()
{
    if (mysql_res)
        mysql_free_result(mysql_res);
    mysql_close(mysql);
	
	mysql_is_connected = 0;
	
    return 1;
}

/*************************************************************************/

/*
 * NickServ Specific Secion
 */

/*************************************************************************/
void db_mysql_save_ns_req(NickRequest * nr)
{
    char *qnick, *qpasscode, *qpassword, *qemail;
    char sqlcmd[MAX_SQL_BUF];

    qnick = db_mysql_quote(nr->nick);
    qpasscode = db_mysql_quote(nr->passcode);
    qpassword = db_mysql_quote(nr->password);
    qemail = db_mysql_quote(nr->email);

    snprintf(sqlcmd, MAX_SQL_BUF,
             "REPLACE anope_ns_request (nick,passcode,password,email,requested,active)"
             " VALUES ('%s','%s','%s','%s','%d','1')",
             qnick, qpasscode, qpassword, qemail, (int) nr->requested);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }
    free(qnick);
    free(qpasscode);
    free(qpassword);
    free(qemail);
}

char *db_mysql_secure(char *pass)
{

    char epass[BUFSIZE];

    /* Initialize the buffer. Bug #86 */
    memset(epass, '\0', BUFSIZE);

#ifdef USE_ENCRYPTION
    if (pass) {
        /* If we use the builtin encryption don't double encrypt! */
        snprintf(epass, sizeof(epass), "'%s'", pass);
    }
#else

    if (!pass) {
        snprintf(epass, sizeof(epass), "''");
    } else if ((!MysqlSecure) || (strcmp(MysqlSecure, "") == 0)) {
        snprintf(epass, sizeof(epass), "'%s'", pass);
    } else if (strcmp(MysqlSecure, "des") == 0) {
        snprintf(epass, sizeof(epass), "ENCRYPT('%s')", pass);
    } else if (strcmp(MysqlSecure, "md5") == 0) {
        snprintf(epass, sizeof(epass), "MD5('%s')", pass);
    } else if (strcmp(MysqlSecure, "sha") == 0) {
        snprintf(epass, sizeof(epass), "SHA('%s')", pass);
    } else {
        snprintf(epass, sizeof(epass), "ENCODE('%s','%s')", pass,
                 MysqlSecure);
    }

#endif

    return sstrdup(epass);

}

/*************************************************************************/
void db_mysql_save_ns_core(NickCore * nc)
{
    char sqlcmd[MAX_SQL_BUF];
    int j;
    char **access;
    Memo *memos;
    char *cnick, *cpass, *epass, *cemail, *cgreet, *curl, *caccess,
        *msender, *mtext;

    cnick = db_mysql_quote(nc->display);
    cpass = db_mysql_quote(nc->pass);
    cemail = db_mysql_quote(nc->email);
    cgreet = db_mysql_quote(nc->greet);
    curl = db_mysql_quote(nc->url);

    epass = db_mysql_secure(cpass);
    free(cpass);

    /* Let's take care of the core itself */
    /* Update the existing records */
    snprintf(sqlcmd, MAX_SQL_BUF,
             "UPDATE anope_ns_core SET pass=%s,email='%s',greet='%s',icq='%d',url='%s',flags='%d',"
             "language='%d',accesscount='%d',memocount='%d',memomax='%d',channelcount='%d'"
             ",channelmax='%d',active='1' WHERE display='%s'",
             epass, cemail, cgreet, nc->icq, curl, nc->flags,
             nc->language, nc->accesscount, nc->memos.memocount,
             nc->memos.memomax, nc->channelcount, nc->channelmax, cnick);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }

    /* need to write a wrapper for mysql_affected_rows */
    /* Our previous UPDATE affected no rows, therefore this is a new record */
    if ((int) mysql_affected_rows(mysql) <= 0) {

        /* Let's take care of the core itself */
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_ns_core (display,pass,email,greet,icq,url,flags,"
                 "language,accesscount,memocount,memomax,channelcount,channelmax,active)"
                 " VALUES ('%s',%s,'%s','%s','%d','%s','%d','%d','%d','%d','%d','%d','%d','1')",
                 cnick, epass, cemail, cgreet, nc->icq, curl, nc->flags,
                 nc->language, nc->accesscount, nc->memos.memocount,
                 nc->memos.memomax, nc->channelcount, nc->channelmax);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
    }

    /* Now let's do the access */
    for (j = 0, access = nc->access; j < nc->accesscount; j++, access++) {
        caccess = db_mysql_quote(*access);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_ns_access (display,access) VALUES ('%s','%s')",
                 cnick, caccess);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        free(caccess);
    }

    /* And... memos */
    memos = nc->memos.memos;
    for (j = 0; j < nc->memos.memocount; j++, memos++) {
        msender = db_mysql_quote(memos->sender);
        mtext = db_mysql_quote(memos->text);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_ms_info (receiver,number,flags,time,sender,text,serv)"
                 " VALUES ('%s','%d','%d','%d','%s','%s','NICK')",
                 cnick, memos->number, memos->flags,
                 (int) memos->time, msender, mtext);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        free(msender);
        free(mtext);
    }

    free(cnick);
    free(epass);
    free(cemail);
    free(cgreet);
    free(curl);
}


/*************************************************************************/
void db_mysql_save_ns_alias(NickAlias * na)
{
    char sqlcmd[MAX_SQL_BUF];
    char *nnick, *nlmask, *nlrname, *nlquit, *nncnick;
    nnick = db_mysql_quote(na->nick);
    nlmask = db_mysql_quote(na->last_usermask);
    nlrname = db_mysql_quote(na->last_realname);
    nlquit = db_mysql_quote(na->last_quit);
    nncnick = db_mysql_quote(na->nc->display);
    snprintf(sqlcmd, MAX_SQL_BUF,
             "UPDATE anope_ns_alias SET last_usermask='%s',last_realname='%s',last_quit='%s',time_registered='%d',last_seen='%d',status='%d',display='%s',active='1' WHERE nick='%s'",
             nlmask, nlrname, nlquit, (int) na->time_registered,
             (int) na->last_seen, (int) na->status, nncnick, nnick);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }
    /* Our previous UPDATE affected no rows, therefore this is a new record */
    if ((int) mysql_affected_rows(mysql) <= 0) {
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT INTO anope_ns_alias (nick,last_usermask,last_realname,last_quit,time_registered,last_seen,status,display,active) VALUES ('%s','%s','%s','%s','%d','%d','%d','%s','1')",
                 nnick, nlmask, nlrname, nlquit, (int) na->time_registered,
                 (int) na->last_seen, (int) na->status, nncnick);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
    }

    free(nnick);
    free(nlmask);
    free(nlrname);
    free(nlquit);
    free(nncnick);

    return;
}

/*************************************************************************/

/*
 * ChanServ Specific Secion
 */

/*************************************************************************/
void db_mysql_save_cs_info(ChannelInfo * ci)
{
    char sqlcmd[MAX_SQL_BUF];
    int j, position;
    Memo *memos;
    char *ciname, *cifoundernick, *cisuccessornick, *cifounderpass,
        *cidesc, *ciurl, *ciemail, *cilasttopic, *cilasttopicsetter,
        *ciforbidby, *ciforbidreason, *cimlock_key, *cimlock_flood,
        *cimlock_redirect, *cientrymsg, *cibotnick, *msender, *mtext,
        *ciaccessdisp, *ciakickdisp, *ciakickreason, *ciakickcreator,
        *cbadwords, *efounderpass;

    ciname = db_mysql_quote(ci->name);
    if (ci->founder) {
        cifoundernick = db_mysql_quote(ci->founder->display);
    } else {
        cifoundernick = db_mysql_quote("");
    }
    if (ci->successor) {
        cisuccessornick = db_mysql_quote(ci->successor->display);
    } else {
        cisuccessornick = db_mysql_quote("");
    }
    cifounderpass = db_mysql_quote(ci->founderpass);
    cidesc = db_mysql_quote(ci->desc);
    ciurl = db_mysql_quote(ci->url);
    ciemail = db_mysql_quote(ci->email);
    cilasttopic = db_mysql_quote(ci->last_topic);
    cilasttopicsetter = db_mysql_quote(ci->last_topic_setter);
    ciforbidby = db_mysql_quote(ci->forbidby);
    ciforbidreason = db_mysql_quote(ci->forbidreason);
    cimlock_key = db_mysql_quote(ci->mlock_key);
    cimlock_flood = db_mysql_quote(ci->mlock_flood);
    cimlock_redirect = db_mysql_quote(ci->mlock_redirect);
    cientrymsg = db_mysql_quote(ci->entry_message);
    if (ci->bi) {
        cibotnick = db_mysql_quote(ci->bi->nick);
    } else {
        cibotnick = db_mysql_quote("");
    }

    efounderpass = db_mysql_secure(cifounderpass);
    free(cifounderpass);

    /* Let's take care of the core itself */
    snprintf(sqlcmd, MAX_SQL_BUF,
             "UPDATE anope_cs_info SET founder='%s',successor='%s',founderpass=%s,"
             "descr='%s',url='%s',email='%s',time_registered='%d',last_used='%d',"
             "last_topic='%s',last_topic_setter='%s',last_topic_time='%d',flags='%d',"
             "forbidby='%s',forbidreason='%s',bantype='%d',accesscount='%d',"
             "akickcount='%d',mlock_on='%d',mlock_off='%d',mlock_limit='%d',"
             "mlock_key='%s',mlock_flood='%s',mlock_redirect='%s',entry_message='%s',"
             "memomax='%d',botnick='%s',botflags='%d',bwcount='%d',"
             "capsmin='%d',capspercent='%d',floodlines='%d',floodsecs='%d',"
             "repeattimes='%d',active='1' WHERE name='%s'",
             cifoundernick,
             cisuccessornick,
             efounderpass, cidesc, ciurl, ciemail,
             (int) ci->time_registered, (int) ci->last_used,
             cilasttopic, cilasttopicsetter,
             (int) ci->last_topic_time, (int) ci->flags,
             ciforbidby, ciforbidreason, (int) ci->bantype,
             (int) ci->accesscount, (int) ci->akickcount,
             (int) ci->mlock_on, (int) ci->mlock_off,
             (int) ci->mlock_limit, cimlock_key,
             cimlock_flood,
             cimlock_redirect,
             cientrymsg,
             (int) ci->memos.memomax,
             cibotnick,
             (int) ci->botflags,
             (int) ci->bwcount,
             (int) ci->capsmin,
             (int) ci->capspercent,
             (int) ci->floodlines,
             (int) ci->floodsecs, (int) ci->repeattimes, ciname);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }

    /* Our previous UPDATE affected no rows, therefore this is a new record */
    if ((int) mysql_affected_rows(mysql) <= 0) {
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_cs_info (name,founder,successor,founderpass,"
                 "descr,url,email,time_registered,last_used,last_topic,last_topic_setter"
                 ",last_topic_time,flags,forbidby,forbidreason,bantype,accesscount,akickcount"
                 ",mlock_on,mlock_off,mlock_limit,mlock_key,mlock_flood,mlock_redirect,"
                 "entry_message,botnick,botflags,bwcount,capsmin,capspercent,floodlines,"
                 "floodsecs,repeattimes,active) VALUES ('%s','%s','%s',%s,'%s','%s','%s'"
                 ",'%d','%d','%s','%s','%d','%d','%s','%s','%d','%d','%d','%d','%d','%d',"
                 "'%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%d','%d','1')",
                 ciname,
                 cifoundernick,
                 cisuccessornick,
                 efounderpass, cidesc, ciurl, ciemail,
                 (int) ci->time_registered, (int) ci->last_used,
                 cilasttopic, cilasttopicsetter,
                 (int) ci->last_topic_time, (int) ci->flags,
                 ciforbidby, ciforbidreason, (int) ci->bantype,
                 (int) ci->accesscount, (int) ci->akickcount,
                 (int) ci->mlock_on, (int) ci->mlock_off,
                 (int) ci->mlock_limit, cimlock_key,
                 cimlock_flood,
                 cimlock_redirect,
                 cientrymsg,
                 cibotnick,
                 (int) ci->botflags,
                 (int) ci->bwcount,
                 (int) ci->capsmin,
                 (int) ci->capspercent,
                 (int) ci->floodlines,
                 (int) ci->floodsecs, (int) ci->repeattimes);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
    }

    /* Memos */
    memos = ci->memos.memos;
    for (j = 0; j < ci->memos.memocount; j++, memos++) {
        msender = db_mysql_quote(memos->sender);
        mtext = db_mysql_quote(memos->text);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_ms_info (receiver,number,flags,time,sender,text,serv)"
                 " VALUES ('%s','%d','%d','%d','%s','%s','CHAN')",
                 ciname, memos->number, memos->flags,
                 (int) memos->time, msender, mtext);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        free(msender);
        free(mtext);
    }

    /* Access */
    for (j = 0; j < ci->accesscount; j++) {
        if (ci->access[j].in_use) {
            ciaccessdisp = db_mysql_quote(ci->access[j].nc->display);
            snprintf(sqlcmd, MAX_SQL_BUF,
                     "INSERT DELAYED INTO anope_cs_access (in_use,level,display,channel,last_seen)"
                     " VALUES ('%d','%d','%s','%s','%d')",
                     (int) ci->access[j].in_use, (int) ci->access[j].level,
                     ciaccessdisp, ciname, (int) ci->access[j].last_seen);
            if (db_mysql_query(sqlcmd)) {
                log_perror("Can't create sql query: %s", sqlcmd);
                db_mysql_error(MYSQL_WARNING, "query");
            }
            free(ciaccessdisp);
        }
    }

    /* Levels */
    position = 0;
    for (j = 0; j < CA_SIZE; j++) {
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_cs_levels (channel, position, level) VALUES ('%s','%d','%d')",
                 ciname, position++, (int) ci->levels[j]);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
    }

    /* Akicks */
    for (j = 0; j < ci->akickcount; j++) {
        ciakickdisp =
            ci->akick[j].flags & AK_USED ? ci->akick[j].
            flags & AK_ISNICK ? db_mysql_quote(ci->akick[j].u.nc->
                                               display) :
            db_mysql_quote(ci->akick[j].u.mask) : "";
        ciakickreason =
            ci->akick[j].flags & AK_USED ? db_mysql_quote(ci->akick[j].
                                                          reason) : "";
        ciakickcreator =
            ci->akick[j].flags & AK_USED ? db_mysql_quote(ci->akick[j].
                                                          creator) : "";
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_cs_akicks (channel, flags, dmask, reason, creator,"
                 " addtime) VALUES ('%s','%d','%s','%s','%s','%d')",
                 ciname, (int) ci->akick[j].flags, ciakickdisp,
                 ciakickreason, ciakickcreator,
                 ci->akick[j].flags & AK_USED ? (int) ci->akick[j].
                 addtime : 0);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        if (ci->akick[j].flags & AK_USED) {
            free(ciakickdisp);
            free(ciakickreason);
            free(ciakickcreator);
        }
    }

    /* Bad Words */
    for (j = 0; j < ci->bwcount; j++) {
        if (ci->badwords[j].in_use) {
            cbadwords = db_mysql_quote(ci->badwords[j].word);
            snprintf(sqlcmd, MAX_SQL_BUF,
                     "INSERT DELAYED INTO anope_cs_badwords (channel, word, type)"
                     " VALUES ('%s','%s','%d')", ciname, cbadwords,
                     (int) ci->badwords[j].type);
            free(cbadwords);
            if (db_mysql_query(sqlcmd)) {
                log_perror("Can't create sql query: %s", sqlcmd);
                db_mysql_error(MYSQL_WARNING, "query");
            }
        }
    }

    /* TTB's */
    for (j = 0; j < TTB_SIZE; j++) {
        snprintf(sqlcmd, MAX_SQL_BUF, "INSERT DELAYED INTO anope_cs_ttb (channel, ttb_id, value)"
				      " VALUES ('%s', %d, %d)", ciname, j, ci->ttb[j]);
        if (db_mysql_query(sqlcmd)) {
	        log_perror("Can't create sql query: %s", sqlcmd);
                db_mysql_error(MYSQL_WARNING, "query");
        }
    }

    free(ciname);
    free(cifoundernick);        /* mark */
    free(cisuccessornick);      /* mark */
    free(efounderpass);
    free(cidesc);
    free(ciurl);
    free(ciemail);
    free(cilasttopic);
    free(cilasttopicsetter);
    free(cimlock_key);
    free(cimlock_flood);
    free(cimlock_redirect);
    free(cientrymsg);
    free(cibotnick);            /* mark */
    free(ciforbidby);
    free(ciforbidreason);

    return;
}

/*************************************************************************/


/*
 * OperServ Specific Section
 */

/*************************************************************************/
void db_mysql_save_os_db(unsigned int maxucnt, unsigned int maxutime,
                         SList * ak, SList * sgl, SList * sql, SList * szl)
{
    char sqlcmd[MAX_SQL_BUF];
    Akill *t_ak;
    SXLine *t_sl;
    char *takuser, *takhost, *takby, *takreason, *tslmask, *tslby,
        *tslreason;

    int i, j;

    rdb_clear_table("anope_os_core");

    snprintf(sqlcmd, MAX_SQL_BUF,
             "INSERT DELAYED INTO anope_os_core (maxusercnt,maxusertime,akills_count,"
             "sglines_count,sqlines_count,szlines_count) VALUES "
             "('%d','%d','%d','%d','%d','%d')", maxucnt, maxutime,
             ak->count, sgl->count, sql->count, szl->count);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }

    /* now the akills saving */
    rdb_clear_table("anope_os_akills");

    j = ak->count;
    for (i = 0; i < j; i++) {
        t_ak = ak->list[i];
        takuser = db_mysql_quote(t_ak->user);
        takhost = db_mysql_quote(t_ak->host);
        takby = db_mysql_quote(t_ak->by);
        takreason = db_mysql_quote(t_ak->reason);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_os_akills (user,host,xby,reason,seton,expire) VALUES ('%s','%s','%s','%s','%d','%d')",
                 takuser,
                 takhost,
                 takby, takreason, (int) t_ak->seton, (int) t_ak->expires);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        free(takuser);
        free(takhost);
        free(takby);
        free(takreason);
    }

/* sglines save */
    rdb_clear_table("anope_os_sglines");

    j = sgl->count;
    for (i = 0; i < j; i++) {
        t_sl = sgl->list[i];
        tslmask = db_mysql_quote(t_sl->mask);
        tslby = db_mysql_quote(t_sl->by);
        tslreason = db_mysql_quote(t_sl->reason);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_os_sglines (mask,xby,reason,seton,expire) VALUES"
                 " ('%s','%s','%s','%d','%d')",
                 tslmask,
                 tslby, tslreason, (int) t_sl->seton, (int) t_sl->expires);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        free(tslmask);
        free(tslby);
        free(tslreason);
    }

/* sqlines save */
    rdb_clear_table("anope_os_sqlines");

    j = sql->count;
    for (i = 0; i < j; i++) {
        t_sl = sql->list[i];
        tslmask = db_mysql_quote(t_sl->mask);
        tslby = db_mysql_quote(t_sl->by);
        tslreason = db_mysql_quote(t_sl->reason);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_os_sqlines (mask,xby,reason,seton,expire) VALUES ('%s','%s','%s','%d','%d')",
                 tslmask,
                 tslby, tslreason, (int) t_sl->seton, (int) t_sl->expires);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        free(tslmask);
        free(tslby);
        free(tslreason);
    }

/* szlines save */
    rdb_clear_table("anope_os_szlines");

    j = szl->count;
    for (i = 0; i < j; i++) {
        t_sl = szl->list[i];
        tslmask = db_mysql_quote(t_sl->mask);
        tslby = db_mysql_quote(t_sl->by);
        tslreason = db_mysql_quote(t_sl->reason);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "INSERT DELAYED INTO anope_os_szlines (mask,xby,reason,seton,expire) VALUES"
                 " ('%s','%s','%s','%d','%d')",
                 tslmask,
                 tslby, tslreason, (int) t_sl->seton, (int) t_sl->expires);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        free(tslmask);
        free(tslby);
        free(tslreason);
    }

    return;
}

/*************************************************************************/
void db_mysql_save_news(NewsItem * ni)
{
    char sqlcmd[MAX_SQL_BUF];
    char *nitext, *niwho;
    nitext = db_mysql_quote(ni->text);
    niwho = db_mysql_quote(ni->who);
    snprintf(sqlcmd, MAX_SQL_BUF,
             "INSERT DELAYED INTO anope_os_news (type,num,ntext,who,`time`)"
             " VALUES ('%d','%d','%s','%s','%d')",
             ni->type, ni->num, nitext, niwho, (int) ni->time);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }
    free(nitext);
    free(niwho);

    return;
}

/*************************************************************************/
void db_mysql_save_exceptions(Exception * e)
{
    char sqlcmd[MAX_SQL_BUF];
    char *emask, *ewho, *ereason;
    emask = db_mysql_quote(e->mask);
    ewho = db_mysql_quote(e->who);
    ereason = db_mysql_quote(e->reason);
    snprintf(sqlcmd, MAX_SQL_BUF,
             "INSERT DELAYED INTO anope_os_exceptions (mask,lim,who,reason,`time`,expires)"
             " VALUES ('%s','%d','%s','%s','%d','%d')",
             emask, e->limit, ewho,
             ereason, (int) e->time, (int) e->expires);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }
    free(emask);
    free(ewho);
    free(ereason);
    return;
}

/*************************************************************************/


/*
 * HostServ Specific Section
 */

/*************************************************************************/
/* TODO: Add vident to tables! */
void db_mysql_save_hs_core(HostCore * hc)
{
    char sqlcmd[MAX_SQL_BUF];
    char *hcnick, *hcvident, *hcvhost, *hccreator;
    hcnick = db_mysql_quote(hc->nick);
    hcvident = db_mysql_quote(hc->vIdent);
    hcvhost = db_mysql_quote(hc->vHost);
    hccreator = db_mysql_quote(hc->creator);
    snprintf(sqlcmd, MAX_SQL_BUF,
             "INSERT DELAYED INTO anope_hs_core (nick,vident,vhost,creator,`time`)"
             " VALUES ('%s','%s','%s','%s','%d')",
             hcnick, hcvident, hcvhost, hccreator, (int) hc->time);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }
    free(hcnick);
    free(hcvident);
    free(hcvhost);
    free(hccreator);

    return;
}

/*************************************************************************/

/*
 * HostServ Specific Section
 */

/*************************************************************************/
void db_mysql_save_bs_core(BotInfo * bi)
{
    char sqlcmd[MAX_SQL_BUF];
    char *binick, *biuser, *bihost, *bireal;
    binick = db_mysql_quote(bi->nick);
    biuser = db_mysql_quote(bi->user);
    bihost = db_mysql_quote(bi->host);
    bireal = db_mysql_quote(bi->real);
    snprintf(sqlcmd, MAX_SQL_BUF,
             "INSERT DELAYED INTO anope_bs_core (nick,user,host,rname,flags,created"
             ",chancount) VALUES ('%s','%s','%s','%s','%d','%d','%d')",
             binick, biuser,
             bihost, bireal, bi->flags, (int) bi->created, bi->chancount);
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }
    free(binick);
    free(biuser);
    free(bihost);
    free(bireal);
}

/*************************************************************************/

void db_mysql_load_bs_dbase(void)
{
    BotInfo *bi;
    char sqlcmd[MAX_SQL_BUF];

    if (!do_mysql)
        return;

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `nick`,`user`,`host`,`rname`,`flags`,`created`,`chancount` FROM `anope_bs_core`");

    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    if (mysql_num_rows(mysql_res) == 0) {
        mysql_free_result(mysql_res);
        return;
    }
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        bi = makebot(mysql_row[0]);
        bi->user = sstrdup(mysql_row[1]);
        bi->host = sstrdup(mysql_row[2]);
        bi->real = sstrdup(mysql_row[3]);
        bi->flags = atoi(mysql_row[4]);
        bi->created = atoi(mysql_row[5]);
        bi->chancount = atoi(mysql_row[6]);
    }
    mysql_free_result(mysql_res);
}

void db_mysql_load_hs_dbase(void)
{
    char sqlcmd[MAX_SQL_BUF];
    char *nick;
    char *vHost;
    char *creator;
    char *vIdent;
    int32 time;

    if (!do_mysql)
        return;

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `nick`,`vident`,`vhost`,`creator`,`time` FROM `anope_hs_core`");

    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    if (mysql_num_rows(mysql_res) == 0) {
        mysql_free_result(mysql_res);
        return;
    }
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        nick = sstrdup(mysql_row[0]);
        vIdent = sstrdup(mysql_row[1]);
        vHost = sstrdup(mysql_row[2]);
        creator = sstrdup(mysql_row[3]);
        time = atoi(mysql_row[4]);
        addHostCore(nick, vIdent, vHost, creator, time);
        free(nick);
        free(vHost);
        free(creator);
        free(vIdent);
    }
    mysql_free_result(mysql_res);
}

void db_mysql_load_news(void)
{
    char sqlcmd[MAX_SQL_BUF];
    int j;

    if (!do_mysql)
        return;

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `type`,`num`,`ntext`,`who`,`time` FROM `anope_os_news`");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    nnews = mysql_num_rows(mysql_res);
    if (nnews < 8)
        news_size = 16;
    else if (nnews >= 16384)
        news_size = 32767;
    else
        news_size = 2 * nnews;
    news = scalloc(sizeof(*news) * news_size, 1);
    if (!nnews) {
        mysql_free_result(mysql_res);
        return;
    }
    j = 0;
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        news[j].type = atoi(mysql_row[0]);
        news[j].num = atoi(mysql_row[1]);
        news[j].text = sstrdup(mysql_row[2]);
        snprintf(news[j].who, NICKMAX, "%s", mysql_row[3]);
        news[j].time = atoi(mysql_row[4]);
        j++;
    }
    mysql_free_result(mysql_res);
}

void db_mysql_load_exceptions(void)
{
    char sqlcmd[MAX_SQL_BUF];
    int j;

    if (!do_mysql)
        return;

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `mask`,`lim`,`who`,`reason`,`time`,`expires` FROM `anope_os_exceptions`;");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    nexceptions = mysql_num_rows(mysql_res);
    exceptions = scalloc(sizeof(Exception) * nexceptions, 1);
    j = 0;
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        exceptions[j].mask = sstrdup(mysql_row[0]);
        exceptions[j].limit = atoi(mysql_row[1]);
        snprintf(exceptions[j].who, NICKMAX, "%s", mysql_row[2]);
        exceptions[j].reason = sstrdup(mysql_row[3]);
        exceptions[j].time = atoi(mysql_row[4]);
        exceptions[j].expires = atoi(mysql_row[5]);
        j++;
    }
    mysql_free_result(mysql_res);
}

#define HASH(host) ((tolower((host)[0])&31)<<5 | (tolower((host)[1])&31))

void db_mysql_load_os_dbase(void)
{
    char sqlcmd[MAX_SQL_BUF];
    Akill *ak;
    SXLine *sx;
    int akc, sgc, sqc, szc;

    if (!do_mysql)
        return;

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `maxusercnt`,`maxusertime`,`akills_count`,`sglines_count`,`sqlines_count`,`szlines_count` FROM `anope_os_core`;");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    if ((mysql_row = mysql_fetch_row(mysql_res))) {
        maxusercnt = atoi(mysql_row[0]);
        maxusertime = atoi(mysql_row[1]);
        akc = atoi(mysql_row[2]);
        sgc = atoi(mysql_row[3]);
        sqc = atoi(mysql_row[4]);
        szc = atoi(mysql_row[5]);
    } else {
        maxusercnt = 0;
        maxusertime = time(NULL);
        akc = sgc = sqc = szc = 0;
    }
    mysql_free_result(mysql_res);

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `user`,`host`,`xby`,`reason`,`seton`,`expire` FROM `anope_os_akills`;");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    slist_setcapacity(&akills, akc);
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        ak = scalloc(sizeof(Akill), 1);
        ak->user = sstrdup(mysql_row[0]);
        ak->host = sstrdup(mysql_row[1]);
        ak->by = sstrdup(mysql_row[2]);
        ak->reason = sstrdup(mysql_row[3]);
        ak->seton = atoi(mysql_row[4]);
        ak->expires = atoi(mysql_row[5]);
        slist_add(&akills, ak);
    }
    mysql_free_result(mysql_res);

    slist_setcapacity(&sglines, sgc);
    slist_setcapacity(&sqlines, sqc);
    slist_setcapacity(&szlines, szc);

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `mask`,`xby`,`reason`,`seton`,`expire` FROM `anope_os_sglines`;");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql statement: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        sx = scalloc(sizeof(SXLine), 1);
        sx->mask = sstrdup(mysql_row[0]);
        sx->by = sstrdup(mysql_row[1]);
        sx->reason = sstrdup(mysql_row[2]);
        sx->seton = atoi(mysql_row[3]);
        sx->expires = atoi(mysql_row[4]);
        slist_add(&sglines, sx);
    }
    mysql_free_result(mysql_res);

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `mask`,`xby`,`reason`,`seton`,`expire` FROM `anope_os_sqlines`;");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql statement: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        sx = scalloc(sizeof(SXLine), 1);
        sx->mask = sstrdup(mysql_row[0]);
        sx->by = sstrdup(mysql_row[1]);
        sx->reason = sstrdup(mysql_row[2]);
        sx->seton = atoi(mysql_row[3]);
        sx->expires = atoi(mysql_row[4]);
        slist_add(&sqlines, sx);
    }
    mysql_free_result(mysql_res);

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `mask`,`xby`,`reason`,`seton`,`expire` FROM `anope_os_szlines`;");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql statement: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        sx = scalloc(sizeof(SXLine), 1);
        sx->mask = sstrdup(mysql_row[0]);
        sx->by = sstrdup(mysql_row[1]);
        sx->reason = sstrdup(mysql_row[2]);
        sx->seton = atoi(mysql_row[3]);
        sx->expires = atoi(mysql_row[4]);
        slist_add(&szlines, sx);
    }
    mysql_free_result(mysql_res);
}

#undef HASH

void db_mysql_load_cs_dbase(void)
{
    char sqlcmd[MAX_SQL_BUF], *tempstr;
    ChannelInfo *ci;
    int n_levels, j;
    MYSQL_RES *res;
    MYSQL_ROW row;

    if (!do_mysql)
        return;

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `name`,`founder`,`successor`,`founderpass`,`descr`,`url`,`email`,`time_registered`,`last_used`,`last_topic`,`last_topic_setter`,`last_topic_time`,`flags`,`forbidby`,`forbidreason`,`bantype`,`accesscount`,`akickcount`,`mlock_on`,`mlock_off`,`mlock_limit`,`mlock_key`,`mlock_flood`,`mlock_redirect`,`entry_message`,`memomax`,`botnick`,`botflags`,`bwcount`,`capsmin`,`capspercent`,`floodlines`,`floodsecs`,`repeattimes` FROM `anope_cs_info`");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    if (mysql_num_rows(mysql_res) == 0) {
        mysql_free_result(mysql_res);
        return;
    }
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        ci = scalloc(sizeof(ChannelInfo), 1);
        snprintf(ci->name, CHANMAX, "%s", mysql_row[0]);
        ci->founder = findcore(mysql_row[1]);
        ci->successor = findcore(mysql_row[2]);
        snprintf(ci->founderpass, PASSMAX, "%s", mysql_row[3]);
        ci->desc = sstrdup(mysql_row[4]);
        ci->url = sstrdup(mysql_row[5]);
        if (strlen(ci->url) == 0) {
            free(ci->url);
            ci->url = NULL;
        }
        ci->email = sstrdup(mysql_row[6]);
        if (strlen(ci->email) == 0) {
            free(ci->email);
            ci->email = NULL;
        }
        ci->time_registered = atoi(mysql_row[7]);
        ci->last_used = atoi(mysql_row[8]);
        ci->last_topic = sstrdup(mysql_row[9]);
        snprintf(ci->last_topic_setter, NICKMAX, "%s", mysql_row[10]);
        ci->last_topic_time = atoi(mysql_row[11]);
        ci->flags = atoi(mysql_row[12]);
#ifdef USE_ENCRYPTION
        if (!(ci->flags & (CI_ENCRYPTEDPW | CI_VERBOTEN))) {
            if (debug)
                alog("debug: %s: encrypting password for %s on load",
                     s_ChanServ, ci->name);
            if (encrypt_in_place(ci->founderpass, PASSMAX) < 0)
                fatal("%s: load database: Can't encrypt %s password!",
                      s_ChanServ, ci->name);
            ci->flags |= CI_ENCRYPTEDPW;
        }
#else
        if (ci->flags & CI_ENCRYPTEDPW) {
            fatal
                ("%s: load database: password for %s encrypted but encryption disabled, aborting",
                 s_ChanServ, ci->name);
        }
#endif
        ci->flags &= ~CI_INHABIT;

        ci->forbidby = sstrdup(mysql_row[13]);
        ci->forbidreason = sstrdup(mysql_row[14]);
        ci->bantype = atoi(mysql_row[15]);

        tempstr = db_mysql_quote(ci->name);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "SELECT `position`,`level` FROM `anope_cs_levels` WHERE `channel` = '%s'",
                 tempstr);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
            return;
        }
        res = mysql_store_result(mysql);
        n_levels = mysql_num_rows(res);
        ci->levels = scalloc(2 * CA_SIZE, 1);
        reset_levels(ci);
        while ((row = mysql_fetch_row(res))) {
            ci->levels[atoi(row[0])] = atoi(row[1]);
        }
        mysql_free_result(res);
        ci->accesscount = atoi(mysql_row[16]);
        if (ci->accesscount) {
            ci->access = scalloc(ci->accesscount, sizeof(ChanAccess));
            snprintf(sqlcmd, MAX_SQL_BUF,
                     "SELECT `in_use`,`level`,`display`,`last_seen` FROM `anope_cs_access` WHERE `channel` = '%s'",
                     tempstr);
            if (db_mysql_query(sqlcmd)) {
                log_perror("Can't create sql query: %s", sqlcmd);
                db_mysql_error(MYSQL_WARNING, "query");
                return;
            }
            res = mysql_store_result(mysql);
            j = 0;
            while ((row = mysql_fetch_row(res))) {
                ci->access[j].in_use = atoi(row[0]);
                if (ci->access[j].in_use) {
                    ci->access[j].level = atoi(row[1]);
                    ci->access[j].nc = findcore(row[2]);
                    if (ci->access[j].nc == NULL)
                        ci->access[j].in_use = 0;
                    ci->access[j].last_seen = atoi(row[3]);
                }
                j++;
            }
            mysql_free_result(res);
        } else {
            ci->access = NULL;
        }
        ci->akickcount = atoi(mysql_row[17]);
        if (ci->akickcount) {
            ci->akick = scalloc(ci->akickcount, sizeof(AutoKick));
            snprintf(sqlcmd, MAX_SQL_BUF,
                     "SELECT `flags`,`dmask`,`reason`,`creator`,`addtime` FROM `anope_cs_akicks` WHERE `channel` = '%s'",
                     tempstr);
            if (db_mysql_query(sqlcmd)) {
                log_perror("Can't create sql query: %s", sqlcmd);
                db_mysql_error(MYSQL_WARNING, "query");
                return;
            }
            res = mysql_store_result(mysql);
            j = 0;
            while ((row = mysql_fetch_row(res))) {
                ci->akick[j].flags = atoi(row[0]);
                if (ci->akick[j].flags & AK_USED) {
                    if (ci->akick[j].flags & AK_ISNICK) {
                        ci->akick[j].u.nc = findcore(row[1]);
                        if (!ci->akick[j].u.nc)
                            ci->akick[j].flags &= ~AK_USED;
                    } else {
                        ci->akick[j].u.mask = sstrdup(row[1]);
                    }
                    ci->akick[j].reason = sstrdup(row[2]);
                    ci->akick[j].creator = sstrdup(row[3]);
                    ci->akick[j].addtime = atoi(row[4]);
                }
                j++;
            }
            mysql_free_result(res);
        } else {
            ci->akick = NULL;
        }
        ci->mlock_on = atoi(mysql_row[18]);
        ci->mlock_off = atoi(mysql_row[19]);
        ci->mlock_limit = atoi(mysql_row[20]);
        ci->mlock_key = sstrdup(mysql_row[21]);
        ci->mlock_flood = sstrdup(mysql_row[22]);
        ci->mlock_redirect = sstrdup(mysql_row[23]);
        ci->memos.memomax = atoi(mysql_row[25]);
        snprintf(sqlcmd, MAX_SQL_BUF,
                 "SELECT `number`,`flags`,`time`,`sender`,`text` FROM `anope_ms_info` WHERE `receiver` = '%s'",
                 tempstr);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        res = mysql_store_result(mysql);
        ci->memos.memocount = mysql_num_rows(res);
        if (ci->memos.memocount) {
            Memo *memos;
            memos = scalloc(sizeof(Memo) * ci->memos.memocount, 1);
            ci->memos.memos = memos;
            while ((row = mysql_fetch_row(res))) {
                memos->number = atoi(row[0]);
                memos->flags = atoi(row[1]);
                memos->time = atoi(row[2]);
                snprintf(memos->sender, NICKMAX, "%s", row[3]);
                memos->text = sstrdup(row[4]);
                memos->moduleData = NULL;
                memos++;
            }
        }
        mysql_free_result(res);
        ci->entry_message = sstrdup(mysql_row[24]);
        if (strlen(ci->entry_message) == 0) {
            free(ci->entry_message);
            ci->entry_message = NULL;
        }
        ci->c = NULL;

        ci->bi = findbot(mysql_row[26]);
        ci->botflags = atoi(mysql_row[27]);
        ci->capsmin = atoi(mysql_row[30]);
        ci->capspercent = atoi(mysql_row[31]);
        ci->floodlines = atoi(mysql_row[32]);
        ci->floodsecs = atoi(mysql_row[33]);
        ci->repeattimes = atoi(mysql_row[34]);

        ci->ttb = scalloc(2 * TTB_SIZE, 1);
        snprintf(sqlcmd, MAX_SQL_BUF, "SELECT `ttb_id`, `value` FROM `anope_cs_ttb` WHERE `channel` = '%s'", tempstr);
        if (db_mysql_query(sqlcmd)) {
            log_perror("Can't create sql query: %s", sqlcmd);
            db_mysql_error(MYSQL_WARNING, "query");
        }
        res = mysql_store_result(mysql);
        while ((row = mysql_fetch_row(res))) {
	    j = atoi(row[0]);
	    ci->ttb[j] = atoi(row[1]);
        }
        for (j = 0; j < TTB_SIZE; j++) {
	    if (!ci->ttb[j])
	        ci->ttb[j] = 0;
        }

        ci->bwcount = atoi(mysql_row[29]);
        if (ci->bwcount) {
            ci->badwords = scalloc(ci->bwcount, sizeof(BadWord));
            snprintf(sqlcmd, MAX_SQL_BUF,
                     "SELECT `word`,`type` FROM `anope_cs_badwords` WHERE `channel` = '%s'",
                     tempstr);
            if (db_mysql_query(sqlcmd)) {
                log_perror("Can't create sql query: %s", sqlcmd);
                db_mysql_error(MYSQL_WARNING, "query");
            }
            res = mysql_store_result(mysql);
            j = 0;
            while ((row = mysql_fetch_row(res))) {
                ci->badwords[j].in_use = 1;
                if (ci->badwords[j].in_use) {   /* I know... but for later */
                    ci->badwords[j].word = sstrdup(row[0]);
                    ci->badwords[j].type = atoi(row[1]);
                }
                j++;
            }
            mysql_free_result(res);
        } else {
            ci->badwords = NULL;
        }
        alpha_insert_chan(ci);
        free(tempstr);
    }
    mysql_free_result(mysql_res);

    for (j = 0; j < 256; j++) {
        ChannelInfo *next;
        for (ci = chanlists[j]; ci; ci = next) {
            next = ci->next;
            if (!(ci->flags & CI_VERBOTEN) && !ci->founder) {
                alog("%s: database load: Deleting founderless channel %s",
                     s_ChanServ, ci->name);
                delchan(ci);
            }
        }
    }
}

void db_mysql_load_ns_req_dbase(void)
{
    char sqlcmd[MAX_SQL_BUF];
    NickRequest *nr;

    if (!do_mysql)
        return;

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `nick`,`passcode`,`password`,`email`,`requested`,`active` FROM `anope_ns_request`;");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    if (mysql_num_rows(mysql_res) == 0) {
        mysql_free_result(mysql_res);
        return;
    }
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        nr = scalloc(1, sizeof(NickRequest));
        nr->nick = sstrdup(mysql_row[0]);
        nr->passcode = sstrdup(mysql_row[1]);
        nr->password = sstrdup(mysql_row[2]);
        nr->email = sstrdup(mysql_row[3]);
        nr->requested = atoi(mysql_row[4]);
        insert_requestnick(nr);
    }
    mysql_free_result(mysql_res);
}

void db_mysql_load_ns_dbase(void)
{
    char sqlcmd[MAX_SQL_BUF], *tmpstr;
    NickCore *nc;
    NickAlias *na;
    MYSQL_RES *res;
    MYSQL_ROW row;
    int i, j;

    if (!do_mysql)
        return;

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `display`,`pass`,`email`,`icq`,`url`,`flags`,`language`,`accesscount`,`memocount`,`memomax`,`channelcount`,`channelmax`,`greet`,`active` FROM `anope_ns_core`");

    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
        return;
    }
    mysql_res = mysql_store_result(mysql);
    if (mysql_num_rows(mysql_res) == 0) {
        mysql_free_result(mysql_res);
        return;
    }

    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        nc = scalloc(1, sizeof(NickCore));

        nc->display = sstrdup(mysql_row[0]);
        nc->pass = sstrdup(mysql_row[1]);
        nc->email = sstrdup(mysql_row[2]);
        nc->icq = atoi(mysql_row[3]);
        nc->url = sstrdup(mysql_row[4]);
        nc->flags = atoi(mysql_row[5]);
        nc->language = atoi(mysql_row[6]);
        nc->accesscount = atoi(mysql_row[7]);
        nc->memos.memocount = atoi(mysql_row[8]);
        nc->memos.memomax = atoi(mysql_row[9]);
        nc->channelcount = atoi(mysql_row[10]);
        nc->channelmax = atoi(mysql_row[11]);

        if (mysql_row[12][0] == '\0')   /* check if it's empty */
            nc->greet = NULL;
        else
            nc->greet = sstrdup(mysql_row[12]);

        if (!NSAllowKillImmed)
            nc->flags &= ~NI_KILL_IMMED;

#ifdef USE_ENCRYPTION
        if (nc->pass && !(nc->flags & NI_ENCRYPTEDPW)) {
            if (debug)
                alog("debug: %s: encrypting password for `%s' on load",
                     s_NickServ, nc->display);
            if (encrypt_in_place(nc->pass, PASSMAX) < 0)
                fatal("%s: Can't encrypt `%s' nickname password!",
                      s_NickServ, nc->display);

            nc->flags |= NI_ENCRYPTEDPW;
        }
#else
        if (nc->flags & NI_ENCRYPTEDPW)
            fatal
                ("%s: load database: password for %s encrypted but encryption disabled, aborting",
                 s_NickServ, nc->display);
#endif

        if (nc->flags & NI_SERVICES_ADMIN)
            slist_add(&servadmins, nc);
        if (nc->flags & NI_SERVICES_OPER)
            slist_add(&servopers, nc);

        if (nc->accesscount) {
            char **access;
            access = scalloc(sizeof(char *) * nc->accesscount, 1);
            nc->access = access;
            tmpstr = db_mysql_quote(nc->display);
            snprintf(sqlcmd, MAX_SQL_BUF,
                     "SELECT `access` FROM `anope_ns_access` WHERE `display` = '%s'",
                     tmpstr);
            free(tmpstr);
            if (db_mysql_query(sqlcmd)) {
                log_perror("Can't create sql query: %s", sqlcmd);
                db_mysql_error(MYSQL_WARNING, "query");
            }
            res = mysql_store_result(mysql);
            while ((row = mysql_fetch_row(res))) {
                if (strlen(row[0]) > 0) {
                    *access = sstrdup(row[0]);
                    access++;
                }
            }
            mysql_free_result(res);
        }

        if (nc->memos.memocount) {
            Memo *memos;
            memos = scalloc(sizeof(Memo) * nc->memos.memocount, 1);
            nc->memos.memos = memos;
            tmpstr = db_mysql_quote(nc->display);
            snprintf(sqlcmd, MAX_SQL_BUF,
                     "SELECT `number`,`flags`,`time`,`sender`,`text` FROM `anope_ms_info` WHERE `receiver` = '%s' ORDER BY `number` ASC",
                     tmpstr);
            free(tmpstr);
            if (db_mysql_query(sqlcmd)) {
                log_perror("Can't create sql query: %s", sqlcmd);
                db_mysql_error(MYSQL_WARNING, "query");
            }
            res = mysql_store_result(mysql);
            while ((row = mysql_fetch_row(res))) {
                memos->number = atoi(row[0]);
                memos->flags = atoi(row[1]);
                memos->time = atoi(row[2]);
                snprintf(memos->sender, NICKMAX, "%s", row[3]);
                memos->text = sstrdup(row[4]);
                memos->moduleData = NULL;
                memos++;
            }
            mysql_free_result(res);
        }
        insert_core(nc);
    }
    mysql_free_result(mysql_res);

    snprintf(sqlcmd, MAX_SQL_BUF,
             "SELECT `display`,`nick`,`time_registered`,`last_seen`,`status`,`last_usermask`,`last_realname`,`last_quit` FROM `anope_ns_alias`");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }
    mysql_res = mysql_store_result(mysql);
    while ((mysql_row = mysql_fetch_row(mysql_res))) {
        na = scalloc(1, sizeof(NickAlias));
        na->nick = sstrdup(mysql_row[1]);

        na->last_usermask = sstrdup(mysql_row[5]);
        na->last_realname = sstrdup(mysql_row[6]);
        na->last_quit = sstrdup(mysql_row[7]);
        na->time_registered = atoi(mysql_row[2]);
        na->last_seen = atoi(mysql_row[3]);
        na->status = atoi(mysql_row[4]);
        na->status &= ~NS_TEMPORARY;
        tmpstr = sstrdup(mysql_row[0]);
        na->nc = findcore(tmpstr);
        free(tmpstr);

        if (na->nc)
            slist_add(&na->nc->aliases, na);

        if (!(na->status & NS_VERBOTEN)) {
            if (!na->last_usermask)
                na->last_usermask = sstrdup("");
            if (!na->last_realname)
                na->last_realname = sstrdup("");
        }

        if (na->nc)
            na->nc->flags &= ~NI_SERVICES_ROOT;
        alpha_insert_alias(na);
    }
    mysql_free_result(mysql_res);

    for (j = 0; j < 1024; j++) {
        NickAlias *next;
        for (na = nalists[j]; na; na = next) {
            next = na->next;
            if (!na->nc) {
                alog("%s: while loading database: %s has no core! We delete it.", s_NickServ, na->nick);
                delnick(na);
                continue;
            }
            for (i = 0; i < RootNumber; i++) {
                if (!stricmp(ServicesRoots[i], na->nick))
                    na->nc->flags |= NI_SERVICES_ROOT;
            }
        }
    }
}

/* get random mysql number for the generator */
unsigned int mysql_rand(void)
{
    char sqlcmd[MAX_SQL_BUF];
    unsigned int num = 0;
    if (!do_mysql)
        return 0;
    snprintf(sqlcmd, MAX_SQL_BUF, "SELECT RAND()");
    if (db_mysql_query(sqlcmd)) {
        log_perror("Can't create sql query: %s", sqlcmd);
        db_mysql_error(MYSQL_WARNING, "query");
    }
    mysql_res = mysql_store_result(mysql);
    if (mysql_num_rows(mysql_res) == 0) {
        mysql_free_result(mysql_res);
        return 0;
    }
    mysql_row = mysql_fetch_row(mysql_res);
    num = (atoi(mysql_row[0]) * UserKey3);
    mysql_free_result(mysql_res);
    return num;
}
