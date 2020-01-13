// Include flex header 
#include "flex-core/flex-core.h"
#include "flex-collections/flex-collections.h"

#define FLEX_TRUE 1u
#define FLEX_FALSE 0u
#define FLEX_BOOL uint8_t

// https://www.json.org/json-en.html

fObject* fJSON_ParseArray(fVM* vm, char* buf, uint32_t* curr, uint32_t* length);
fObject* fJSON_ParseObject(fVM* vm, char* buf, uint32_t* curr, uint32_t* length);
void fJSON_SkipWhitespace(char* buf, uint32_t* curr, uint32_t* length);
fObject* fJSON_ParseString(fVM* vm, char* buf, uint32_t* curr, uint32_t* length);
fObject* fJSON_ParseTrue(fVM* vm, char* buf, uint32_t* curr, uint32_t* length);
fObject* fJSON_ParseFalse(fVM* vm, char* buf, uint32_t* curr, uint32_t* length);
fObject* fJSON_ParseNull(fVM* vm, char* buf, uint32_t* curr, uint32_t* length);
fObject* fJSON_ParseNumber(fVM* vm, char* buf, uint32_t* curr, uint32_t* length);

static inline void fJSON_WriteObject(fString* json_string, const fObject* obj, const fObjectType type);
static inline void fJSON_WriteString(fString* json_string, const fObject* value);
static inline void fJSON_WriteFloat(fString* json_string, const fObject* value);
static inline void fJSON_WriteDouble(fString* json_string, const fObject* value);
static inline void fJSON_WriteInt8(fString* json_string, const fObject* value);
static inline void fJSON_WriteUInt8(fString* json_string, const fObject* value);
static inline void fJSON_WriteInt16(fString* json_string, const fObject* value);
static inline void fJSON_WriteUInt16(fString* json_string, const fObject* value);
static inline void fJSON_WriteInt32(fString* json_string, const fObject* value);
static inline void fJSON_WriteUInt32(fString* json_string, const fObject* value);
static inline void fJSON_WriteInt64(fString* json_string, const fObject* value);
static inline void fJSON_WriteUInt64(fString* json_string, const fObject* value);
static inline void fJSON_WriteBool(fString* json_string, const fObject* value);
static inline void fJSON_WriteNull(fString* json_string, const fObject* value);
static inline void fJSON_WriteVector(fString* json_string, const fObject* value);
static inline void fJSON_WriteHashTable(fString* json_string, const fObject* value);

enum json_token_type : uint8_t {
  JSON_TYPE_INVALID = 0,
  JSON_TYPE_STRING,
  JSON_TYPE_NUMBER,
  JSON_TYPE_TRUE,
  JSON_TYPE_FALSE,
  JSON_TYPE_NULL,
  JSON_TYPE_OBJECT_START,
  JSON_TYPE_OBJECT_END,
  JSON_TYPE_ARRAY_START,
  JSON_TYPE_ARRAY_END,
};

enum json_token_type get_token_type(char c) {
	switch (c) {
		case '{': return JSON_TYPE_OBJECT_START;
		case '[': return JSON_TYPE_ARRAY_START;
		case '"': return JSON_TYPE_STRING;
		case 'n': return JSON_TYPE_NULL;
		case 'f': return JSON_TYPE_FALSE;
		case 't': return JSON_TYPE_TRUE;
	}

	if ((c >= '0' && c <= '9') || (c == '-' || c == '+')) {
		return JSON_TYPE_NUMBER;
	} else {
		return JSON_TYPE_INVALID;
	}
}

fObject* fJSON_ParseObject(fVM* vm, char* buf, uint32_t* curr, uint32_t* length) {
	// puts("fJSON_ParseObject");
	// printf("curr: %d\n", *curr);
	char c = buf[*curr];
	// puts(buf);

	assert(c == '{');

	fHashTable* ht = fHashTable_New();
	assert(ht);


	// Skip {
	(*curr)++;
	c = buf[*curr];

	enum json_token_type token_type;

	fObject *key, *value;

	while (buf[*curr] != '}') {
		// printf("c=%c\n", buf[*curr]);
		fJSON_SkipWhitespace(buf, curr, length);
		token_type = get_token_type(buf[*curr]);

		if (token_type == JSON_TYPE_STRING) {
			key = fJSON_ParseString(vm, buf, curr, length);
		}

		fJSON_SkipWhitespace(buf, curr, length);
		c = buf[*curr];
		
		assert(c == ':');
		(*curr)++;
		
		fJSON_SkipWhitespace(buf, curr, length);

		token_type = get_token_type(buf[*curr]); 

		// fVM* vm, fObject* a, fObject* key, fObject* b
		// fHashTable_SetFromObject(a->hashTableValue, key, b);

		switch (token_type) {
			case JSON_TYPE_STRING: value = fJSON_ParseString(vm, buf, curr, length); break;
			case JSON_TYPE_NUMBER: value = fJSON_ParseNumber(vm, buf, curr, length); break;
			case JSON_TYPE_TRUE: value = fJSON_ParseTrue(vm, buf, curr, length); break;
			case JSON_TYPE_FALSE: value = fJSON_ParseFalse(vm, buf, curr, length); break;
			case JSON_TYPE_NULL: value = fJSON_ParseNull(vm, buf, curr, length); break;
			case JSON_TYPE_ARRAY_START: value = fJSON_ParseArray(vm, buf, curr, length); break;
			case JSON_TYPE_OBJECT_START: value = fJSON_ParseObject(vm, buf, curr, length); break;
			default: break;
		}

		fHashTable_SetFromObject(ht, key, value);

		fJSON_SkipWhitespace(buf, curr, length);

		if (buf[*curr] == ',') {
			(*curr)++;
		}
	}

	(*curr)++;
	// fHashTable_Set(ht, key, 5, &x);

	return fVM_PushHashTableFromHashTable(vm, ht);
}

void fJSON_SkipWhitespace(char* buf, uint32_t* curr, uint32_t* length) {
	const uint32_t end = (*length)-1;
	char c = buf[*curr];

	while (buf[*curr] != buf[end] && (c == ' ' || c == '\r' || c == '\n' || c == '\t')) {
		(*curr)++;
		c = buf[*curr];
	}
}

fObject* fJSON_ParseArray(fVM* vm, char* buf, uint32_t* curr, uint32_t* length) {
	fVector* vec = fVector_New();
	assert(vec);
	assert(buf[*curr] == '[');

	// Skip [
	(*curr)++;
	char c = buf[*curr];

	enum json_token_type token_type;

	while (buf[*curr] != ']') {
		// printf("c=%c\n", buf[*curr]);
		fJSON_SkipWhitespace(buf, curr, length);
		token_type = get_token_type(buf[*curr]);

		switch (token_type) {
			case JSON_TYPE_STRING: fVector_Push(vec, fJSON_ParseString(vm, buf, curr, length)); break;
			case JSON_TYPE_NUMBER: fVector_Push(vec, fJSON_ParseNumber(vm, buf, curr, length)); break;
			case JSON_TYPE_TRUE: fVector_Push(vec, fJSON_ParseTrue(vm, buf, curr, length)); break;
			case JSON_TYPE_FALSE: fVector_Push(vec, fJSON_ParseFalse(vm, buf, curr, length)); break;
			case JSON_TYPE_NULL: fVector_Push(vec, fJSON_ParseNull(vm, buf, curr, length)); break;
			case JSON_TYPE_ARRAY_START: fVector_Push(vec, fJSON_ParseArray(vm, buf, curr, length)); break;
			case JSON_TYPE_OBJECT_START: fVector_Push(vec, fJSON_ParseObject(vm, buf, curr, length)); break;
			default: break;
		}

		fJSON_SkipWhitespace(buf, curr, length);

		if (buf[*curr] == ',') {
			(*curr)++;
		}
	}

	// Skip ]
	(*curr)++;

	return fVM_PushVectorFromVector(vm, vec);
}

fObject* fJSON_ParseString(fVM* vm, char* buf, uint32_t* curr, uint32_t* length) {
	// puts("fJSON_ParseString");
	assert(buf[*curr] == '"');

	// Skip Leading "
	(*curr)++;
	uint32_t start = *curr;
	uint32_t len = 0;
	
	while (*curr < *length) {
		if (buf[*curr] == '"') break;
		(*curr)++;
		len++;
	}
	// printf("Len: %d\n", len);
	// printf("Start: %p\n", buf + start);

	fString* str = fString_NewWithLength(buf + start, len);

	// printf("String (%d): '%s'\n", len, fString_GetBuffer(str));

	assert(buf[*curr] == '"');
	// Skip Trailing "
	(*curr)++;

	return fVM_PushStringFromString(vm, str);
}

fObject* fJSON_ParseTrue(fVM* vm, char* buf, uint32_t* curr, uint32_t* length) {
	// puts("fJSON_ParseTrue");
	char c = buf[*curr];

	assert(c == 't');

	while (c == 't' || c == 'r' || c == 'u' || c == 'e') {
		(*curr)++;
		c = buf[*curr];
	}

	return fVM_GetTrueObject(vm);
}

fObject* fJSON_ParseFalse(fVM* vm, char* buf, uint32_t* curr, uint32_t* length) {
	// puts("fJSON_ParseFalse");
	char c = buf[*curr];

	assert(c == 'f');

	while (c == 'f' || c == 'a' || c == 'l' || c == 's' || c == 'e') {
		(*curr)++;
		c = buf[*curr];
	}

	return fVM_GetFalseObject(vm);
}

fObject* fJSON_ParseNull(fVM* vm, char* buf, uint32_t* curr, uint32_t* length) {
	// puts("fJSON_ParseNull");
	char c = buf[*curr];

	assert(c == 'n');

	while (c == 'n' || c == 'u' || c == 'l') {
		(*curr)++;
		c = buf[*curr];
	}

	return fVM_GetNullObject(vm);
}

fObject* fJSON_ParseNumber(fVM* vm, char* buf, uint32_t* curr, uint32_t* length) {
	// puts("fJSON_ParseNumber");
	char c = buf[*curr];

	assert((c >= '0' && c <= '9') || (c == '-' || c == '+'));

	uint32_t start = *curr;
	uint32_t len = 0;
	uint8_t isNegative = FLEX_FALSE;
	uint8_t isFraction = FLEX_FALSE;
	uint8_t isExponent = FLEX_FALSE;
	uint8_t fractionLen = 0;
	
	while (*curr < *length) {
		c = buf[*curr];

		if (c == '-') {
			isNegative = FLEX_TRUE;
			(*curr)++;
			len++;
			c = buf[*curr];
		}

		if (c >= '1' & c <= '9') {
			// Parse digits
			while (c >= '1' && c <= '9') {
				(*curr)++;
				len++;
				c = buf[*curr];
			}
		}

		if (c == '.')  {
			isFraction = FLEX_TRUE;
			(*curr)++;
			len++;
			c = buf[*curr];

			// Parse fraction
			while (c >= '0' && c <= '9') {
				fractionLen++;
				(*curr)++;
				len++;
				c = buf[*curr];
			}
		}

		if (c == 'e' || c == 'E') {
			isExponent = FLEX_TRUE;
			printf("TODO: EXPONENT");
			(*curr)++;
			len++;
			c = buf[*curr];
		}

		if (!(c >= '1' & c <= '9')) break;
	}

	// printf("Len: %d\n", len);
	// printf("Start: %p\n", buf + start);
	// printf("isFraction: %d\n", isFraction);
	// printf("isNegative: %d\n", isNegative);
	// printf("isExponent: %d\n", isExponent);
	// printf("fractionLen: %d\n", fractionLen);

	char *ptr;

	if (isFraction) {
		if (fractionLen > 7) {
			double val = strtod(buf+start, &ptr); // strntod(buf + start, ptr, len); //
			return fVM_PushDouble(vm, val);
		} else {
			float val = strtof(buf+start, &ptr);
			return fVM_PushFloat(vm, val);
		}
	} else {
		if (isNegative) {
			if (len < 11) {
				int32_t val = strtol(buf+start, NULL, 0);
				return fVM_PushInt32(vm, val);
			} else {
				int64_t val = strtol(buf+start, NULL, 0);
				return fVM_PushInt64(vm, val);
			}
		} else {
			if (len < 11) {
				uint32_t val = strtoul(buf+start, NULL, 0);
				return fVM_PushUInt32(vm, val);
			} else {
				uint64_t val = strtoul(buf+start, NULL, 0);
				return fVM_PushUInt64(vm, val);
			}
		}
	}

	// fString* str = fString_NewWithLength(buf + start, len);
	// printf("String (%d): '%s'\n", len, fString_GetBuffer(str));
	// assert(buf[*curr] == '"');

	return fVM_PushDouble(vm, 0);
	// return fVM_PushStringFromString(vm, str);
}

fObject* fJSON_Parse(fVM* vm, fString* val) {
	// fObject* hash_empty = fVM_PushRoot(globalVM, fVM_PushHashTable(globalVM, 0));
	// fHashTable* hash_empty = fHashTable_New();
	char* buf = fString_GetBuffer(val);
	uint32_t length = fString_GetLength(val);
	uint32_t curr = 0;

	assert(vm);
	assert(val);
	assert(length > 0);

	enum json_token_type root_type = get_token_type(buf[0]);

	switch (root_type) {
		case JSON_TYPE_STRING: return fJSON_ParseString(vm, buf, &curr, &length);
		case JSON_TYPE_NUMBER: return fJSON_ParseNumber(vm, buf, &curr, &length);
		case JSON_TYPE_TRUE: return fJSON_ParseTrue(vm, buf, &curr, &length);
		case JSON_TYPE_FALSE: return fJSON_ParseFalse(vm, buf, &curr, &length);
		case JSON_TYPE_NULL: return fJSON_ParseNull(vm, buf, &curr, &length);
		case JSON_TYPE_ARRAY_START: return fJSON_ParseArray(vm, buf, &curr, &length);
		case JSON_TYPE_OBJECT_START: return fJSON_ParseObject(vm, buf, &curr, &length);
		default: 
			return fVM_PushString(vm, "Invalid JSON String");
	}
}


static inline void fJSON_WriteObject(fString* json_string, const fObject* obj, const fObjectType type) {
	switch (type) {
		case OBJ_INT8: fJSON_WriteInt8(json_string, obj); break;
		case OBJ_INT16: fJSON_WriteInt16(json_string, obj); break;
		case OBJ_INT32: fJSON_WriteInt32(json_string, obj); break;
		case OBJ_INT64: fJSON_WriteInt64(json_string, obj); break;
		case OBJ_UINT8: fJSON_WriteUInt8(json_string, obj); break;
		case OBJ_UINT16: fJSON_WriteUInt16(json_string, obj); break;
		case OBJ_UINT32: fJSON_WriteUInt32(json_string, obj); break;
		case OBJ_UINT64: fJSON_WriteUInt64(json_string, obj); break;
		case OBJ_FLOAT: fJSON_WriteFloat(json_string, obj); break;
		case OBJ_DOUBLE: fJSON_WriteDouble(json_string, obj); break;
		case OBJ_STRING: fJSON_WriteString(json_string, obj); break;
		case OBJ_VECTOR: fJSON_WriteVector(json_string, obj); break;
		case OBJ_HASH_TABLE: fJSON_WriteHashTable(json_string, obj); break;
		case OBJ_ARRAY: break;
		case OBJ_NULL: fJSON_WriteNull(json_string, obj); break;
		case OBJ_BOOL: fJSON_WriteBool(json_string, obj); break;
		default:
			printf("Type not supported for JSON Serialization");
		break;
	}
}

static inline void fJSON_WriteString(fString* json_string, const fObject* value) {
	// Write opening "
	fString_AddFromChar(json_string, "\"", 1);

	// // Write String content
	fString_Add(json_string, fObject_GetStringValue(value));

	// // Write closing "
	fString_AddFromChar(json_string, "\"", 1);
}

static inline void fJSON_WriteFloat(fString* json_string, const fObject* value) {
	char buf[64];
	sprintf(buf, "%0.7f", fObject_GetFloatValue(value));

	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteDouble(fString* json_string, const fObject* value) {
	char buf[64];
	sprintf(buf, "%0.15f", fObject_GetDoubleValue(value));

	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteInt8(fString* json_string, const fObject* value) {
	char buf[64];
	f_inttostr(buf, fObject_GetInt8Value);
	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteUInt8(fString* json_string, const fObject* value) {
	char buf[64];
	f_itoa_unpadded(buf, fObject_GetUInt8Value);
	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteInt16(fString* json_string, const fObject* value) {
	char buf[64];
	f_inttostr(buf, fObject_GetInt16Value(value));
	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteUInt16(fString* json_string, const fObject* value) {
	char buf[64];
	f_itoa_unpadded(buf, fObject_GetUInt16Value(value));
	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteInt32(fString* json_string, const fObject* value) {
	char buf[64];
	f_inttostr(buf, fObject_GetInt32Value(value));
	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteUInt32(fString* json_string, const fObject* value) {
	char buf[64];
	f_itoa_unpadded(buf, fObject_GetUInt32Value(value));
	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteInt64(fString* json_string, const fObject* value) {
	char buf[64];
	sprintf(buf, "%lld", fObject_GetInt64Value(value));
	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteUInt64(fString* json_string, const fObject* value) {
	char buf[64];
	sprintf(buf, "%lld", fObject_GetUInt64Value(value));
	fString_AddFromChar(json_string, buf, strlen(buf));
}

static inline void fJSON_WriteBool(fString* json_string, const fObject* value) {
	if (fObject_GetBoolValue(value)) {
		fString_AddFromChar(json_string, "true", 4);
	} else {
		fString_AddFromChar(json_string, "false", 5);
	}
}

static inline void fJSON_WriteNull(fString* json_string, const fObject* value) {	
	fString_AddFromChar(json_string, "null", 4);
}

static inline void fJSON_WriteVector(fString* json_string, const fObject* value) {
	fString_AddFromChar(json_string, "[", 1);

	fVector* vec = fObject_GetVectorValue(value);
	uint32_t len = fVector_GetLength(vec);
	fObject* obj;
	fObjectType obj_type;

	for (uint32_t i = 0; i < len; i++) {
		obj = fVector_Get(vec, i);
		obj_type = fObject_GetType(obj);

		fJSON_WriteObject(json_string, obj, obj_type);

		if (i < len-1) {
			fString_AddFromChar(json_string, ",", 1);
		}
	}

	fString_AddFromChar(json_string, "]", 1);	
}


static inline void fJSON_WriteHashTable(fString* json_string, const fObject* value) {
	fString_AddFromChar(json_string, "{", 1);
	fHashTable* ht = fObject_GetHashTableValue(value);

	uint32_t len = fHashTable_GetLength(ht);
	fObject* key, val;
	fObjectType key_type, val_type;

	fHashTableIterator *iter = fHashTableIterator_New(ht);
    fHashTableItem *item = fHashTableIterator_Next(iter);
    uint32_t i = 0;
    
    while (item) {
		key = fHashTableItem_GetKey(item);
		key_type = fObject_GetType(key);
		value = fHashTableItem_GetValue(item);
		val_type = fObject_GetType(value);

		fJSON_WriteObject(json_string, key, key_type);
		fString_AddFromChar(json_string, ":", 1);
		fJSON_WriteObject(json_string, value, val_type);

		if (i < len-1) {
			fString_AddFromChar(json_string, ",", 1);
		}
		i++;
		item = fHashTableIterator_Next(iter);
	}
    
    fHashTableIterator_Free(iter);

	fString_AddFromChar(json_string, "}", 1);
}

fObject* fJSON_ToString(fVM* vm, fObject* obj) {
	assert(vm);
	assert(obj);

	fString* json_string = fString_NewWithCapacity(1024);
	fObjectType root_type = fObject_GetType(obj);

	fString* strVal;

	fJSON_WriteObject(json_string, obj, root_type);

	return fVM_PushStringFromString(vm, json_string);
}
