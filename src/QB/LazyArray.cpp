#include <QB/LazyArray.h>
#include <QB/malloc.h>

namespace Script
{
    typedef void __cdecl CleanupArray_NativeCall(LazyArray* arr);
	CleanupArray_NativeCall* CleanupArray_Native = (CleanupArray_NativeCall *)(0x00415430);
    
    void CleanupArray(LazyArray* arr)
    {
        CleanupArray_Native(arr);
    }

    // ----------------------------
    // Create new array from pool.
    // ----------------------------
    
    LazyArray* LazyArray::s_create()
    {
        LazyArray* result = (LazyArray*)qbArrayMalloc(sizeof(LazyArray), 1);
          
        if (!result)
        {
            return nullptr;
        }
       
        result->Initialize();
        return result;
    }
    
    // ----------------------------
    // Purge and cleanup an array.
    // ----------------------------
    
    void LazyArray::s_free(LazyArray* arr)
    {
        if (!arr)
            return;
            
        // Calling this will call Clear in the array.
        Script::CleanupArray(arr);
        FreeQBArray(arr);
    }
    
    // ----------------------------
	// Initialize the array.
    // ----------------------------
    
    typedef void(__thiscall* ArrayInitializeCall)(LazyArray* arr);
    ArrayInitializeCall ArrayInitialize = (ArrayInitializeCall)(0x0046CAA0);

	void LazyArray::Initialize()
	{ 
        type = 0;
        m_union = 0;
        length = 0;
        byte1 = 0;

        ArrayInitialize(this);
	}

    // ----------------------------
    // Clear the array.
    // ----------------------------

    typedef void(__thiscall* Clear_NativeCall)(LazyArray* arr);
    Clear_NativeCall Clear_Native = (Clear_NativeCall)(0x0046CAB0);

    void LazyArray::Clear() {
        Clear_Native(this);
    }

    // ----------------------------
    // Set array size and type.
    // ----------------------------
    
    typedef void (__thiscall* SetSizeAndType_NativeCall)(LazyArray* arr, int size, uint8_t type);
    SetSizeAndType_NativeCall SetSizeAndType_Native = (SetSizeAndType_NativeCall)(0x0046CC50);
    
    void LazyArray::SetSizeAndType(int size, uint8_t type)
    {
        SetSizeAndType_Native(this, size, type);
    }

    // ----------------------------
    // Set integer
    // ----------------------------

    typedef void(__thiscall* SetInteger_NativeCall)(LazyArray* arr, uint32_t index, int int_val);
    SetInteger_NativeCall SetInteger_Native = (SetInteger_NativeCall)(0x0046CB10);

    void LazyArray::SetInteger(uint32_t index, int int_val)
    {
        SetInteger_Native(this, index, int_val);
    }  
}
