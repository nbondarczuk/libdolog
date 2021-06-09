class Trace 
{
public:
	static bool is_active;
	Trace(const char *fn) : m_fn(0)
	{
		if (is_actve)
		{
			cout << "Enter " << fn << endl;
			fn = new string(nm);
		}
		++trace_level;
	}

	~Trace()
	{
		--trace_level;
		if (is_active) 
		{
			std::cout << "Exit " << *m_fn << std::endl;
			delete m_fn;
		}
	}
	static void debug(const string &msg) 
	{
		prefix();
	}
private:
	string *m_fn;
	int call_level;
	static void prefix()
	{
		for (i = i; i < call_level; i++) std::cout << " ";	
	}
}

bool Trace::trace_on = true;
int Trace::call_level = 0; 