#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include"debug_log.h"
#include"iniparser.h"
#include"dictionary.h"
#include"func_iniparse.h"

int getini_int_value(const char * ininame,const char *section,const char *key_section)
{
	int n = 0;	
	char key[128] = {0};
	
	dictionary *ini;

	ini = iniparser_load(ininame);//parser the file
	if(ini == NULL){
		DEBUG_LOG(DEBUGLEVEL1,"can not open %s\n",ininame);
		return LOAD_INIFILE_ERROR;
	}	
	
	snprintf(key,sizeof(key),"%s:%s",section,key_section);
	DEBUG_LOG(DEBUGLEVEL3,"key:[%s]\n",key);
	
	n = iniparser_getint(ini,key,-1);

	iniparser_freedict(ini);//free dirctionary obj

	return n;
}

char *getini_string_value(const char * ininame,const char *section,const char *key_section)
{
	char *str;
	char key[128] = {0};
	static char str_value[128] = {0};

	dictionary *ini;

	ini = iniparser_load(ininame);//parser the file
	if(ini == NULL){
		DEBUG_LOG(DEBUGLEVEL1,"can not open %s\n",ininame);
		return NULL;
	}	
	
	snprintf(key,sizeof(key),"%s:%s",section,key_section);
	DEBUG_LOG(DEBUGLEVEL3,"key:[%s]\n",key);
	
	str = iniparser_getstring(ini,key,NULL);

	if(str){
		strncpy(str_value,str,sizeof(str_value));
	}
	else{
		DEBUG_LOG(DEBUGLEVEL2,"iniparser_getstring null !\n");
		return NULL;
	}

	iniparser_freedict(ini);//free dirctionary obj	

	return str_value;
}

int setini_value(const char * ininame,const char *section,const char *key_section,char *value)
{
	int ret = 0;
	char key[128] = {0};
	FILE *ini_f;
	
	dictionary *ini;

	ini = iniparser_load(ininame);//parser the file
	if(ini == NULL){
		DEBUG_LOG(DEBUGLEVEL1,"can not open %s\n",ininame);
		return LOAD_INIFILE_ERROR;
	}
	
	snprintf(key,sizeof(key),"%s:%s",section,key_section);
	DEBUG_LOG(DEBUGLEVEL3,"key:[%s]\n",key);
	
	if(iniparser_find_entry(ini,section) == 1){
		ret = iniparser_set(ini, key, value);
	}
	else{
		ret = iniparser_set(ini,section,"section");
		ret = iniparser_set(ini, key, value);
	}
	
	ini_f=fopen(ininame,"w"); 
	if(ini_f == NULL){
		DEBUG_LOG(DEBUGLEVEL1,"fopen open %s error !\n",ininame);
		return LOAD_INIFILE_ERROR;
	}
	else{
		iniparser_dump_ini(ini,ini_f);
		fclose(ini_f);
	}	
	
	iniparser_freedict(ini);//free dirctionary obj	
	
	return 0;
}

int delini_key(const char * ininame,const char *section,const char *key_section)
{
	int ret = 0;
	char key[128] = {0};
	FILE *ini_f;
	
	dictionary *ini;

	ini = iniparser_load(ininame);//parser the file
	if(ini == NULL){
		DEBUG_LOG(DEBUGLEVEL1,"can not open %s\n",ininame);
		return LOAD_INIFILE_ERROR;
	}
	
	snprintf(key,sizeof(key),"%s:%s",section,key_section);
	DEBUG_LOG(DEBUGLEVEL3,"key:[%s]\n",key);
	
	dictionary_unset(ini,key);
	
	ini_f=fopen(ininame,"w");
	if(ini_f == NULL){
		DEBUG_LOG(DEBUGLEVEL1,"fopen open %s error !\n",ininame);
		return LOAD_INIFILE_ERROR;
	}
	else{
		iniparser_dump_ini(ini,ini_f);
		fclose(ini_f);
	}	

	return 0;
}

int delini_section(const char * ininame,const char *section)
{
	int ret = 0;
	char key[128] = {0};
	FILE *ini_f;
	
	dictionary *ini;

	ini = iniparser_load(ininame);//parser the file
	if(ini == NULL){
		DEBUG_LOG(DEBUGLEVEL1,"can not open %s\n",ininame);
		return LOAD_INIFILE_ERROR;
	}
	
	DEBUG_LOG(DEBUGLEVEL3,"section:[%s]\n",section);

	iniparser_unset(ini,section);

	ini_f=fopen(ininame,"w"); 
	if(ini_f == NULL){
		DEBUG_LOG(DEBUGLEVEL1,"fopen open %s error !\n",ininame);
		return LOAD_INIFILE_ERROR;
	}
	else{
		iniparser_dump_ini(ini,ini_f);
		fclose(ini_f);
	}	

	return 0;
}
