#ifndef __HPS_CONFIG_H__
#define __HPS_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "log.h"
#include <map>
#include <yaml-cpp/yaml.h>

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
protected:
    std::string m_name;
    std::string m_description;
};

template<class T>
class hps_ConfigVar: public hps_ConfigVarBase{
public:
    typedef std::shared_ptr<hps_ConfigVar> ptr;

    hps_ConfigVar(const std::string &name, 
                    const T& default_value, 
                    const std::string& description = ""):hps_ConfigVarBase(name, description), m_val(default_value)
    {
        
    }

    std::string toString() override
    {
        try
        {   
            return boost::lexical_cast<std::string>(m_val);
        }
        catch (std::exception& e)
        {
            HPS_LOG_ERROR(HPS_LOG_ROOT()) << "hps_ConfigVar::toString exception" 
                << e.what() << " convert:" << typeid(m_val).name() << " to string";
        }
        return "";
    }

    //把string类型转为成员类型
    bool fromString(const std::string& val) override
    {
        try
        {
            m_val = boost::lexical_cast<T>(val);
        }
        catch (std::exception& e)
        {
            HPS_LOG_ERROR(HPS_LOG_ROOT()) << "hps_ConfigVar::toString exception" 
                << e.what() << " convert: string to" << typeid(m_val).name();
        }
        return false;
    }
    const T getValue() const { return m_val; };
    void setValue(const T& v ) { m_val = v; };
private:
    T m_val;
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
        auto tmp = Lookup<T>(name);
        if (tmp)
        {
            HPS_LOG_INFO(HPS_LOG_ROOT()) << "Lookup name=" << name << "exists";
            return tmp;
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678") != std::string::npos)
        {
            HPS_LOG_ERROR(HPS_LOG_ROOT()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }

        typename hps_ConfigVar<T>::ptr v(new hps_ConfigVar<T> (name, default_value, description));
        s_datas[name] = v;
        return v;
    }

    //查找
    template <class T>
    static typename hps_ConfigVar<T>::ptr Lookup(const std::string& name)
    {
        auto it = s_datas.find(name);
        if (it == s_datas.end())
        {
            return nullptr;
        }
        return std::dynamic_pointer_cast<hps_ConfigVar<T> >(it -> second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static hps_ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static hps_ConfigVarMap s_datas;
};


}

#endif