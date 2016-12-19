#pragma once 

namespace BRS
{
/**
* auto free the instance in the current scope.
*/
#define BrsAutoFree(className, instance, is_array) \
	__BrsAutoFree<className> _auto_free_##instance(&instance, is_array)
#define BrsAutoFreeA(className, instance) \
__BrsAutoFree<className> _auto_free_array_##instance(&instance, true)
template<class T>
class __BrsAutoFree
{
private:
    T** ptr;
    bool is_array;
public:
    /**
    * auto delete the ptr.
    * @is_array a bool value indicates whether the ptr is a array.
    */
    __BrsAutoFree(T** _ptr, bool _is_array){
        ptr = _ptr;
        is_array = _is_array;
    }
    
    virtual ~__BrsAutoFree(){
        if (ptr == NULL || *ptr == NULL) {
            return;
        }
        
        if (is_array) {
            delete[] *ptr;
        } else {
            delete *ptr;
        }
        
        *ptr = NULL;
    }
};

}