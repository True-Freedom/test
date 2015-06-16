#include "stdafx.h"
#include "PluginConfig.h"

CPluginConfig *CPluginConfig::m_self = nullptr;
CPluginConfig::Destroyer CPluginConfig::m_destroyer;


/*
	Destroyer
*/
CPluginConfig::Destroyer::Destroyer()
	: m_config(nullptr)
{
}

CPluginConfig::Destroyer::~Destroyer()
{
	if (m_config)
		delete m_config;
}

void CPluginConfig::Destroyer::Initialize(CPluginConfig *config)
{
	if (!m_config)
		m_config = config;
}


/*
	Configuration
*/
CPluginConfig::CPluginConfig(void)
	: m_total(0)
{
	m_cfg_arr = static_cast<PluginCfg*>(HeapAlloc(GetProcessHeap(),
		HEAP_ZERO_MEMORY, MAX_PARAMS_COUNT * sizeof(PluginCfg)));
}


CPluginConfig::~CPluginConfig(void)
{
	if (m_cfg_arr)
	{
		HeapFree(GetProcessHeap(), 0, m_cfg_arr);
	}
	m_sink.clear();
}

CPluginConfig& CPluginConfig::Instance(void)
{
	if (!m_self)
	{
		m_self = new CPluginConfig();
		m_destroyer.Initialize(m_self);
	}

	return *m_self;
}


int CPluginConfig::GetInt(const char *name, int *value, const char *defvalue)
{
	char strValue[128];
	int result = GetValue(name, strValue, _countof(strValue), defvalue);
	*value = atoi(strValue);
	return result;
}

int CPluginConfig::GetFloat(const char *name, double *value, const char *defvalue)
{
	char strValue[128];
	int result = GetValue(name, strValue, _countof(strValue), defvalue);
	*value = atof(strValue);
	return result;
}

int CPluginConfig::GetString(const char *name, char *value, const int size, const char *defvalue)
{
	return GetValue(name, value, size, defvalue);
}

bool CPluginConfig::CheckParam(const char *name)
{
	CLocker locker(m_cs);
	return (name && Search(name));
}

bool CPluginConfig::CheckValue(const char *name, const char *value)
{
	CLocker locker(m_cs);
	bool result = false;

	if (name && value)
	{
		PluginCfg *conf = Search(name);
		if (conf)
		{
			result  = (strcmp(conf->value, value) == 0);
		}
	}

	return result;
}

bool CPluginConfig::Check(const char *name)
{
	CLocker locker(m_cs);
	return (name && Search(name));
}

bool CPluginConfig::Check(const char *name, const char *value)
{
	CLocker locker(m_cs);
	bool result = false;

	if (name && value)
	{
		PluginCfg *conf = Search(name);
		if (conf)
		{
			result  = (strcmp(conf->value, value) == 0);
		}
	}

	return result;
}


int CPluginConfig::GetValue(const char *name, char *value, const int size, const char *defvalue)
{
	CLocker locker(m_cs);
	int result = FALSE;

	if (name && value)
	{
		PluginCfg *conf = Search(name);
		if (conf)
		{
			StringCchCopy(value, size, conf->value);
			result = TRUE;
		}
		else if (defvalue)
		{
			PluginCfg cfg = { 0 };
			StringCchCopy(cfg.name, _countof(cfg.name), name);
			StringCchCopy(cfg.value, _countof(cfg.value), defvalue);
			StringCchCopy(value, size, defvalue);
			result = Add(&cfg);
		}
	}

	return result;
}

int CPluginConfig::SetValue(const char *name, const char *value)
{
	CLocker locker(m_cs);
	int result = FALSE;

	if (name && value)
	{
		PluginCfg cfg = { 0 };
		StringCchCopy(cfg.name, _countof(cfg.name), name);
		StringCchCopy(cfg.value, _countof(cfg.value), value);
		result = Add(&cfg);
	}

	return result;
}




int CPluginConfig::Add(const PluginCfg *value)
{
	CLocker locker(m_cs);
	BOOL result = FALSE;
	PluginCfg *conf = SearchAdd(value->name);

	if (conf)
	{
		memcpy(conf, value, sizeof(PluginCfg));
		Save();
		result = TRUE;
	}

	return result;
}

int CPluginConfig::Set(const PluginCfg *values, const int total)
{
	CLocker locker(m_cs);
	int result = FALSE;
	
	if (total >= 0)
	{
		m_total = min(MAX_PARAMS_COUNT, total);

		if (values)
		{
			// скопируем параметры
			memcpy(m_cfg_arr, values, m_total * sizeof(PluginCfg));
			// обнулим хвост
			memset(&m_cfg_arr[m_total], 0, (MAX_PARAMS_COUNT - m_total) * sizeof(PluginCfg));
			result = TRUE;
		}
		else	// ??? все параметры удалены
			memset(m_cfg_arr, 0, MAX_PARAMS_COUNT * sizeof(PluginCfg));

		Save();
	}

	return result;
}

int CPluginConfig::Get(const char *name, PluginCfg *value)
{
	CLocker locker(m_cs);
	PluginCfg *conf = nullptr;
	
	if (name && value)
	{
		if (conf = Search(name))
			memcpy(value, conf, sizeof(PluginCfg));
	}

	return (conf != 0);
}

int CPluginConfig::Next(const int index, PluginCfg *value)
{
	CLocker locker(m_cs);
	BOOL result = FALSE;
	
	if (value && m_cfg_arr && index >= 0 && index < m_total)
	{
		memcpy(value, &m_cfg_arr[index], sizeof(PluginCfg));
		result = TRUE;
	}

	return result;
}

int CPluginConfig::Delete(const char *name)
{
	CLocker locker(m_cs);
	BOOL result = FALSE;

	if (name)
	{
		PluginCfg *conf = Search(name);
		if (conf)
		{
			m_total--;
			int index = int(conf - m_cfg_arr);
			int count = m_total - index;
			if (index + 1 < MAX_PARAMS_COUNT && count > 0)
				memmove(conf, conf + 1, count * sizeof(PluginCfg));
			memset(&m_cfg_arr[m_total], 0, sizeof(PluginCfg));
			Save();
			result = TRUE;
		}
	}

	return result;
}

int CPluginConfig::Total(void)
{
	return m_total;
}

int CPluginConfig::Log(LPSTR value, int *len)
{
	// ???
	return 0;
}

PluginCfg* CPluginConfig::Search(const char *name)
{
	CLocker locker(m_cs);
	PluginCfg *conf = nullptr;

	if (m_cfg_arr && name)
	{
		for (int i = 0; i < m_total; i++)
		{
			if (strcmp(name, m_cfg_arr[i].name) == 0)
			{
				conf = &m_cfg_arr[i];
				break;
			}
		}
	}

	return conf;
}

PluginCfg* CPluginConfig::SearchAdd(const char *name)
{
	CLocker locker(m_cs);

	PluginCfg *conf = Search(name);

	if (!conf && m_total < MAX_PARAMS_COUNT)
	{
		conf = &m_cfg_arr[m_total++];
	}

	return conf;
}


void CPluginConfig::Load(const char *filename)
{
	CLocker locker(m_cs);
	StringCchCopy(m_filename, _countof(m_filename), filename);

	if (m_cfg_arr)
	{
		HANDLE hFile = CreateFile(m_filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwBytes;
			DWORD dwSize = GetFileSize(hFile, nullptr);
			
			if (dwSize > 0 &&
				dwSize <= MAX_PARAMS_COUNT * sizeof(PluginCfg) &&
				ReadFile(hFile, m_cfg_arr, dwSize, &dwBytes, nullptr))
			{
				m_total = dwSize / sizeof(PluginCfg);
			}

			CloseHandle(hFile);
		}
	}
}

void CPluginConfig::Save(void)
{
	CLocker locker(m_cs);

	if (m_cfg_arr)
	{
		if (m_total > 0)
		{
			HANDLE hFile = CreateFile(m_filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwBytes;
				DWORD dwSize = m_total * sizeof(PluginCfg);
				WriteFile(hFile, m_cfg_arr, dwSize, &dwBytes, nullptr);
				CloseHandle(hFile);
			}
		}
		else
		{
			DeleteFile(m_filename);
		}
	}
}

void CPluginConfig::OnConfigChange(void)
{
	for (auto &sink : m_sink)
	{
		sink->OnConfigChange(*this);
	}
}

bool CPluginConfig::Subscribe(IConfigSink *sink)
{
	return m_sink.insert(sink).second;
}

void CPluginConfig::Unsubscribe(IConfigSink *sink)
{
	auto it = m_sink.find(sink);
	if (it != m_sink.end())
	{
		m_sink.erase(it);
	}
}



/*
	Plugin configuration
*/
int APIENTRY CPluginConfig::MtSrvPluginCfgAdd(const PluginCfg *value)
{
	CPluginConfig &config = CPluginConfig::Instance();
	int result = config.Add(value);
	config.OnConfigChange();
	return result;
}

int APIENTRY CPluginConfig::MtSrvPluginCfgSet(const PluginCfg *values, const int total)
{
	CPluginConfig &config = CPluginConfig::Instance();
	int result = config.Set(values, total);
	config.OnConfigChange();
	return result;
}

int APIENTRY CPluginConfig::MtSrvPluginCfgGet(LPCSTR name, PluginCfg *value)
{
	return CPluginConfig::Instance().Get(name, value);
}

int APIENTRY CPluginConfig::MtSrvPluginCfgNext(const int index, PluginCfg *value)
{
	return CPluginConfig::Instance().Next(index, value);
}

int APIENTRY CPluginConfig::MtSrvPluginCfgDelete(LPCSTR name)
{
	CPluginConfig &config = CPluginConfig::Instance();
	int result = config.Delete(name);
	config.OnConfigChange();
	return result;
}

int APIENTRY CPluginConfig::MtSrvPluginCfgTotal()
{
	return CPluginConfig::Instance().Total();
}

int APIENTRY CPluginConfig::MtSrvPluginCfgLog(LPSTR value, int *len)
{
	return CPluginConfig::Instance().Log(value, len);
}
