//------------------------------------------------
//
//	LAZY ARRAY
//
//------------------------------------------------

#pragma once

#include <stdint.h>

namespace Script
{
    struct LazyStruct;
    
	struct LazyArray
	{
        // TODO: Port from GHWT:DE.
        static LazyArray* s_create();
        static void s_free(LazyArray* arr);
        
		uint8_t byte0;
		uint8_t byte1;
		
		//~ QbValueType type;
		uint8_t type;
		
		uint8_t byte3;
        
		int length;
        
        union
        {
            // Generic pointer.
            // Used when calling Mem::Free.
            uint32_t *mp_array_data;
            
            int *mp_integers;
            float *mp_floats;
            uint32_t *mp_checksums;
            
            char **mpp_strings;
            char **mpp_local_strings;
            void **mpp_pairs;
            void **mpp_vectors;
            LazyStruct **mpp_structures;
            LazyArray **mpp_arrays;
            
            // In the case of the array containing only 1 element, the element itself is
            // stored here, rather than allocating a block of memory for it.
            // This is a memory optimization.
            // Each memory block uses 16 bytes for the header, and the data is padded to
            // occupy 16 bytes. So in the case of an array of 1 element this saves 32 bytes.
            // There are lots of arrays of 1 element, eg the links arrays in each node of 
            // the NodeArray often only contain 1 link.
            int m_integer;
            float m_float;
            uint32_t m_checksum;
            char *mp_string;
            char *mp_local_string;
            void *mp_pair;
            void *mp_vector;
            LazyStruct *mp_structure;
            LazyArray *mp_array;
            // Used to zero the single element.
            uint32_t m_union;
        };
        
		public:
			void Initialize();
            void Clear();
            void SetSizeAndType(int size, uint8_t type);
            void SetInteger(uint32_t index, int int_val);
            
	};
    
    //~ void CopyArray(LazyArray* dst, LazyArray* src, int allowPointers);
}

namespace Script
{
    void CleanupArray(LazyArray* arr);
}
