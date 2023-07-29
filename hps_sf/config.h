#ifndef __HPS_CONFIG_H__
#define __HPS_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "log.h"
#include <map>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace hps_sf{

//基类存放公用属性
class hps_ConfigVarBase{
public:
    typedef std::shared_ptr<hps_ConfigVarBase> ptr;
    hps_ConfigVarBase(const std::string& name, const std::string& description = ""):m_name(name), m_description(description)
    {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~hps_ConfigVarBase(){}

    const std::string& getName() const {return m_name;}
    const std::string& getDescription() const {return m_description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypename() const = 0;
protected:
    std::string m_name;
    std::string m_description;
};

// 定义基础类型的转换类, 将F类型转成T类型
template<class F, class T>
class hps_LexicalCast
{
public:
    T operator() (const F& v)
    {
        return boost::lexical_cast<T>(v);
    }
};

// 偏特化类型转化：将string转化成vector
template<class T>
class hps_LexicalCast<std::string, std::vector<T> >
{
public:
    std::vector<T> operator() (const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;    //这里使用typename定义类型，因为在模板中T的类型没有确定下来，让编译器确定这是模板的中的类型不是成员变量
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); i ++)
        {
            ss.str("");
            ss << node[i];
            vec.push_back(hps_LexicalCast<std::string, T>() (ss.str()));
        }
        return vec;
    }
};

// 偏特化类型格式转化：将vector转化成string
template<class T>
class hps_LexicalCast<std::vector<T>, std::string >
{
public:
    std::string operator() (const std::vector<T>& v)
    {
        YAML::Node node;
        for (auto& i : v)
        {
            node.push_back(YAML::Load(hps_LexicalCast<T, std::string>() ( i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 偏特化类型转化：将string转化成list
template<class T>
class hps_LexicalCast<std::string, std::list<T> >
{
public:
    std::list<T> operator() (const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;    //这里使用typename定义类型，因为在模板中T的类型没有确定下来，让编译器确定这是模板的中的类型不是成员变量
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); i ++)
        {
            ss.str("");
            ss << node[i];
            vec.push_back(hps_LexicalCast<std::string, T>() (ss.str()));
        }
        return vec;
    }
};

// 偏特化类型格式转化：将list转化成string
template<class T>
class hps_LexicalCast<std::list<T>, std::string >
{
public:
    std::string operator() (const std::list<T>& v)
    {
        YAML::Node node;
        for (auto& i : v)
        {
            node.push_back(YAML::Load(hps_LexicalCast<T, std::string>() (i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 偏特化类型转化：将string转化成set
template<class T>
class hps_LexicalCast<std::string, std::set<T> >
{
public:
    std::set<T> operator() (const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;    //这里使用typename定义类型，因为在模板中T的类型没有确定下来，让编译器确定这是模板的中的类型不是成员变量
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); i ++)
        {
            ss.str("");
            ss << node[i];
            vec.insert(hps_LexicalCast<std::string, T>() (ss.str()));
        }
        return vec;
    }
};

// 偏特化类型格式转化：将vector转化成string
template<class T>
class hps_LexicalCast<std::set<T>, std::string >
{
public:
    std::string operator() (const std::set<T>& v)
    {
        YAML::Node node;
        for (auto& i : v)
        {
            node.push_back(YAML::Load(hps_LexicalCast<T, std::string>() ( i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 偏特化类型转化：将string转化成unordered_set
template<class T>
class hps_LexicalCast<std::string, std::unordered_set<T> >
{
public:
    std::unordered_set<T> operator() (const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;    //这里使用typename定义类型，因为在模板中T的类型没有确定下来，让编译器确定这是模板的中的类型不是成员变量
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); i ++)
        {
            ss.str("");
            ss << node[i];
            vec.insert(hps_LexicalCast<std::string, T>() (ss.str()));
        }
        return vec;
    }
};

// 偏特化类型格式转化：将vector转化成string
template<class T>
class hps_LexicalCast<std::unordered_set<T>, std::string >
{
public:
    std::string operator() (const std::unordered_set<T>& v)
    {
        YAML::Node node;
        for (auto& i : v)
        {
            node.push_back(YAML::Load(hps_LexicalCast<T, std::string>() ( i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 偏特化类型转化：将string转化成map
template<class T>
class hps_LexicalCast<std::string, std::map<std::string, T> >
{
public:
    std::map<std::string, T> operator() (const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;    //这里使用typename定义类型，因为在模板中T的类型没有确定下来，让编译器确定这是模板的中的类型不是成员变量
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); it ++)
        {
            ss.str("");
            ss << it -> second;
            vec.insert(std::make_pair(it -> first.Scalar(), 
                        hps_LexicalCast<std::string, T>() (ss.str())));
        }
        return vec;
    }
};

// 偏特化类型格式转化：将map转化成string
template<class T>
class hps_LexicalCast<std::map<std::string, T>, std::string >
{
public:
    std::string operator() (const std::map<std::string, T>& v)
    {
        YAML::Node node;
        for (auto& i : v)
        {
            node[i.first] = YAML::Load(hps_LexicalCast<T, std::string>() (i.second)); 
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 偏特化类型转化：将string转化成unordered_map
template<class T>
class hps_LexicalCast<std::string, std::unordered_map<std::string, T> >
{
public:
    std::unordered_map<std::string, T> operator() (const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;    //这里使用typename定义类型，因为在模板中T的类型没有确定下来，让编译器确定这是模板的中的类型不是成员变量
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); it ++)
        {
            ss.str("");
            ss << it -> second;
            vec.insert(std::make_pair(it -> first.Scalar(), 
                        hps_LexicalCast<std::string, T>() (ss.str())));
        }
        return vec;
    }
};

// 偏特化类型格式转化：将map转化成string
template<class T>
class hps_LexicalCast<std::unordered_map<std::string, T>, std::string >
{
public:
    std::string operator() (const std::unordered_map<std::string, T>& v)
    {
        YAML::Node node;
        for (auto& i : v)
        {
            node[i.first] = YAML::Load(hps_LexicalCast<T, std::string>() (i.second)); 
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// FromStr T operator(const std::string& )
// ToStr std::string operator(const T & )
template<class T, class FromStr = hps_LexicalCast<std::string, T>, class ToStr = hps_LexicalCast<T, std::string> >
class hps_ConfigVar: public hps_ConfigVarBase{
public:
    typedef std::shared_ptr<hps_ConfigVar> ptr;
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    hps_ConfigVar(const std::string &name, 
                    const T& default_value, 
                    const std::string& description = ""):hps_ConfigVarBase(name, description), m_val(default_value)
    {
        
    }

    std::string toString() override
    {
        try
        {   
            // return boost::lexical_cast<std::string>(m_val);
            return ToStr() (m_val);
        }
        catch (std::exception& e)
        {
            HPS_LOG_ERROR(HPS_LOG_ROOT()) << "hps_ConfigVar::toString exception" 
                << e.what() << " convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    //把string类型转为成员类型
    bool fromString(const std::string& val) override
    {
        try
        {
            // m_val = boost::lexical_cast<T>(val);
            setValue(FromStr() (val));
        }
        catch (std::exception& e)
        {
            HPS_LOG_ERROR(HPS_LOG_ROOT()) << "hps_ConfigVar::toString exception" 
                << e.what() << " convert: string to" << typeid(m_val).name() << " - " << val;
        }
        return false;
    }
    const T getValue() const { return m_val; }

    void setValue(const T& v ) 
    { 
        if (v == m_val) return ;
        for (auto& i : m_cbs)
        {
            i.second(m_val, v);
        }
        m_val = v;
    }
    std::string getTypename() const override {return typeid(T).name();}
    void addListener(uint64_t key, on_change_cb cb)
    {
        m_cbs[key] = cb;
    }
    
    void delListener(uint64_t key)
    {
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key)
    {
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it -> second;
    }

    void clearListener()
    {
        m_cbs.clear();
    }
private:
    T m_val;
    // 变更回调函数组, unit_64 key, 要求唯一，一般可以使用hash
    std::map<uint64_t, on_change_cb> m_cbs;

};


//管理的类
class hps_Config
{
public:
    typedef std::map<std::string, hps_ConfigVarBase::ptr> hps_ConfigVarMap;

    template <class T>
    static typename hps_ConfigVar<T>::ptr Lookup(const std::string& name, 
                            const T& default_value, const std::string& description = "")
    {
        auto it = GetDatas().find(name);
        if (it != GetDatas().end())
        {
            auto tmp = std::dynamic_pointer_cast<hps_ConfigVar<T> > (it -> second);
            if (tmp)
            {
                HPS_LOG_INFO(HPS_LOG_ROOT()) << "Lookup name=" << name << " exsits";
                return tmp;
            }
            else 
            {
                HPS_LOG_ERROR(HPS_LOG_ROOT()) << "Lookup name=" << name << " exsits but type not" << typeid(T).name() << " real_type=" << it -> second -> getTypename() << " " << it -> second -> toString();  
                return NULL;
            }
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678") != std::string::npos)
        {
            HPS_LOG_ERROR(HPS_LOG_ROOT()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }

        typename hps_ConfigVar<T>::ptr v(new hps_ConfigVar<T> (name, default_value, description));
        GetDatas()[name] = v;
        return v;
    }

    //查找
    template <class T>
    static typename hps_ConfigVar<T>::ptr Lookup(const std::string& name)
    {
        auto it = GetDatas().find(name);
        if (it == GetDatas().end())
        {
            return nullptr;
        }
        return std::dynamic_pointer_cast<hps_ConfigVar<T> >(it -> second);
    }

    static void LoadFromYaml(const YAML::Node& root);
   
    static hps_ConfigVarBase::ptr LookupBase(const std::string& name);
private:
  static hps_ConfigVarMap& GetDatas() 
  {
    static hps_ConfigVarMap s_datas;
    return s_datas;
  }
};


}

#endif