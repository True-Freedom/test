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
//--- все в локе
   m_sync.Lock();
//--- удаляем все
   if(m_cfgs      !=NULL) { delete[] m_cfgs;       m_cfgs      =NULL; }
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- занулим все
   m_cfgs_total=m_cfgs_max=0;
//--- разлочимся
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
//--- проверки
   if(filename==NULL) return(FALSE);
//--- все в локе
   m_sync.Lock();
//--- зануляем все
   m_filename[0]=0; m_cfgs_total=0;
//--- нужно еще обязательно удлать индекс
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- запомним имя файла
   COPY_STR(m_filename,filename);
//--- откроем файл
   if(!file.Open(m_filename,GENERIC_READ,OPEN_ALWAYS))
     {
      m_sync.Unlock();
      return(FALSE);
     }
//--- читаем конфиги
   while(file.GetNextLine(buffer,sizeof(buffer)-1)>0)
     {
      //--- пропустим пустые строки
      if(buffer[0]==';' || buffer[0]==0) continue;
      //--- обрежем строку на всякий случай
      TERMINATE_STR(buffer);
      //--- пропускаем пробелы
      start=buffer; while(*start==' ') start++;
      //--- найдем равно и обрежем по нему
      if((cp=strstr(start,"="))==NULL) continue;
      *cp=0;
      //--- получим имя параметра
      COPY_STR(cfg.name,start);
      //--- теперь собственно само значение, пропустим пробелы
      start=cp+1; while(*start==' ') start++;
      //--- копируем значение
      COPY_STR(cfg.value,start);
      //--- выставим порядок
      cfg.reserved[0]=m_cfgs_total+1;
      //--- смотрим есть ли место
      if(m_cfgs==NULL || m_cfgs_total>=m_cfgs_max)
        {
         //--- выделяем буфер большего размер
         if((temp=new PluginCfg[m_cfgs_total+1024])==NULL) { m_sync.Unlock(); return(FALSE); }
         //--- перенесем все чтобыло в старом массиве в новый
         if(m_cfgs!=NULL)
           {
            memcpy(temp,m_cfgs,sizeof(PluginCfg)*m_cfgs_total);
            delete[] m_cfgs;
           }
         //--- заменяем буфер
         m_cfgs    =temp;
         m_cfgs_max=m_cfgs_total+1024;
        }
      //--- добавляем конфиг в список
      memcpy(&m_cfgs[m_cfgs_total++],&cfg,sizeof(PluginCfg));
     }
//--- отсортируем конфиги по имени
   qsort(m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName);
//--- закроем файл
   file.Close();
//--- разлочимся и скажем окей
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Total(void)
  {
   int total;
//--- вытащим в локе количество записей
   m_sync.Lock();
   total=m_cfgs_total;
   m_sync.Unlock();
//--- вернем количество конфигов
   return(total);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Add(const int pos,const PluginCfg *cfg)
  {
   PluginCfg *temp;
   int        i;
//--- проверки
   if(cfg==NULL) return(FALSE);
//--- все в локе
   m_sync.Lock();
//--- смортим место есть?
   if(m_cfgs==NULL || m_cfgs_total>=m_cfgs_max)
     {
      //--- выделим буфер большего размера
      if((temp=new PluginCfg[m_cfgs_total+1024])==NULL) { m_sync.Unlock(); return(FALSE); }
      //--- если у нас что-то было скопируем в новый буфер и удалим старый
      if(m_cfgs!=NULL)
        {
         memcpy(temp,m_cfgs,sizeof(PluginCfg)*m_cfgs_total);
         delete[] m_cfgs;
        }
      //--- заменим на новый буфер
      m_cfgs    =temp;
      m_cfgs_max=m_cfgs_total+1024;
     }
//--- вставляем
   memcpy(&m_cfgs[m_cfgs_total],cfg,sizeof(PluginCfg));
//--- удалим индексы
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- выставим позицию просмотра
   m_cfgs[m_cfgs_total].reserved[0]=(pos<1?m_cfgs_total+1:pos);
//--- у всех у кого такая же позиция опустим пониже
   for(i=0;i<m_cfgs_total;i++)
      if(m_cfgs[i].reserved[0]>=m_cfgs[m_cfgs_total].reserved[0]) 
         m_cfgs[i].reserved[0]++;
//--- подкрутим, типа добавили
   m_cfgs_total++;
//--- пересортируем конфиги по имени
   qsort(m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName);
//--- сохранимся
   Save();
//--- разлочимся и скажем что все окей
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Delete(const char *name)
  {
   PluginCfg *temp;
//--- проверки
   if(name==NULL) return(FALSE);
//--- все делаем в локе
   m_sync.Lock();
//--- сразу удалим индексы
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- найдем конфиг
   if((temp=(PluginCfg *)bsearch(name,m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName))==NULL)
     {
      m_sync.Unlock();
      return(FALSE);
     }
//--- собственно схлопываем
   if(temp-m_cfgs<m_cfgs_total)
      memmove(temp,temp+1,sizeof(PluginCfg)*(m_cfgs_total-(temp-m_cfgs)-1));
//--- корректируем количество
   m_cfgs_total--;
//--- сохранимся
   Save();
//--- снимем лок и вернем тип-топ
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int CConfiguration::Next(const int index,PluginCfg *cfg)
  {
   int i;
//--- проверки
   if(index<0 || cfg==NULL) return(FALSE);
//--- все делаем в локе
   m_sync.Lock();
//--- теперь проверим детальней
   if(index>=m_cfgs_total) { m_sync.Unlock(); return(FALSE); }
//--- проверим у нас индекс есть?
   if(m_cfgs_index==NULL)
     {
      //--- строим индекс: для начала выделим под него место
      if((m_cfgs_index=new PluginCfg*[m_cfgs_total])==NULL) { m_sync.Unlock(); return(FALSE); }
      //--- собственно строим индекс
      for(i=0;i<m_cfgs_total;i++) m_cfgs_index[i]=&m_cfgs[i];
      //--- теперь осталось отсортировать
      qsort(m_cfgs_index,m_cfgs_total,sizeof(PluginCfg *),SortIndex);
      //--- подправим порядок в оригинале
      for(i=0;i<m_cfgs_total;i++) m_cfgs_index[i]->reserved[0]=i+1;
     }
//--- отдадим то что хотят
   memcpy(cfg,m_cfgs_index[index],sizeof(PluginCfg));
//--- разлочимся
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
//--- проверки
   if(name==NULL || cfg==NULL) return(FALSE);
//--- все делаем в локе
   m_sync.Lock();
//--- найдем конфиг по имени
   if((temp=(PluginCfg *)bsearch(name,m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName))==NULL)
     {
      m_sync.Unlock();
      return(FALSE);
     }
//--- скопируем то что просили
   memcpy(cfg,temp,sizeof(PluginCfg));
//--- смотрим позицию изменилась?
   if(pos>0 && temp->reserved[0]!=pos)
     {
      //--- выставим новую позицию
      temp->reserved[0]=pos;
      //--- удалим индекс
      if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
      //--- у всех у кого такая же позиция опустим пониже
      for(i=0;i<m_cfgs_total;i++)
         if(m_cfgs[i].reserved[0]>=pos && temp!=&m_cfgs[i])
            m_cfgs[i].reserved[0]++;
      //--- сохраним
      Save();
     }
//--- разлочимся и скажем что все окей
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
//--- проверки
   if(cfgs==NULL || cfgs_total<0) return(FALSE);
//--- все делаем в локе
   m_sync.Lock();
//--- сразу удалим индексы
   if(m_cfgs_index!=NULL) { delete[] m_cfgs_index; m_cfgs_index=NULL; }
//--- проверим места хватит?
   if(m_cfgs==NULL || m_cfgs_total>=m_cfgs_max)
     {
      //--- выделим буфер побольше
      if((temp=new PluginCfg[m_cfgs_total+1024])==NULL) { m_sync.Unlock(); return(FALSE); }
      //--- если у нас был буфер удалим его
      if(m_cfgs!=NULL) delete[] m_cfgs;
      //--- выставим новый буфер
      m_cfgs    =temp;
      m_cfgs_max=m_cfgs_total+1024;
     }
//--- копируем новые настройки
   memcpy(m_cfgs,cfgs,sizeof(PluginCfg)*cfgs_total);
//--- выставим количество
   m_cfgs_total=cfgs_total;
//--- выставим порядок
   for(i=0;i<m_cfgs_total;i++) m_cfgs[i].reserved[0]=i+1;
//--- отсортируем все дело
   qsort(m_cfgs,m_cfgs_total,sizeof(PluginCfg),SortByName);
//--- сохранимся
   Save();
//--- разлочимся и скажем все окей
   m_sync.Unlock();
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| Получает целое число по имени                                    |
//+------------------------------------------------------------------+
int CConfiguration::GetInteger(const int pos,const char *name,int *value,const char *defvalue)
  {
   PluginCfg cfg={0};
//--- проверки
   if(name==NULL || value==NULL) return(FALSE);
//--- пробуем получить конфиг
   if(Get(name,&cfg,pos)==FALSE)
     {
      //--- подготовим новый конфиг
      COPY_STR(cfg.name,name);
      if(defvalue!=NULL) COPY_STR(cfg.value,defvalue);
      //--- добавляем
      if(Add(pos,&cfg)==FALSE) return(FALSE);
     }
//--- вытащим значение
   *value=atoi(cfg.value);
//--- вернем окей
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| Получает строку по имени                                         |
//+------------------------------------------------------------------+
int CConfiguration::GetString(const int pos,const char *name,char *value,const int size,const char *defvalue)
  {
   PluginCfg cfg={0};
//--- проверки
   if(name==NULL || value==NULL || size<0) return(FALSE);
//--- пробуем получить конфиг
   if(Get(name,&cfg,pos)==FALSE)
     {
      //--- подготовим новый конфиг
      COPY_STR(cfg.name,name);
      if(defvalue!=NULL) COPY_STR(cfg.value,defvalue);
      //--- добавляем 
      if(Add(pos,&cfg)==FALSE) return(FALSE);
     }
//--- вытащим строку
   strncpy(value,cfg.value,size);
//--- скажем все окей
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| Получает число с плавающей точкой по имени                       |
//+------------------------------------------------------------------+
int CConfiguration::GetFloat(const int pos,const char *name,double *value,const char *defvalue)
  {
   PluginCfg cfg={0};
//--- проверки
   if(name==NULL || value==NULL) return(FALSE);
//--- пробуем получить конфиг
   if(Get(name,&cfg,pos)==FALSE)
     {
      //--- подготовим новый конфиг
      COPY_STR(cfg.name,name);
      if(defvalue!=NULL) COPY_STR(cfg.value,defvalue);
      //--- добавляем
      if(Add(pos,&cfg)==FALSE) return(FALSE);
     }
//--- вытащим значение
   *value=atof(cfg.value);
//--- вернем окей
   return(TRUE);
  }
//+------------------------------------------------------------------+
//| Обязательно в локе                                               |
//+------------------------------------------------------------------+
int CConfiguration::Save(void)
  {
   CStringFile file;
   int         i;
   char        buffer[256];
//--- проверки
   if(m_filename[0]==0 || m_cfgs==NULL || m_cfgs_total<0) return(FALSE);
//--- проверим индексы и если что создадим их
   if(m_cfgs_index==NULL)
     {
      //--- строим индекс: для начала выделим под него место
      if((m_cfgs_index=new PluginCfg*[m_cfgs_total])==NULL) return(FALSE);
      //--- собственно строим индекс
      for(i=0;i<m_cfgs_total;i++) m_cfgs_index[i]=&m_cfgs[i];
      //--- теперь осталось отсортировать
      qsort(m_cfgs_index,m_cfgs_total,sizeof(PluginCfg *),SortIndex);
      //--- подправим порядок в оригинале
      for(i=0;i<m_cfgs_total;i++) m_cfgs_index[i]->reserved[0]=i+1;
     }
//--- откроем файл
   if(!file.Open(m_filename,GENERIC_WRITE,CREATE_ALWAYS)) return(FALSE);
//--- собственно пишем по индексам, чтобы в файле был нужный порядок конфигов
   for(i=0;i<m_cfgs_total;i++)
     {
      //--- сформируем строку
      _snprintf(buffer,sizeof(buffer)-1,"%s=%s\n",m_cfgs_index[i]->name,m_cfgs_index[i]->value);
      //--- сбросим ее на диск
      file.Write(buffer,strlen(buffer));
     }
//--- закроем файл
   file.Close();
//--- скажем все окей
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
