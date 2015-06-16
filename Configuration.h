//+------------------------------------------------------------------+
//|                                              MetaTrader Stopouts |
//|                   Copyright 2001-2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once
//+------------------------------------------------------------------+
//| Простой синхронизатор                                            |
//+------------------------------------------------------------------+
class CSync
  {
private:
   CRITICAL_SECTION  m_cs;
public:
                     CSync()  { ZeroMemory(&m_cs,sizeof(m_cs)); InitializeCriticalSection(&m_cs); }
                    ~CSync()  { DeleteCriticalSection(&m_cs);   ZeroMemory(&m_cs,sizeof(m_cs));   }
   inline void       Lock()   { EnterCriticalSection(&m_cs); }
   inline void       Unlock() { LeaveCriticalSection(&m_cs); }
  };
//+------------------------------------------------------------------+
//| Конфигурация                                                     |
//+------------------------------------------------------------------+
class CConfiguration 
  {
private:
   CSync             m_sync;           // синхронизатор
   PluginCfg        *m_cfgs;           // конфиги
   int               m_cfgs_total;     // количество конфигов
   int               m_cfgs_max;       // максимальное количество конфигов
   PluginCfg       **m_cfgs_index;     // индекс конфигов
   char              m_filename[256];  // имя файла конфигураций
public:
                     CConfiguration();
                    ~CConfiguration();
   //--- загрузка конфигов
   int               Load(const char *filename);
   //--- работа с сырыми конфигами
   int               Total(void);
   int               Add(const int pos,const PluginCfg *cfg);
   int               Delete(const char *name);
   int               Next(const int index,PluginCfg *cfg);
   int               Get(const char *name,PluginCfg *cfg,const int pos=0);
   int               Set(const PluginCfg *cfgs,const int cfgs_total);
   //--- высокоуровневый доступ
   int               GetInteger(const int pos,const char *name,int *value,const char *defvalue=NULL);
   int               GetString(const int pos,const char *name,char *value,const int size,const char *defvalue=NULL);
   int               GetFloat(const int pos,const char *name,double *value,const char *defvalue=NULL);

private:
   //--- сохранение конфига
   int               Save(void);
   //--- сортировки
   static int        SortByName(const void *param1,const void *param2);
   static int        SortIndex(const void *param1,const void *param2);
  };

extern CConfiguration ExtConfig;
//+------------------------------------------------------------------+
