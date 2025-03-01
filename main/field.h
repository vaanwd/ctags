/*
 *
 *  Copyright (c) 2015, Red Hat, Inc.
 *  Copyright (c) 2015, Masatake YAMATO
 *
 *  Author: Masatake YAMATO <yamato@redhat.com>
 *
 *   This source code is released for free distribution under the terms of the
 *   GNU General Public License version 2 or (at your option) any later version.
 *
 */
#ifndef CTAGS_MAIN_FIELD_H
#define CTAGS_MAIN_FIELD_H

/*
*   INCLUDE FILES
*/

#include "general.h"
#include "types.h"

#include "vstring.h"

/*
*   DATA DECLARATIONS
*/

typedef enum eFieldType { /* extension field content control */
	FIELD_UNKNOWN = -1,

	/* BASIC FIELDS */
	FIELD_NAME,
	FIELD_INPUT_FILE,
	FIELD_PATTERN,

	FIELD_ECTAGS_START,
	FIELD_COMPACT_INPUT_LINE = FIELD_ECTAGS_START,

	/* EXTENSION FIELDS */
	FIELD_JSON_LOOP_START,
	FIELD_FILE_SCOPE = FIELD_JSON_LOOP_START,
	FIELD_KIND_LONG,
	FIELD_KIND,
	FIELD_LANGUAGE,
	FIELD_LINE_NUMBER,
	FIELD_SCOPE,
	FIELD_TYPE_REF,
	FIELD_KIND_KEY,
	FIELD_ECTAGS_LOOP_START,
	FIELD_INHERITANCE = FIELD_ECTAGS_LOOP_START,
	FIELD_ACCESS,
	FIELD_IMPLEMENTATION,
	FIELD_SIGNATURE,
	FIELD_ECTAGS_LOOP_LAST = FIELD_SIGNATURE,

	/* Extension fields newly introduced in Universal Ctags. */
	FIELDS_UCTAGS_START,
	FIELD_REF_MARK = FIELDS_UCTAGS_START,
	FIELD_SCOPE_KEY,
	FIELD_SCOPE_KIND_LONG,
	FIELD_UCTAGS_LOOP_START,
	FIELD_ROLES = FIELD_UCTAGS_LOOP_START,
	FIELD_EXTRAS,
	FIELD_XPATH,
	FIELD_END_LINE,
	FIELD_EPOCH,
	FIELD_NTH,

	FIELD_BUILTIN_LAST = FIELD_NTH,
} fieldType ;

#define fieldDataTypeFlags "sib" /* used in --list-fields */
typedef enum eFieldDataType {
	FIELDTYPE_STRING  = 1 << 0,
	FIELDTYPE_INTEGER = 1 << 1,
	FIELDTYPE_BOOL    = 1 << 2,

	/* used in --list-fields */
	FIELDTYPE_END_MARKER = 1 << 3,
} fieldDataType;
/* Interpretation of VALUE of field
 *
 * With attachParserField() declared in entry.h, you can set a C string
 * as a VALUE for the specified field.
 *
 * The VALUE is interpreted very differently depending on the output
 * format: ctags, xref, and json. See field.h.
 *
 * For FIELDTYPE_STRING
 * --------------------
 * Consider if you set "str" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:str
 *   xref | str
 *   json | "foo": "str"
 *
 * Consider if you set "" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:
 *   xref | (nothing)
 *   json | "foo": ""
 *
 * Consider if you don't set the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | (nothing)
 *   xref | (nothing)
 *   json | (nothing)
 *
 *
 * For FIELDTYPE_STRING|FIELDTYPE_BOOL
 * -----------------------------------
 * If the value points "" (empty C string), the json writer prints it as
 * false, and the xref writer prints it as -.
 * THEREFORE, in the xref format, there is no way to distinguish the
 * output for the value "" and "-". Both are printed as "-".
 * If the value points a non-empty C string, Both json writer and xref
 * writers print it as-is.
 *
 * Consider if you set "str" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:str
 *   xref | str
 *   json | "foo": "str"
 *
 * Consider if you set "" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:
 *   xref | - (as specified as FIELD_NULL_LETTER_CHAR/FIELD_NULL_LETTER_STRING)
 *   json | "foo": false
 *
 * Consider if you don't set the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | (nothing)
 *   xref | (nothing)
 *   json | (nothing)
 *
 *
 * For FIELDTYPE_BOOL
 * ------------------
 * Either VALUE points "" (empty C string) or a non-epmty C string,
 * the json writer always prints true. In the same condition, the xref
 * writer always prints the name of field.
 *
 * If a value is not set, the field is treated as holding false.
 * The json writer prints nothing for the field holding false.
 * The xref writer prints - for the field holding false.

 * Consider if you set "str" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:
 *   xref | foo
 *   json | "foo": true
 *
 * Consider if you set "" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:
 *   xref | foo
 *   json | "foo": true
 *
 * Consider if you don't set the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | (nothing)
 *   xref | - (as specified as FIELD_NULL_LETTER_CHAR/FIELD_NULL_LETTER_STRING)
 *   json | (nothing)
 *
 *
 * For FIELDTYPE_INTEGER
 * ---------------------
 * If the value points "" (empty C string), the all writers print it as
 * 0. If the value points a string which cannot be converted to an integer with
 * strtol(3), the all writers print it as 1.
 *
 * Consider if you set "99" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:99
 *   xref | 99
 *   json | "foo": 99
 *
 * Consider if you set "str" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:1
 *   xref | 1
 *   json | "foo": 1
 *
 * Consider if you set "" to the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | foo:0
 *   xref | 0
 *   json | "foo": 0
 *
 * Consider if you don't set the field "foo":
 *
 * WRITER | OUTPUT
 * -------+-----------
 *  ctags | (nothing)
 *   xref | (nothing)
 *   json | (nothing)
 *
 *
 * The other data type and the combination of types are not implemented yet.
 *
 */

typedef const char* (*fieldRenderer)(const tagEntryInfo *const,
									 const char *,
									 vString *);

struct sFieldDefinition {
	/* letter, and ftype are initialized in the main part,
	   not in a parser. */
#define NUL_FIELD_LETTER '\0'
	unsigned char letter;
	const char* name;
	const char* description;
	bool enabled;

	fieldRenderer render;
	fieldRenderer renderNoEscaping;
	bool (* doesContainAnyChar) (const tagEntryInfo *const, const char*, const char *);

	bool (* isValueAvailable) (const tagEntryInfo *const, const fieldDefinition *);

	const char * getterValueType;
	struct _EsObject * (* getValueObject) (const tagEntryInfo *, const fieldDefinition *);
	const char * setterValueType;

	/* Return es_false if passed value is acceptable.
	   Return an error object is unacceptable. */
	struct _EsObject * (* checkValueForSetter) (const fieldDefinition *, const struct _EsObject *);
	struct _EsObject * (* setValueObject) (tagEntryInfo *, const fieldDefinition *, const struct _EsObject *);

	fieldDataType dataType; /* used in json output */

	unsigned int ftype;	/* Given from the main part */
};


/*
*   FUNCTION PROTOTYPES
*/

extern bool isFieldEnabled (fieldType type);

#endif	/* CTAGS_MAIN_FIELD_H */
