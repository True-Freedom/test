#pragma once

#define MAX_PARAMS_COUNT	256

class CPluginConfig;


/*
	IConfigSink
*/
class IConfigSink
{
public:
	virtual void OnConfigChange(CPluginConfig &config) {}
};



/*
	Configuration
*/
class CPluginConfig
{
private:
	/*
		Destroyer
	*/
	class Destroyer
	{
	public:
		Destroyer();
		~Destroyer();
		void Initialize(CPluginConfig *config);
	private:
		CPluginConfig *m_config;
	};

protected:
	CPluginConfig(void);
	CPluginConfig(const CPluginConfig&);
	CPluginConfig operator = (CPluginConfig&);
	~CPluginConfig(void);
	friend class Destroyer;

public:
	static CPluginConfig& Instance(void);

public:
	int GetInt(const char *name, int *value, const char *defvalue);
	int GetFloat(const char *name, double *value, const char *defvalue);
	int GetString(const char *name, char *value, const int size, const char *defvalue);
	bool CheckParam(const char *name);
	bool CheckValue(const char *name, const char *value);
	bool Check(const char *name);
	bool Check(const char *name, const char *value);
	
private:
	int GetValue(const char *name, char *value, const int size, const char *defvalue);
	int SetValue(const char *name, const char *value);

public:
	/*
		������� Set(values, total), Total() � Next(index, config) ���������� ��������
		�������������������: Set(values, total), N = Total(),
		����� Next(index in [0..N), cofig)
	*/
	int Add(const PluginCfg *value);
	int Set(const PluginCfg *values, const int total);
	int Get(const char *name, PluginCfg *value);
	int Next(const int index, PluginCfg *value);
	int Delete(const char *name);
	int Total(void);
	int Log(LPSTR value, int *len);

	// !!!
	/*
		???
		��� ��������� �������������� ������� Save()
		����� ������� �� ������� Add() � Delete()
		��� �� ����� ������� �� � �� ������ ����������� �������� Set()
	*/

private:
	PluginCfg* Search(const char *name);
	PluginCfg* SearchAdd(const char *name);

public:
	void Load(const char *filename);
private:
	void Save(void);

private:
	void OnConfigChange(void);
public:
	bool Subscribe(IConfigSink *sink);
	void Unsubscribe(IConfigSink *sink);

private:
	static int APIENTRY MtSrvPluginCfgAdd(const PluginCfg *value);
	static int APIENTRY MtSrvPluginCfgSet(const PluginCfg *values, const int total);
	static int APIENTRY MtSrvPluginCfgGet(LPCSTR name, PluginCfg *value);
	static int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg *value);
	static int APIENTRY MtSrvPluginCfgDelete(LPCSTR name);
	static int APIENTRY MtSrvPluginCfgTotal();
	static int APIENTRY MtSrvPluginCfgLog(LPSTR value, int *len);

private:
	CCriticalSection m_cs;
	PluginCfg *m_cfg_arr;
	int m_total;
	// !!! �������� �� std::list
	//IConfigSink *m_sink;
	std::set<IConfigSink*> m_sink;
	char m_filename[MAX_PATH];

private:
	static CPluginConfig *m_self;
	static Destroyer m_destroyer;
};

