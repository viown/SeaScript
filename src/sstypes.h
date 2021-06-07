#ifndef SS_TYPES_H
#define SS_TYPES_H

/* language data types */

typedef enum {
	INTEGER,
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
	ss_Number* items;
	int length;
} ss_Array;

#endif // SS_TYPES_H
