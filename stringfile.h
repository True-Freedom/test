//+------------------------------------------------------------------+
//|                                              MetaTrader Stopouts |
//|                   Copyright 2001-2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once
//+------------------------------------------------------------------+
//| Класс построкового чтения файла                                  |
//+------------------------------------------------------------------+
class CStringFile
  {
private:
   HANDLE            m_file;                 // хендл файла
   DWORD             m_file_size;            // размер файла
   BYTE             *m_buffer;               // буфер для чтения
   int               m_buffer_size;          // его размер
   int               m_buffer_index;         // текущая позиция парсинга
   int               m_buffer_readed;        // размер считанного в память буфера
   int               m_buffer_line;          // счетчик строк в файле

public:
                     CStringFile(const int nBufSize=65536);
                    ~CStringFile();
   //---
   bool              Open(LPCTSTR lpFileName,const DWORD dwAccess,const DWORD dwCreationFlags);
   inline void       Close() { if(m_file!=INVALID_HANDLE_VALUE) { CloseHandle(m_file); m_file=INVALID_HANDLE_VALUE; } m_file_size=0; }
   inline DWORD      Size() const { return(m_file_size); }
   int               Read(void  *buffer,const DWORD length);
   int               Write(const void *buffer,const DWORD length);
   void              Reset();
   int               GetNextLine(char *line,const int maxsize);
  };
//+------------------------------------------------------------------+
