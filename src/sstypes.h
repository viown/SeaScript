#ifndef SS_TYPES_H
#define SS_TYPES_H

/* SeaScript data types */

typedef enum {
	NUMBER,
	STRING,
	ARRAY,
	FUNCTION
} ObjectType;

typedef struct {
	void* object;
	ObjectType type;
} ss_Object;

typedef double ss_Number;

typedef struct {
	void** items;
	int length;
} ss_Array;

#endif // SS_TYPES_H
