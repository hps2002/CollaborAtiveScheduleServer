#ifndef __HPS_SINGLETON_H__
#define __HPS_SINGLETON_H__

namespace hps_sf{

template <class T, class X = void, int N = 0>
class hps_Singleton{
public:
    static T* GetInstance()
    {
        static T v;
        return &v;
    }
};

template<class T, class X = void, int N = 0>
class hps_SingletonPtr
{
public:
    static std::shared_ptr<T> GetInstance()
    {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}
#endif