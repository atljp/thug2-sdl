//------------------------------------------------
//
//	LAZY STRUCT
//	Basically hack for editing CFunc values
//  Credit: Zedek the Plague Doctor (Guitar Hero Worldtour Definitive Edition / reTHAWed)
//
//------------------------------------------------

#pragma once
#include <stdint.h>
#include "LazyArray.h"

#define QBTYPE_INT				1
#define QBTYPE_FLOAT			2
#define QBTYPE_STRING			3
#define QBTYPE_SCRIPT			7
#define QBTYPE_STRUCTURE		10
#define QBTYPE_ARRAY			12
#define QBTYPE_CHECKSUM			13



namespace Script
{
	struct LazyStructItem
	{
		uint8_t unkFlag0;
		uint8_t itemType;
		uint8_t flags;				// QbNodeFlags
		uint8_t unkByteA;
		uint32_t key;				// QBKey
		uint32_t value;				// Value, could be pointer etc.
		LazyStructItem* next;
	};

	struct LazyStruct
	{
		uint16_t unk0;
		uint8_t unk2; //might be a debug flag to not free or something
		uint8_t unk3;
		LazyStructItem* first;

	public:

		void Initialize();
		void Free();
		void Clear();

		static LazyStruct* s_create();
		static void s_free(LazyStruct* to_free);

		LazyStructItem* GetItem(uint32_t qbKey);
		LazyStructItem* LastItem();
		LazyStructItem* AddItem(uint32_t qbKey);
		LazyStructItem* GetOrAdd(uint32_t qbKey);

		bool Contains(uint32_t qbKey);
		bool ContainsFlag(uint32_t qbKey);

		void AddFloat(uint32_t nameChecksum, float float_val);
		
		void SetStringItem(uint32_t qbKey, char* value);
		void AddStructure(uint32_t qbKey, LazyStruct* value); // Sets item that already exists!
		void AddArray(uint32_t nameChecksum, const Script::LazyArray* p_array);
		void RedefineArrayItem(uint32_t qbKey, void* value); // Redefine an existing array item

		int GetInteger(uint32_t qbKey);
		float GetFloat(uint32_t qbKey);
		bool GetChecksum(uint32_t checksum, uint32_t* p_checksum, bool assert);
		void AddInteger(uint32_t checksum, uint32_t value);
		void AddPair(uint32_t nameChecksum, float x, float y);
		void AddChecksum(uint32_t qbKey, uint32_t value);
		void AppendStructure(const LazyStruct* append_from);
		void AddArrayPointer(uint32_t id, void* arr);

		void* GetArray(uint32_t qbKey);
		char* GetString(uint32_t qbKey);
		LazyStruct* GetStruct(uint32_t qbKey);
	};
}