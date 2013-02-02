
#pragma once

template <typename T> 
class Singleton
{
protected:
	static T *m_pInstance;
		
public:
	static void CreateInstance() 
	{ 
		if (!m_pInstance)
			m_pInstance = new T();
	}
	static void DestroyInstance()
	{
		if (m_pInstance != 0) 
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}
		
	inline static T * Instance( void );
	inline static T & Ref( void ); 
	
	inline static bool Incarnated();
};

template <typename T> 
T *Singleton<T>::Instance(void)
{
	return m_pInstance;
}

template <typename T> 
T & Singleton<T>::Ref( void )
{
	// Attempt to dereference null ptr will result in
	// runtime_error 
	return *m_pInstance;
}
	
template <class T> 
bool Singleton<T>::Incarnated()
{	
	return m_pInstance != NULL;
}
	
// initialize the static instance ptr to null
template <typename T> 
T * Singleton<T>::m_pInstance = NULL;
