#ifndef SS_TYPES_H
#define SS_TYPES_H

/* SeaScript data types */

typedef double ss_Number;

struct ss_String {
	char* string;
	int length;
};

struct ss_Array {
	void** items;
	int length;
};

#endif // SS_TYPES_H
