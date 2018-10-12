
#ifndef	__FUNC_INIPARSE__
#define	__FUNC_INIPARSE__

#define LOAD_INIFILE_ERROR -101
#define EXIST_INIFILE_ERROR -102

int getini_int_value(const char * ininame,const char *section,const char *key_section);
char *getini_string_value(const char * ininame,const char *section,const char *key_section);

int setini_value(const char * ininame,const char *section,const char *key_section,char *value);

int delini_key(const char * ininame,const char *section,const char *key_section);
int delini_section(const char * ininame,const char *section);

#endif
