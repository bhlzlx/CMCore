#ifndef HGL_RES_POOL_INCLUDE
#define HGL_RES_POOL_INCLUDE

#include<hgl/type/Map.h>
namespace hgl
{
    template<typename T> struct RefData
    {
        int ref_count;
        T data;

    public:

        RefData(const T &value)
        {
            ref_count=1;
            data=value;
        }
    };//template<typename T> struct RefData

    /**
     * 附带引用计数的资源池列表
     */ 
    template<typename K,typename V> class ResPool
    {
    protected:

        using ActiveItem=RefData<V>;

        MapObject<K,ActiveItem> active_items;               ///<活跃的数据
        Map<K,V> idle_items;                                ///<引用计数为0的

    public:

        virtual ~ResPool()=default;

        const int GetActiveCount()const{return active_items.GetCount();}        ///<取得活跃项数量
        const int GetIdleCount()const{return idle_items.GetCount();}            ///<取得闲置项数量

        /**
         * 根据key列表统计数据
         * @param key_list          要扫描的key列表
         * @param in_active_count   有多少个key是活跃的
         * @param in_idle_count     有多少个key是闲置的
         * @param out_count         active/idle中都不存在的key有多少个
         * @param idle_left_count   不在active/idle中，但可以从idle中释放的个数
         */
        void Stats(const K *key_list,const int key_count,int *in_active_count,int *in_idle_count,int *out_count,int *idle_left_count)
        {
            *in_active_count=0;
            *in_idle_count=0;
            *out_count=0;
            *idle_left_count=0;
            
            const K *kp=key_list;
            for(int i=0;i<key_count;i++)
            {
                if(active_items.KeyExist(*kp))
                    ++(*in_active_count);
                else
                if(idle_items.KeyExist(*kp))
                    ++(*in_idle_count);
                else
                    ++(*out_count);

                ++kp;
            }

            *idle_left_count=idle_items.GetCount()-(*in_idle_count);
        }

        /**
         * 确定指定key是否在活跃列表
         */
        bool KeyExistActive(const K &key)const{return active_items.KeyExist(key);}

        /**
         * 确定指定key是否在闲置列表
         */
        bool KeyExistIdle(const K &key)const{return idle_items.KeyExist(key);}

        /**
         * 确定指定key是否在列表中(包括活跃列表和闲置列表)
         */
        bool KeyExist(const K &key)const
        {
            if(active_items.KeyExist(key))
                return(true);

            if(idle_items.KeyExist(key))
                return(true);

            return(false);
        }

        /**
         * 获取指定key数据的引用计数
         * @param key 要查询的key
         * @return >0 在活跃列表中
         * @return =0 在闲置列表中
         * @return <0 不存在
         */
        int GetRefCount(const K &key)const
        {
            ActiveItem *ai;

            if(active_items.Get(key,ai))                    //在活跃列表中找
                return ai->ref_count;

            if(idle_items.KeyExist(key))
                return 0;

            return -1;
        }

        /**
         * 根据key获取指定值(并增加引用引数)
         */
        bool Get(const K &key,V &value)
        {
            int pos;
            ActiveItem *ai;
                
            if(active_items.Get(key,ai))                    //在活跃列表中找
            {
                ++ai->ref_count;
                value=ai->data;
                return(true);
            }

            pos=idle_items.GetValueAndSerial(key,value);
            if(pos>0)                   //在限制列表中找
            {
                active_items.Add(key,new ActiveItem(value));
                idle_items.DeleteBySerial(pos);
                return(true);
            }

            return(false);
        }

        /**
         * 外部增加一个值
         */
        void Add(const K &key,V &value)
        {
            active_items.Add(key,new ActiveItem(value));
        }

        /**
         * 释放一个值的使用(并释放引用计数)
         */
        void Release(const K &key)
        {
            int pos;
            ActiveItem *ai;

            pos=active_items.GetValueAndSerial(key,ai);

            if(pos>0)
            {
                --ai->ref_count;

                if(ai->ref_count==0)
                {
                    idle_items.Add(key,ai->data);
                    active_items.DeleteBySerial(pos);
                }

                return;
            }
        }
    };//template<typename K,typename V> class ResPool
}//namespace hgl
#endif//HGL_RES_POOL_INCLUDE
