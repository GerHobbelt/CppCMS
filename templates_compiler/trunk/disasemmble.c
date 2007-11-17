#include <stdio.h>
#include "bytecode.h"

char buffer[65536];

void rewrite_buffer(int len)
{
	int i;
	for(i=0;i<len && i<32;i++) {
		if(buffer[i]<32) buffer[i]='.';
	}
	buffer[i]=0;
}

main(int argc,char *argv[])
{
	FILE *f=fopen(argv[1],"r");
	struct Tmpl_Op op;

	while(fread(&op,sizeof(op),1,f)==1){
		printf("%04x ",ftell(f)-sizeof(op));
		switch(op.opcode){
		case OP_INLINE:
			fread(buffer,1,op.parameter,f);
			rewrite_buffer(op.parameter);
			printf("Inline: `%32s' end at %04x\n",buffer,op.jump);
			break;
		case OP_CALL:
			printf("Call %d\n",op.parameter);
			break;
		case OP_VAR:
			printf("Var %d\n",op.parameter);
			break;
		case OP_INCLUDE:
			printf("Include %d\n",op.parameter);
			break;
		case OP_GOTO_IF_TRUE:
		case OP_GOTO_IF_FALSE:
		case OP_GOTO_IF_DEF:
		case OP_GOTO_IF_NDEF:
			printf("goto %04x if ",op.jump);
			switch(op.opcode){
			case OP_GOTO_IF_TRUE: printf("true");  break;
			case OP_GOTO_IF_FALSE: printf("false");  break;
			case OP_GOTO_IF_DEF: printf("def");  break;
			case OP_GOTO_IF_NDEF: printf("not def");  break;
			}
			printf(" %d\n",op.parameter);
			break;
		case OP_GOTO:
			printf("goto %04x\n",op.jump);
			break;
		case OP_STOP:
			printf("stop\n");
			break;
		default:
			printf("Unknown opcode!!!\b");
		}
	}
}
