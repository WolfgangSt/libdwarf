#ifndef names_h
#define names_h
/* 
	makename.h   
	$Revision: 1.2 $
	$Date: 1999/03/05 21:59:53 $

	This is for putting strings into stable storage and
	avoiding duplicate string storage.

*/

char * makename(char *); /* returns Name given string. Copies to stable
	string storage and creates Name if one not in existence
	already */

#endif
