//+------------------------------------------------------------------+
//|                                              MetaTrader Stopouts |
//|                   Copyright 2001-2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once
//+------------------------------------------------------------------+
//| ������� �������������                                            |
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
//| ������������                                                     |
//+------------------------------------------------------------------+
class CConfiguration 
  {
private:
   CSync             m_sync;           // �������������
   PluginCfg        *m_cfgs;           // �������
   int               m_cfgs_total;     // ���������� ��������
   int               m_cfgs_max;       // ������������ ���������� ��������
   PluginCfg       **m_cfgs_index;     // ������ ��������
   char              m_filename[256];  // ��� ����� ������������
public:
                     CConfiguration();
                    ~CConfiguration();
   //--- �������� ��������
   int               Load(const char *filename);
   //--- ������ � ������ ���������
   int               Total(void);
   int               Add(const int pos,const PluginCfg *cfg);
   int               Delete(const char *name);
   int               Next(const int index,PluginCfg *cfg);
   int               Get(const char *name,PluginCfg *cfg,const int pos=0);
   int               Set(const PluginCfg *cfgs,const int cfgs_total);
   //--- ��������������� ������
   int               GetInteger(const int pos,const char *name,int *value,const char *defvalue=NULL);
   int               GetString(const int pos,const char *name,char *value,const int size,const char *defvalue=NULL);
   int               GetFloat(const int pos,const char *name,double *value,const char *defvalue=NULL);

private:
   //--- ���������� �������
   int               Save(void);
   //--- ����������
   static int        SortByName(const void *param1,const void *param2);
   static int        SortIndex(const void *param1,const void *param2);
  };

extern CConfiguration ExtConfig;
//+------------------------------------------------------------------+
