//+------------------------------------------------------------------+
//|                                              MetaTrader Stopouts |
//|                   Copyright 2001-2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "..\stdafx.h"
#include "stringfile.h"
//+------------------------------------------------------------------+
//| Конструктор                                                      |
//+------------------------------------------------------------------+
CStringFile::CStringFile(const int nBufSize) :
             m_file(INVALID_HANDLE_VALUE),m_file_size(0),
             m_buffer(new BYTE[nBufSize]),m_buffer_size(nBufSize),
             m_buffer_index(0),m_buffer_readed(0),m_buffer_line(0)
  {
  }
//+------------------------------------------------------------------+
//| Деструктор                                                       |
//+------------------------------------------------------------------+
CStringFile::~CStringFile()
  {
//--- закроем соединение
   Close();
//--- освободим буфер
   if(m_buffer!=NULL) { delete[] m_buffer; m_buffer=NULL; }
  }
//+------------------------------------------------------------------+
//| Открытие файла для чтения                                        |
//| dwAccess       -GENERIC_READ или GENERIC_WRITE                   |
//| dwCreationFlags-CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS        |
//+------------------------------------------------------------------+
bool CStringFile::Open(LPCTSTR lpFileName,const DWORD dwAccess,const DWORD dwCreationFlags)
  {
//--- закроем на всякий случай предыдущий файл
   Close();
//--- проверки
   if(lpFileName!=NULL)
     {
      //--- создадим файл
      m_file=CreateFile(lpFileName,dwAccess,FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,dwCreationFlags,FILE_ATTRIBUTE_NORMAL,NULL);
      //--- определим размер файла (не больше 4Gb)
      if(m_file!=INVALID_HANDLE_VALUE) m_file_size=GetFileSize(m_file,NULL);
     }
//--- вернем результат
   return(m_file!=INVALID_HANDLE_VALUE);
  }
//+------------------------------------------------------------------+
//| Запись буфера указанной длины в файл                             |
//+------------------------------------------------------------------+
int CStringFile::Read(void *buffer,const DWORD length)
  {
   DWORD readed=0;
//--- проверки
   if(m_file==INVALID_HANDLE_VALUE || buffer==NULL || length<1) return(0);
//--- считаем и вернем результат
   if(ReadFile(m_file,buffer,length,&readed,NULL)==0) readed=0;
//--- вернем кол-во считанных байт
   return(readed);
  }
//+------------------------------------------------------------------+
//| Чтение буфера указанной длины из файла                           |
//+------------------------------------------------------------------+
int CStringFile::Write(const void *buffer,const DWORD length)
  {
   DWORD written=0;
//--- проверки
   if(m_file==INVALID_HANDLE_VALUE || buffer==NULL || length<1) return(0);
//--- запишем данные
   if(WriteFile(m_file,buffer,length,&written,NULL)==0) written=0;
//--- вернем кол-во записанных байт
   return(written);
  }
//+------------------------------------------------------------------+
//| Выставимся на начало файла                                       |
//+------------------------------------------------------------------+
void CStringFile::Reset()
  {
//--- сбросим счетчики
   m_buffer_index=0;
   m_buffer_readed=0;
   m_buffer_line=0;
//--- выставимся на начало файла
   if(m_file!=INVALID_HANDLE_VALUE) SetFilePointer(m_file,0,NULL,FILE_BEGIN);
  }
//+------------------------------------------------------------------+
//| Заполняем строку и возвращаем номер строки. 0-ошибка           |
//+------------------------------------------------------------------+
int CStringFile::GetNextLine(char *line,const int maxsize)
  {
   char  *currsym=line,*lastsym=line+maxsize;
   BYTE  *curpos=m_buffer+m_buffer_index;
//--- проверки
   if(line==NULL || m_file==INVALID_HANDLE_VALUE || m_buffer==NULL) return(0);
//--- крутимся в цикле
   for(;;)
     {
      //--- первая строка или прочитали весь буфер
      if(m_buffer_line==0 || m_buffer_index==m_buffer_readed)
        {
         //--- зануляем счетчики
         m_buffer_index=0;
         m_buffer_readed=0;
         //--- читаем в буфер
         if(::ReadFile(m_file,m_buffer,m_buffer_size,(DWORD*)&m_buffer_readed,NULL)==0)
           {
            Close();
            return(0);
           }
         //--- считали 0 байт? конец файла
         if(m_buffer_readed<1) { *currsym=0; return(currsym!=line ? m_buffer_line:0); }
         curpos=m_buffer;
        }
      //--- анализируем буфер
      while(m_buffer_index<m_buffer_readed)
        {
         //--- дошли до конца?
         if(currsym>=lastsym) { *currsym=0; return(m_buffer_line); }
         //--- проанализируем символ (нашли конец строки)
         if(*curpos=='\n')
           {
            //--- был ли перед этим возврат каретки?
            if(currsym>line && currsym[-1]=='\r') currsym--; // был-вытираем его
            *currsym=0;
            //--- возвращаем номер строки
            m_buffer_line++;
            m_buffer_index++;
            return(m_buffer_line);
           }
         //--- обычный символ-копируем его
         *currsym++=*curpos++; m_buffer_index++;
        }
     }
//--- это невозможно...
   return(0);
  }
//+------------------------------------------------------------------+
