//+------------------------------------------------------------------+
//|                                              MetaTrader Stopouts |
//|                   Copyright 2001-2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "..\stdafx.h"

CConfiguration ExtConfig;
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CConfiguration::CConfiguration() : m_cfgs(NULL),m_cfgs_total(0),m_cfgs_max(0),m_cfgs_index(NULL)
  {
   m_filename[0]=0;
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CConfiguration::~CConfiguration()
  {
//--- ��� � ����
   m_sync.Lock();
//--- ������� ���
   if(m_cfgs      !=NULL) { delete[] m_cfgs;       m_cfgs      =NULL; }
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- ������� ���
   m_cfgs_total=m_cfgs_max=0;
//--- ����������
   m_sync.Unlock();
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Load(const char *filename)
  {
   CStringFile file;
   char        buffer[256],*start,*cp;
   PluginCfg   cfg={0},*temp;
//--- ��������
   if(filename==NULL) return(FALSE);
//--- ��� � ����
   m_sync.Lock();
//--- �������� ���
   m_filename[0]=0; m_cfgs_total=0;
//--- ����� ��� ����������� ������ ������
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- �������� ��� �����
   COPY_STR(m_filename,filename);
//--- ������� ����
   if(!file.Open(m_filename,GENERIC_READ,OPEN_ALWAYS))
     {
      m_sync.Unlock();
      return(FALSE);
     }
//--- ������ �������
   while(file.GetNextLine(buffer,sizeof(buffer)-1)>0)
     {
      //--- ��������� ������ ������
      if(buffer[0]==';' || buffer[0]==0) continue;
      //--- ������� ������ �� ������ ������
      TERMINATE_STR(buffer);
      //--- ���������� �������
      start=buffer; while(*start==' ') start++;
      //--- ������ ����� � ������� �� ����
      if((cp=strstr(start,"="))==NULL) continue;
      *cp=0;
      //--- ������� ��� ���������
      COPY_STR(cfg.name,start);
      //--- ������ ���������� ���� ��������, ��������� �������
      start=cp+1; while(*start==' ') start++;
      //--- �������� ��������
      COPY_STR(cfg.value,start);
      //--- �������� �������
      cfg.reserved[0]=m_cfgs_total+1;
      //--- ������� ���� �� �����
      if(m_cfgs==NULL || m_cfgs_total>=m_cfgs_max)
        {
         //--- �������� ����� �������� ������
         if((temp=new PluginCfg[m_cfgs_total+1024])==NULL) { m_sync.Unlock(); return(FALSE); }
         //--- ��������� ��� ������� � ������ ������� � �����
         if(m_cfgs!=NULL)
           {
            memcpy(temp,m_cfgs,sizeof(PluginCfg)*m_cfgs_total);
            delete[] m_cfgs;
           }
         //--- �������� �����
         m_cfgs    =temp;
         m_cfgs_max=m_cfgs_total+1024;
        }
      //--- ��������� ������ � ������
      memcpy(&m_cfgs[m_cfgs_total++],&cfg,sizeof(PluginCfg));
     }
//--- ����������� ������� �� �����
   qsort(m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName);
//--- ������� ����
   file.Close();
//--- ���������� � ������ ����
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Total(void)
  {
   int total;
//--- ������� � ���� ���������� �������
   m_sync.Lock();
   total=m_cfgs_total;
   m_sync.Unlock();
//--- ������ ���������� ��������
   return(total);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Add(const int pos,const PluginCfg *cfg)
  {
   PluginCfg *temp;
   int        i;
//--- ��������
   if(cfg==NULL) return(FALSE);
//--- ��� � ����
   m_sync.Lock();
//--- ������� ����� ����?
   if(m_cfgs==NULL || m_cfgs_total>=m_cfgs_max)
     {
      //--- ������� ����� �������� �������
      if((temp=new PluginCfg[m_cfgs_total+1024])==NULL) { m_sync.Unlock(); return(FALSE); }
      //--- ���� � ��� ���-�� ���� ��������� � ����� ����� � ������ ������
      if(m_cfgs!=NULL)
        {
         memcpy(temp,m_cfgs,sizeof(PluginCfg)*m_cfgs_total);
         delete[] m_cfgs;
        }
      //--- ������� �� ����� �����
      m_cfgs    =temp;
      m_cfgs_max=m_cfgs_total+1024;
     }
//--- ���������
   memcpy(&m_cfgs[m_cfgs_total],cfg,sizeof(PluginCfg));
//--- ������ �������
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- �������� ������� ���������
   m_cfgs[m_cfgs_total].reserved[0]=(pos<1?m_cfgs_total+1:pos);
//--- � ���� � ���� ����� �� ������� ������� ������
   for(i=0;i<m_cfgs_total;i++)
      if(m_cfgs[i].reserved[0]>=m_cfgs[m_cfgs_total].reserved[0]) 
         m_cfgs[i].reserved[0]++;
//--- ���������, ���� ��������
   m_cfgs_total++;
//--- ������������� ������� �� �����
   qsort(m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName);
//--- ����������
   Save();
//--- ���������� � ������ ��� ��� ����
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Delete(const char *name)
  {
   PluginCfg *temp;
//--- ��������
   if(name==NULL) return(FALSE);
//--- ��� ������ � ����
   m_sync.Lock();
//--- ����� ������ �������
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- ������ ������
   if((temp=(PluginCfg *)bsearch(name,m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName))==NULL)
     {
      m_sync.Unlock();
      return(FALSE);
     }
//--- ���������� ����������
   if(temp-m_cfgs<m_cfgs_total)
      memmove(temp,temp+1,sizeof(PluginCfg)*(m_cfgs_total-(temp-m_cfgs)-1));
//--- ������������ ����������
   m_cfgs_total--;
//--- ����������
   Save();
//--- ������ ��� � ������ ���-���
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Next(const int index,PluginCfg *cfg)
  {
   int i;
//--- ��������
   if(index<0 || cfg==NULL) return(FALSE);
//--- ��� ������ � ����
   m_sync.Lock();
//--- ������ �������� ���������
   if(index>=m_cfgs_total) { m_sync.Unlock(); return(FALSE); }
//--- �������� � ��� ������ ����?
   if(m_cfgs_index==NULL)
     {
      //--- ������ ������: ��� ������ ������� ��� ���� �����
      if((m_cfgs_index=new PluginCfg*[m_cfgs_total])==NULL) { m_sync.Unlock(); return(FALSE); }
      //--- ���������� ������ ������
      for(i=0;i<m_cfgs_total;i++) m_cfgs_index[i]=&m_cfgs[i];
      //--- ������ �������� �������������
      qsort(m_cfgs_index,m_cfgs_total,sizeof(PluginCfg *),SortIndex);
      //--- ��������� ������� � ���������
      for(i=0;i<m_cfgs_total;i++) m_cfgs_index[i]->reserved[0]=i+1;
     }
//--- ������� �� ��� �����
   memcpy(cfg,m_cfgs_index[index],sizeof(PluginCfg));
//--- ����������
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Get(const char *name,PluginCfg *cfg,const int pos)
  {
   PluginCfg *temp;
   int        i;
//--- ��������
   if(name==NULL || cfg==NULL) return(FALSE);
//--- ��� ������ � ����
   m_sync.Lock();
//--- ������ ������ �� �����
   if((temp=(PluginCfg *)bsearch(name,m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName))==NULL)
     {
      m_sync.Unlock();
      return(FALSE);
     }
//--- ��������� �� ��� �������
   memcpy(cfg,temp,sizeof(PluginCfg));
//--- ������� ������� ����������?
   if(pos>0 && temp->reserved[0]!=pos)
     {
      //--- �������� ����� �������
      temp->reserved[0]=pos;
      //--- ������ ������
      if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
      //--- � ���� � ���� ����� �� ������� ������� ������
      for(i=0;i<m_cfgs_total;i++)
         if(m_cfgs[i].reserved[0]>=pos && temp!=&m_cfgs[i])
            m_cfgs[i].reserved[0]++;
      //--- ��������
      Save();
     }
//--- ���������� � ������ ��� ��� ����
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Set(const PluginCfg *cfgs,const int cfgs_total)
  {
   PluginCfg *temp;
   int        i;
//--- ��������
   if(cfgs==NULL || cfgs_total<0) return(FALSE);
//--- ��� ������ � ����
   m_sync.Lock();
//--- ����� ������ �������
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- �������� ����� ������?
   if(m_cfgs==NULL || m_cfgs_total>=m_cfgs_max)
     {
      //--- ������� ����� ��������
      if((temp=new PluginCfg[m_cfgs_total+1024])==NULL) { m_sync.Unlock(); return(FALSE); }
      //--- ���� � ��� ��� ����� ������ ���
      if(m_cfgs!=NULL) delete[] m_cfgs;
      //--- �������� ����� �����
      m_cfgs    =temp;
      m_cfgs_max=m_cfgs_total+1024;
     }
//--- �������� ����� ���������
   memcpy(m_cfgs,cfgs,sizeof(PluginCfg)*cfgs_total);
//--- �������� ����������
   m_cfgs_total=cfgs_total;
//--- �������� �������
   for(i=0;i<m_cfgs_total;i++) m_cfgs[i].reserved[0]=i+1;
//--- ����������� ��� ����
   qsort(m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName);
//--- ����������
   Save();
//--- ���������� � ������ ��� ����
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| �������� ����� ����� �� �����                                    |
//+------------------------------------------------------------------+
int CConfiguration::GetInteger(const int pos,const char *name,int *value,const char *defvalue)
  {
   PluginCfg cfg={0};
//--- ��������
   if(name==NULL || value==NULL) return(FALSE);
//--- ������� �������� ������
   if(Get(name,&cfg,pos)==FALSE)
     {
      //--- ���������� ����� ������
      COPY_STR(cfg.name,name);
      if(defvalue!=NULL) COPY_STR(cfg.value,defvalue);
      //--- ���������
      if(Add(pos,&cfg)==FALSE) return(FALSE);
     }
//--- ������� ��������
   *value=atoi(cfg.value);
//--- ������ ����
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| �������� ������ �� �����                                         |
//+------------------------------------------------------------------+
int CConfiguration::GetString(const int pos,const char *name,char *value,const int size,const char *defvalue)
  {
   PluginCfg cfg={0};
//--- ��������
   if(name==NULL || value==NULL || size<0) return(FALSE);
//--- ������� �������� ������
   if(Get(name,&cfg,pos)==FALSE)
     {
      //--- ���������� ����� ������
      COPY_STR(cfg.name,name);
      if(defvalue!=NULL) COPY_STR(cfg.value,defvalue);
      //--- ��������� 
      if(Add(pos,&cfg)==FALSE) return(FALSE);
     }
//--- ������� ������
   strncpy(value,cfg.value,size);
//--- ������ ��� ����
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| �������� ����� � ��������� ������ �� �����                       |
//+------------------------------------------------------------------+
int CConfiguration::GetFloat(const int pos,const char *name,double *value,const char *defvalue)
  {
   PluginCfg cfg={0};
//--- ��������
   if(name==NULL || value==NULL) return(FALSE);
//--- ������� �������� ������
   if(Get(name,&cfg,pos)==FALSE)
     {
      //--- ���������� ����� ������
      COPY_STR(cfg.name,name);
      if(defvalue!=NULL) COPY_STR(cfg.value,defvalue);
      //--- ���������
      if(Add(pos,&cfg)==FALSE) return(FALSE);
     }
//--- ������� ��������
   *value=atof(cfg.value);
//--- ������ ����
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| ����������� � ����                                               |
//+------------------------------------------------------------------+
int CConfiguration::Save(void)
  {
   CStringFile file;
   int         i;
   char        buffer[256];
//--- ��������
   if(m_filename[0]==0 || m_cfgs==NULL || m_cfgs_total<0) return(FALSE);
//--- �������� ������� � ���� ��� �������� ��
   if(m_cfgs_index==NULL)
     {
      //--- ������ ������: ��� ������ ������� ��� ���� �����
      if((m_cfgs_index=new PluginCfg*[m_cfgs_total])==NULL) return(FALSE);
      //--- ���������� ������ ������
      for(i=0;i<m_cfgs_total;i++) m_cfgs_index[i]=&m_cfgs[i];
      //--- ������ �������� �������������
      qsort(m_cfgs_index,m_cfgs_total,sizeof(PluginCfg *),SortIndex);
      //--- ��������� ������� � ���������
      for(i=0;i<m_cfgs_total;i++) m_cfgs_index[i]->reserved[0]=i+1;
     }
//--- ������� ����
   if(!file.Open(m_filename,GENERIC_WRITE,CREATE_ALWAYS)) return(FALSE);
//--- ���������� ����� �� ��������, ����� � ����� ��� ������ ������� ��������
   for(i=0;i<m_cfgs_total;i++)
     {
      //--- ���������� ������
      _snprintf(buffer,sizeof(buffer)-1,"%s=%s\n",m_cfgs_index[i]->name,m_cfgs_index[i]->value);
      //--- ������� �� �� ����
      file.Write(buffer,strlen(buffer));
     }
//--- ������� ����
   file.Close();
//--- ������ ��� ����
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::SortByName(const void *param1,const void *param2)
  {
   return strcmp(((PluginCfg *)param1)->name,((PluginCfg *)param2)->name);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::SortIndex(const void *param1,const void *param2)
  {
   return((*((PluginCfg **)param1))->reserved[0]-(*((PluginCfg **)param2))->reserved[0]);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
