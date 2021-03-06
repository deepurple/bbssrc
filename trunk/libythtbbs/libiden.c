#include "bbs.h"
#include "identify.h"

#ifdef POP_CHECK

static const char *active_style_str[] = {"", "email", "phone", "idnum", NULL};

//判断信箱是否合法
int invalid_mail(char* mbox)
{
    if (strstr(mbox, "@bbs.")) return 1;
    if (strstr(mbox, ".bbs@")) return 1;
    if (!strstr(mbox, "@")) return 1;
    //if (invalidaddr(mbox)) return 0;
    if (strcmp(mbox+strlen(mbox)-5, "ac.cn")!=0) return 1;
    return 0;
}

/*
//生成随机码
void gencode(char* code)
{
    int i;
    int c1, c2;
    for (i=0; i<CODELEN; ++i) {
        c1=rand()%2;
        c2=rand()%26;
        code[i]=65+c1*32+c2;
    }
    code[CODELEN]='\0';
}
*/

char* str_to_uppercase(char *str)
{
    char *h = str;
    while (*str != '\n' && *str != 0) {
        *str = toupper(*str);
        str++;
    }
    return h;
}

char* str_to_lowercase(char *str)
{
    char *h = str;
    while (*str != '\n' && *str != 0) {
        *str = tolower(*str);
        str++;
    }
    return h;
}


const char* style_to_str(int style)
{
    switch (style) {
	case NO_ACTIVE:
		return "未验证";
		break;
        case MAIL_ACTIVE:
            return "信箱";
            break;
        case PHONE_ACTIVE:
            return "手机号码";
            break;
        case IDCARD_ACTIVE:
            return "其他证件";
            break;
        case FORCE_ACTIVE:
            return "手工激活";
            break;
        default:
            return "未知";
    }
}

/*
//发送验证信函
int send_active_mail(char* mbox, char* code, char* userid, session_t* session)
{
    int return_no;
    FILE *fout;
    char buff[STRLEN];
    char mtitle[STRLEN];
    //char acturl[STRLEN];
    const char *c=sysconf_str("BBS_WEBDOMAIN");

    if (!c) c=sysconf_str("BBSDOMAIN");
    sethomefile(buff, userid, "active_mail");
    fout = fopen(buff, "w");
    if (fout != NULL) {
        fprintf(fout, "Reply-To: bbs@%s\n", "kyxk.net");
        fprintf(fout, "From: bbs@%s\n",  "kyxk.net");
        fprintf(fout, "To: %s\n", str_to_lowercase(mbox));
        fprintf(fout, "Subject: %s实名认证确认信函.\n", BBS_FULL_NAME);
        fprintf(fout, "X-Forwarded-By: SYSOP \n");
        fprintf(fout, "X-Disclaimer: None\n");
        fprintf(fout, "\n");
        fprintf(fout,"您好，这是%s的实名制认证确认信函。如果您没有在本站注册，请不用理会本信件.\n", BBS_FULL_NAME);
        fprintf(fout,"您注册的使用者代号是：%s\n\n",userid);
        fprintf(fout,"您的实名认证验证码是：\n%s\n\n",code);
        // fprintf(fout,"您可以直接点击以下链接完成认证手续：\n");
        // fprintf(fout,"%s\n\n", acturl);
        fprintf(fout,"复制本验证码并在本站验证界面下输入，即可完成认证步骤\n\n");
        fprintf(fout,"亲爱的 %s, %s欢迎您的光临!",userid, BBS_FULL_NAME);
        fprintf(fout, ".\n");
        fclose(fout);
        sprintf(mtitle, "%s欢迎您!", BBS_FULL_NAME);
        return_no = bbs_sendmail(buff,  mtitle, str_to_lowercase(mbox), 0, 1, session);
        unlink(buff);
        return return_no;
    }

    return 0;
}

int send_active_msg(char* phone, char* code,char* userid)
{
    char command[512];
    char sqlbuf[512];
    MYSQL* s=NULL;
    MYSQL_RES *res;

    s=mysql_init(s);
    s = mysql_real_connect(s,"localhost",SQLUSER,SQLPASSWD,SQLDB,3306, NULL,0);
    sprintf(command, "%s. Welcome to KYXK BBS. Your verification code is:%s", userid, code);
    sprintf(sqlbuf,"INSERT INTO outbox(number, text) VALUES('%s', '%s'); " ,phone, command);
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    mysql_close(s);
    
	//system(command);
    return 0;
}

//不知道为什么，本地测试的时候sethomefile()定位不到用户的home目录
//如果实际上可以的话，这个封装函数可以舍弃
int setactivefile(char* genbuf, char* userid, char* filename)
{
    char fpath[STRLEN];
//    FILE* dp;

    strcpy(genbuf, BBS_HOMEPATH);
    strcat(genbuf, "/");
    sethomefile(fpath,userid,filename);
    strcat(genbuf, fpath);

    // sethomefile(genbuf,userid,filename);
    return 0;
}


//用户的家目录下写入随机码
int set_active_code(char* userid, char* code, char* value, int style)
{
    char genbuf[STRLEN];
    FILE* dp;

    setactivefile(genbuf,userid,ACTIVE_FILE);
    if ((dp=fopen(genbuf,"w"))==NULL) {
        fclose(dp);
        return 0;
    }
    fwrite(code, sizeof(char), CODELEN+1, dp);
    fwrite(value, sizeof(char), VALUELEN, dp);
    fwrite(&style, sizeof(int), 1, dp);
    fclose(dp);
    return 1;
}

//用户的家目录下写入随机码
//value和style一起读，是便于url验证方式使用
int get_active_code(char* userid, char* code, char* value, int* style)
{
    char genbuf[STRLEN];
    char fpath[STRLEN];
    FILE* dp;

    strcpy(fpath, BBS_HOMEPATH);
    strcat(fpath, "/");
    setactivefile(genbuf,userid,ACTIVE_FILE);
    strcat(fpath, genbuf);
    if (access(genbuf, 0)==0) {
        if ((dp=fopen(genbuf,"r"))==NULL) {
            fclose(dp);
            return 0;
        }
        fread(code, sizeof(char), CODELEN+1, dp);
        fread(value, sizeof(char), VALUELEN, dp);
        fread(style, sizeof(int), 1, dp);
        fclose(dp);
    } else {
        return FILE_NOT_FOUND;
    }
    return 1;
}
*/

int get_active_value(char* value, struct active_data* act_data)
{
	if (act_data->status==0) {
		return 0;
	}
	else if (act_data->status==MAIL_ACTIVE) {
		strcpy(value, act_data->email);
	}
	else if (act_data->status==PHONE_ACTIVE) {
		strcpy(value, act_data->phone);
	}
	else if (act_data->status==IDCARD_ACTIVE) {
		strcpy(value, act_data->idnum);
	}
	return 1;
}


//查询某个记录绑定了几个id
int query_record_num(char* value, int style)
{
    char sqlbuf[512];
    int count;
    MYSQL *s = NULL;
    MYSQL_RES *res;

    s = mysql_init(s);
    //mysql = mysql_real_connect(mysql,"localhost",SQLUSER,SQLPASSWD,SQLDB,0, NULL,0);
    if (!my_connect_mysql(s)) {
        return -1;
    }
    sprintf(sqlbuf,"SELECT * FROM %s WHERE lower(%s)='%s' AND status>0; " , USERREG_TABLE, active_style_str[style], str_to_lowercase(value));
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    count=mysql_num_rows(res);
    mysql_close(s);
    return count;
}

MYSQL * my_connect_mysql(MYSQL *s)
{

    return mysql_real_connect(s, "localhost", SQLUSER, SQLPASSWD, SQLDB, 3306, NULL, 0);
}


/*写入数据库
 */
int write_active(struct active_data* act_data)
{
    char sqlbuf[512];
    int count;

    MYSQL *s = NULL;
    MYSQL_RES *res;

    s = mysql_init(s);
    if (!my_connect_mysql(s)) {
        return WRITE_FAIL;
    }
/*
    sprintf(sqlbuf,"SELECT * FROM %s WHERE %s='%s' AND status>0; " , USERREG_TABLE, active_style_str[style], record);
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    //大于三个，不写
    if (mysql_num_rows(res)>=MAX_USER_PER_RECORD) {
        mysql_close(s);
        return TOO_MUCH_RECORDS;
    }
*/
    sprintf(sqlbuf,"SELECT * FROM %s WHERE userid='%s'; " , USERREG_TABLE, act_data->userid);
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    count = mysql_num_rows(res);
    if (count==0) {
        sprintf(sqlbuf, "INSERT INTO %s(userid, name, ip, regtime, updatetime, operator, email, phone, idnum, studnum, dept, status) VALUES('%s', '%s', '%s', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, '%s', '%s', '%s', '%s', '%s', '%s', %d);",
                USERREG_TABLE, act_data->userid, act_data->name, act_data->ip, act_data->operator, act_data->email, act_data->phone, act_data->idnum, act_data->stdnum, act_data->dept, act_data->status);
        mysql_real_query(s, sqlbuf, strlen(sqlbuf));
        mysql_close(s);
        return WRITE_SUCCESS;
    } else{
        sprintf(sqlbuf, "UPDATE %s SET updatetime=CURRENT_TIMESTAMP, name='%s', ip='%s', operator='%s', email='%s', phone='%s', idnum='%s', studnum='%s', status=%d, dept='%s' WHERE userid='%s';",
                USERREG_TABLE, act_data->name, act_data->ip, act_data->operator, act_data->email, act_data->phone, act_data->idnum, act_data->stdnum, act_data->status, act_data->dept, act_data->userid);
        mysql_real_query(s, sqlbuf, strlen(sqlbuf));
        mysql_close(s);
        return UPDATE_SUCCESS;
    }

    mysql_close(s);
    return WRITE_FAIL;
}

int read_active(char* userid, struct active_data* act_data)
{
    char sqlbuf[512];
    int count;
    MYSQL *s = NULL;
    MYSQL_RES *res;
    MYSQL_ROW row;

    s = mysql_init(s);
    if (!my_connect_mysql(s)) {
        return 0;
    }
    strcpy(act_data->userid, userid);

    sprintf(sqlbuf,"SELECT name, dept, ip, regtime, updatetime, operator, email, phone, idnum, studnum, status FROM %s WHERE userid='%s'; " , USERREG_TABLE, userid);
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    row=mysql_fetch_row(res);
    count=mysql_num_rows(res);	
    if (count<1) {
        mysql_close(s);
        return 0;
    }
    strcpy(act_data->name, row[0]);
    strcpy(act_data->dept, row[1]);
    strcpy(act_data->ip, row[2]);
    strcpy(act_data->regtime, row[3]);
    strcpy(act_data->uptime, row[4]);
    strcpy(act_data->operator, row[5]);
    strcpy(act_data->email, row[6]);
    strcpy(act_data->phone, row[7]);
    strcpy(act_data->idnum, row[8]);
    strcpy(act_data->stdnum, row[9]);
    act_data->status=atoi(row[10]);
    mysql_close(s);
    return count;
}

/*
int valid_stunum(char* mbox, char* stunum)
{
    char sqlbuf[512];
//    char stunum_cmp[32];
    MYSQL *s = NULL;
    int count;
    MYSQL_RES *res;
    MYSQL_ROW row;

    s = mysql_init(s);
    if (!my_connect_mysql(s)) {
        return 0;
    }
    sprintf(sqlbuf,"SELECT * FROM %s WHERE lower(email)='%s' AND studnum='%s'; " , SCHOOLDATA_TABLE, mbox, stunum);
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    row=mysql_fetch_row(res);
//    strcpy(stunum_cmp, row[0]);
    count=mysql_num_rows(res);	
    mysql_close(s);
//    return !strcmp(stunum, stunum_cmp);
    return count;
}

int get_official_data(struct active_data* act_data)
{
    char sqlbuf[512];
    MYSQL *s = NULL;
    MYSQL_RES *res;
    MYSQL_ROW row;

    s = mysql_init(s);
    if (!my_connect_mysql(s)) {
        return 0;
    }

    sprintf(sqlbuf,"SELECT name, dept  FROM %s WHERE lower(email)='%s' AND studnum='%s'; " , SCHOOLDATA_TABLE, str_to_lowercase(act_data->email), act_data->stdnum);
    mysql_real_query(s, sqlbuf, strlen(sqlbuf));
    res = mysql_store_result(s);
    row=mysql_fetch_row(res);
    strcpy(act_data->name, row[0]);
    strcpy(act_data->dept, row[1]);
    mysql_close(s);
    return 0;
}

*/


#endif

