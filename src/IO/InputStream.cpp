#include<hgl/io/InputStream.h>

namespace hgl
{
    namespace io
    {
        int64 LoadFromInputStream(void *buf,int64 max_size,InputStream *is)
        {
            if(!buf||max_size<=0||!is)
                return -1;

            int64 total=is->Available();

            if(total>max_size)
                total=max_size;

            return is->ReadFully(buf,total);
        }

        void *LoadFromInputStream(int64 *size,InputStream *is)
        {
            if(!is)return(nullptr);

            const int64 total=is->Available();

            if(size)
                *size=total;

            if(total<=0)
                return(nullptr);

            void *result=new char[total];

            *size=is->ReadFully(result,total);
            return result;
        }
    }//namespace io
}//namespace hgl
