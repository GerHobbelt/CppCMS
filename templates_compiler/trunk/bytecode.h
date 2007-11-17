#ifndef _BYTECODE_H
#define _BYTECODE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum {
	OP_INLINE	= 0,
	OP_CALL    	= 1,
	OP_VAR		= 2,
	OP_INCLUDE	= 3,
	
	OP_GOTO_IF_TRUE	= 4,
	OP_GOTO_IF_FALSE= 5,
	OP_GOTO_IF_DEF	= 6,
	OP_GOTO_IF_NDEF	= 7,
	OP_GOTO		= 8,

	OP_STOP = 0xFF
};


struct Tmpl_Op {
	uint8_t		opcode;		// Operation
	uint8_t		reg_id;		// foreach register
	uint16_t	parameter;	// variable id
	uint32_t	jump;		// Jump offset 
};


#ifdef __cplusplus
}
#endif

#endif /* _BYTECODE_H */
